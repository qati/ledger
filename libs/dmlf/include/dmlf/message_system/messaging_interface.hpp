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

#include "dmlf/message_system/message.hpp"
#include "network/generics/promise_of.hpp"
#include "network/service/call_context.hpp"

#include <functional>

namespace fetch {
namespace dmlf {

class MessagingInterface
{
public:
  using Address          = byte_array::ConstByteArray;
  using PromiseOfMessage = fetch::network::PromiseOf<Message>;
  using PromiseOfBool    = fetch::network::PromiseOf<bool>;
  using MessageHandler   = std::function<void(Address const &source, Message const &message, std::shared_ptr<MessagingInterface> &messaging)>;

  MessagingInterface()          = default;
  virtual ~MessagingInterface() = default;

  /// @name Client methods
  /// @{

  virtual PromiseOfMessage Pull(Address const &target, Message const &message) = 0;
  virtual PromiseOfBool    Push(Address const &target, Message const &message) = 0;

  /// @}

  virtual bool ReceiveMessage(service::CallContext const &context, Message const &message) = 0;

  virtual void SetMessageHandler(MessageHandler handler) = 0;

  MessagingInterface(MessagingInterface const &other)             = delete;
  MessagingInterface &operator=(MessagingInterface const &other)  = delete;
  bool            operator==(MessagingInterface const &other)     = delete;
  bool            operator<(MessagingInterface const &other)      = delete;

private:
};

}  // namespace dmlf
}  // namespace fetch
