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

#include "variant/variant.hpp"
#include "dmlf/execution/execution_error_message.hpp"

#include <memory>
#include <vector>

namespace fetch {
namespace dmlf {

class Message
{
public:
  using State        = uint8_t;
  using Uri          = std::string;
  using Variant      = variant::Variant;
  using Payload      = std::vector<Variant>;
  using Error        = ExecutionErrorMessage;
  using ErrorPtr     = std::shared_ptr<Error>;

  Message()
    : state_{0}
    , uri_ {""}
    , error_{nullptr}
    , payload_{}
  {}
  virtual ~Message() = default;

  Message(Message const &other) = delete;
  Message &operator=(Message const &other)  = delete;
  bool            operator==(Message const &other) = delete;
  bool            operator<(Message const &other)  = delete;

private:
  State state_;
  Uri uri_;
  ErrorPtr error_;
  Payload payload_;

  template <typename T, typename D>
  friend struct serializers::MapSerializer;
};

}  // namespace dmlf


namespace serializers {

template <typename D>
struct MapSerializer<fetch::dmlf::Message, D>
{
public:
  using Type       = fetch::dmlf::Message;
  using DriverType = D;

  static uint8_t const STATE        = 1;
  static uint8_t const URI          = 2;
  static uint8_t const ERROR        = 3;
  static uint8_t const PAYLOAD      = 4;

  template <typename Constructor>
  static void Serialize(Constructor &map_constructor, Type const &obj)
  {
    auto map = map_constructor(4);
    map.Append(STATE, obj.state_);
    map.Append(URI,   obj.uri_);
    if (obj.state_ != 0 && obj.error_)
    {
      map.Append(ERROR,  *(obj.error_));
    }
    map.Append(PAYLOAD, obj.payload_);
  }

  template <typename MapDeserializer>
  static void Deserialize(MapDeserializer &map, Type &obj)
  {
    map.ExpectKeyGetValue(STATE, obj.state_);
    map.ExpectKeyGetValue(URI, obj.uri_);
    map.ExpectKeyGetValue(PAYLOAD, obj.payload_);
    if (obj.state_ != 0)
    {
      auto err_ptr = std::make_shared<Type::Error>();
      map.ExpectKeyGetValue(ERROR, *err_ptr);
      obj.error_ = err_ptr;
    }
  }
};

}  // namespace serializers

}  // namespace fetch
