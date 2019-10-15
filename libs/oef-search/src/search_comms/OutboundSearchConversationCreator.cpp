#include "oef-search/search_comms/OutboundSearchConversationCreator.hpp"

#include "logging/logging.hpp"
#include "oef-base/proto_comms/ProtoMessageEndpoint.hpp"
#include "oef-base/threading/StateMachineTask.hpp"
#include "oef-base/utils/Uri.hpp"

#include "oef-base/conversation/OutboundConversationWorkerTask.hpp"
#include "oef-messages/dap_interface.hpp"
#include "oef-messages/search_message.hpp"
#include "oef-messages/search_query.hpp"
#include "oef-messages/search_remove.hpp"
#include "oef-messages/search_transport.hpp"
#include "oef-messages/search_update.hpp"

#include <google/protobuf/message.h>

OutboundSearchConversationCreator::OutboundSearchConversationCreator(size_t     thread_group_id,
                                                                     const Uri &search_uri,
                                                                     Core &     core)
{
  FETCH_LOG_INFO(LOGGING_NAME, "Creating search to search conversation with ",
                 search_uri.ToString());
  worker = std::make_shared<OutboundConversationWorkerTask>(core, search_uri, ident2conversation);

  worker->SetThreadGroupId(thread_group_id);

  worker->submit();
}

OutboundSearchConversationCreator::~OutboundSearchConversationCreator()
{
  worker.reset();
}

std::shared_ptr<OutboundConversation> OutboundSearchConversationCreator::start(
    const Uri &target_path, std::shared_ptr<google::protobuf::Message> initiator)
{
  FETCH_LOG_INFO(LOGGING_NAME, "Starting search to search conversation...");
  auto this_id = ident++;

  std::shared_ptr<OutboundConversation> conv;

  if (target_path.path == "search")
  {
    conv = std::make_shared<OutboundTypedConversation<IdentifierSequence>>(this_id, target_path,
                                                                           initiator);
  }
  else
  {
    FETCH_LOG_WARN(LOGGING_NAME, "Path ", target_path.path,
                   " not supported in search to search comm!");
    throw std::invalid_argument(
        target_path.path + " is not a valid target, to start a OutboundSearchConversationCreator!");
  }

  ident2conversation[this_id] = conv;
  worker->post(conv);
  return conv;
}