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

#include "dmlf/message_system/muddle_messaging.hpp"
#include "core/service_ids.hpp"


namespace fetch {
namespace dmlf {

MuddleMessaging::MuddleMessaging(MuddleMessaging::MuddlePtr ptr, uint16_t server_port, uint16_t client_port)
  : muddle_{ptr}
  , protocol_{this}
{
  muddle_->SetPeerSelectionMode(muddle::PeerSelectionMode::KADEMLIA);
  std::string server = "tcp://127.0.0.1:";
  server += std::to_string(server_port);

  muddle_->Start({server}, {client_port});

  client_ = std::make_shared<RpcClient>("Client", muddle_->GetEndpoint(), SERVICE_DMLF, CHANNEL_RPC);
  server_ = std::make_shared<Server>(muddle_->GetEndpoint(), SERVICE_DMLF, CHANNEL_RPC);
  server_->Add(RPC_DMLF, protocol_.get());
}


}  // namespace dmlf
}  // namespace fetch