#include "oef-search/comms/OefSearchEndpoint.hpp"

#include "oef-base/comms/Endianness.hpp"
#include "oef-base/monitoring/Counter.hpp"
#include "oef-base/monitoring/Gauge.hpp"
#include "oef-base/proto_comms/ProtoMessageReader.hpp"
#include "oef-base/proto_comms/ProtoMessageSender.hpp"
#include "oef-base/threading/Task.hpp"
#include "oef-base/threading/Taskpool.hpp"
#include "oef-base/utils/Uri.hpp"
#include "oef-search/comms/SearchTaskFactory.hpp"

static Gauge count("mt-search.network.OefSearchEndpoint");

OefSearchEndpoint::OefSearchEndpoint(std::shared_ptr<ProtoEndpoint> endpoint)
  : Parent(std::move(endpoint))
{
  count++;
  ident = count.get();
}

void OefSearchEndpoint::close(const std::string &reason)
{
  Counter("mt-search.network.OefSearchEndpoint.closed")++;
  Counter(std::string("mt-search.network.OefSearchEndpoint.closed.") + reason)++;

  socket().close();
}

void OefSearchEndpoint::setState(const std::string &stateName, bool value)
{
  states[stateName] = value;
}

bool OefSearchEndpoint::getState(const std::string &stateName) const
{
  auto entry = states.find(stateName);
  if (entry == states.end())
  {
    return false;
  }
  return entry->second;
}

void OefSearchEndpoint::setup()
{
  // can't do this in the constructor because shared_from_this doesn't work in there.
  std::weak_ptr<OefSearchEndpoint> myself_wp = shared_from_this();

  endpoint->setOnStartHandler([myself_wp]() {});

  auto myGroupId = getIdent();

  endpoint->setOnCompleteHandler([myGroupId, myself_wp](bool success, unsigned long id, Uri uri,
                                                        ConstCharArrayBuffer buffers) {
    if (auto myself_sp = myself_wp.lock())
    {
      Task::setThreadGroupId(myGroupId);
      myself_sp->factory->current_uri_ = uri;
      myself_sp->factory->processMessage(buffers);
    }
  });

  endpoint->setOnErrorHandler([myGroupId, myself_wp](std::error_code const &ec) {
    if (auto myself_sp = myself_wp.lock())
    {
      // myself_sp -> factory -> endpointClosed();
      // myself_sp -> factory.reset();
      Taskpool::getDefaultTaskpool().lock()->cancelTaskGroup(myGroupId);
    }
  });

  endpoint->setOnEofHandler([myGroupId, myself_wp]() {
    if (auto myself_sp = myself_wp.lock())
    {
      // myself_sp -> factory -> endpointClosed();
      // myself_sp -> factory.reset();
      Taskpool::getDefaultTaskpool().lock()->cancelTaskGroup(myGroupId);
    }
  });

  endpoint->setOnProtoErrorHandler([myGroupId, myself_wp](const std::string &message) {
    if (auto myself_sp = myself_wp.lock())
    {
      // myself_sp -> factory -> endpointClosed();
      // myself_sp -> factory.reset();
      Taskpool::getDefaultTaskpool().lock()->cancelTaskGroup(myGroupId);
    }
  });
}

void OefSearchEndpoint::setFactory(std::shared_ptr<SearchTaskFactory> new_factory)
{
  if (factory)
  {
    new_factory->endpoint = factory->endpoint;
  }
  factory = new_factory;
}

OefSearchEndpoint::~OefSearchEndpoint()
{
  endpoint->setOnCompleteHandler(nullptr);
  endpoint->setOnErrorHandler(nullptr);
  endpoint->setOnEofHandler(nullptr);
  endpoint->setOnProtoErrorHandler(nullptr);
  FETCH_LOG_INFO(LOGGING_NAME, "~OefSearchEndpoint");
  count--;
}
