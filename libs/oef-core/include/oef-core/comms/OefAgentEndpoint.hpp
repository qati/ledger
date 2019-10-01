#pragma once
//------------------------------------------------------------------------------
//
//   Copyright 2018-2019 Fetch.AI Limited
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.
//
//------------------------------------------------------------------------------

#include "logging/logging.hpp"
#include "oef-base/proto_comms/ProtoMessageEndpoint.hpp"
#include "oef-core/karma/KarmaAccount.hpp"
#include <memory>

class Core;
template <class OefEndpoint>
class IOefTaskFactory;
class ProtoMessageReader;
class ProtoMessageSender;
class IKarmaPolicy;

class OefAgentEndpoint
  : public EndpointPipe<ProtoMessageEndpoint<std::shared_ptr<google::protobuf::Message>>>,
    public std::enable_shared_from_this<OefAgentEndpoint>
{
public:
  static constexpr char const *LOGGING_NAME = "OefAgentEndpoint";
  using TXType                              = std::shared_ptr<google::protobuf::Message>;

  using Parent = EndpointPipe<ProtoMessageEndpoint<std::shared_ptr<google::protobuf::Message>>>;
  using SELF_P = std::shared_ptr<OefAgentEndpoint>;

  OefAgentEndpoint(std::shared_ptr<ProtoMessageEndpoint<TXType>> endpoint);
  virtual ~OefAgentEndpoint();

  void setFactory(std::shared_ptr<IOefTaskFactory<OefAgentEndpoint>> new_factory);
  void setup(IKarmaPolicy *karmaPolicy);

  virtual void go(void) override
  {
    FETCH_LOG_INFO(LOGGING_NAME, "------------------> OefAgentEndpoint::go");

    SELF_P self = shared_from_this();
    while (!go_functions.empty())
    {
      go_functions.front()(self);
      go_functions.pop_front();
    }

    Parent::go();
  }

  KarmaAccount karma;
  struct
  {
    bool will_heartbeat;
  } capabilities;

  void heartbeat(void);
  void heartbeat_recvd(void);

  void close(const std::string &reason);
  void setState(const std::string &stateName, bool value);
  bool getState(const std::string &stateName) const;

  std::size_t getIdent() const
  {
    return ident;
  }

  void addGoFunction(std::function<void(SELF_P)> func)
  {
    go_functions.push_back(func);
  }

protected:
private:
  std::map<std::string, bool>                        states;
  std::size_t                                        ident;
  std::shared_ptr<IOefTaskFactory<OefAgentEndpoint>> factory;
  std::list<std::function<void(SELF_P)>>             go_functions;
  std::atomic<unsigned int>                          outstanding_heartbeats;

  OefAgentEndpoint(const OefAgentEndpoint &other) = delete;
  OefAgentEndpoint &operator=(const OefAgentEndpoint &other)  = delete;
  bool              operator==(const OefAgentEndpoint &other) = delete;
  bool              operator<(const OefAgentEndpoint &other)  = delete;
};