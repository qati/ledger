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

#include "oef-core/oef-functions/InitialHandshakeTaskFactory.hpp"
#include "oef-core/oef-functions/OefHeartbeatTask.hpp"

#include "oef-core/comms/OefAgentEndpoint.hpp"
#include "oef-core/oef-functions/OefFunctionsTaskFactory.hpp"
#include "oef-core/tasks-base/TSendProtoTask.hpp"
#include "oef-messages/agent.hpp"

void InitialHandshakeTaskFactory::processMessage(ConstCharArrayBuffer &data)
{
  fetch::oef::pb::Agent_Server_ID     id_pb;
  fetch::oef::pb::Agent_Server_Answer answer_pb;

  ConstCharArrayBuffer buff(data);

  try
  {
    switch (state)
    {
    case WAITING_FOR_Agent_Server_ID:
    {
      IOefTaskFactory::read(id_pb, buff);
      public_key_ = id_pb.public_key();

      FETCH_LOG_INFO(LOGGING_NAME, "Sending phrase to agent: ", public_key_);
      std::cout << "Sending phrase to agent: " << public_key_ << std::endl;

      auto phrase = std::make_shared<fetch::oef::pb::Server_Phrase>();
      phrase->set_phrase("RandomlyGeneratedString");

      auto senderTask =
          std::make_shared<TSendProtoTask<fetch::oef::pb::Server_Phrase>>(phrase, getEndpoint());
      senderTask->submit();
      state = WAITING_FOR_Agent_Server_Answer;
    }
    break;
    case WAITING_FOR_Agent_Server_Answer:
    {
      IOefTaskFactory::read(answer_pb, buff);
      auto connected_pb = std::make_shared<fetch::oef::pb::Server_Connected>();
      connected_pb->set_status(true);

      getEndpoint()->capabilities.will_heartbeat = answer_pb.capability_bits().will_heartbeat();

      FETCH_LOG_INFO(LOGGING_NAME, "Agent ", public_key_, " verified, moving to OefFunctions...");

      auto senderTask = std::make_shared<TSendProtoTask<fetch::oef::pb::Server_Connected>>(
          connected_pb, getEndpoint());
      senderTask->submit();
      state = WAITING_FOR_Agent_Server_Answer;

      agents_->add(public_key_, getEndpoint());

      getEndpoint()->karma.upgrade("", public_key_);
      getEndpoint()->karma.perform("login");

      if (getEndpoint()->capabilities.will_heartbeat)
      {
        auto heartbeat = std::make_shared<OefHeartbeatTask>(getEndpoint());
        heartbeat->submit();
      }

      getEndpoint()->setState("loggedin", true);

      successor(
          std::make_shared<OefFunctionsTaskFactory>(core_key_, agents_, public_key_, outbounds));
    }
    break;
    }
  }
  catch (std::exception &ex)
  {
    FETCH_LOG_ERROR(LOGGING_NAME, "processMessage  -- ", ex.what());
    throw ex;
  }
  catch (...)
  {
    FETCH_LOG_ERROR(LOGGING_NAME, "processMessage exception");
    throw;
    // ignore the error.
  }
}