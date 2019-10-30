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

#include "dmlf/message_system/messaging_interface.hpp"
#include "dmlf/message_system/messaging_protocol.hpp"
#include "muddle/rpc/client.hpp"
#include "muddle/rpc/server.hpp"


namespace fetch {
namespace dmlf {

class MuddleMessaging : public MessagingInterface
{
public:
  using Target           = MessagingInterface::Target;
  using PromiseOfMessage = MessagingInterface::PromiseOfMessage;
  using PromiseOfBool    = MessagingInterface::PromiseOfBool;
  using MessageHandler   = MessagingInterface::MessageHandler;
  using MuddlePtr        = muddle::MuddlePtr;

  MuddleMessaging(MuddlePtr muddle, uint16_t server_port, uint16_t client_port);
  ~MuddleMessaging() override = default;


  PromiseOfMessage Pull(Address const &target, Message const &message) override;
  PromiseOfBool    Push(Address const &target, Message const &message) override;

  bool ReceiveMessage(service::CallContext const &context, Message const &message) override;

  void SetMessageHandler(MessageHandler handler) override;

  MuddleMessaging(MuddleMessaging const &other) = delete;
  MuddleMessaging &operator=(MuddleMessaging const &other)  = delete;
  bool            operator==(MuddleMessaging const &other)  = delete;
  bool            operator<(MuddleMessaging const &other)   = delete;

private:
  MuddlePtr muddle_;
  MessageHandler message_handler_;
  std::shared_ptr<Server>    server_
  std::shared_ptr<RpcClient> client_;
  std::shared_ptr<MessagingProtocol> protocol_;
};

}  // namespace dmlf
}  // namespace fetch
