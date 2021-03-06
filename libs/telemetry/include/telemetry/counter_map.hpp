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

#include "telemetry/measurement.hpp"
#include "telemetry/telemetry.hpp"

#include <mutex>
#include <unordered_map>

namespace fetch {
namespace telemetry {

class CounterMap : public Measurement
{
public:
  // Construction / Destruction
  CounterMap(std::string name, std::string description, Labels const &labels = Labels{});
  CounterMap(CounterMap const &) = delete;
  CounterMap(CounterMap &&)      = delete;
  ~CounterMap() override         = default;

  /// @name Accessors
  /// @{
  void Increment(Labels const &keys);
  /// @}

  void ToStream(OutputStream &stream) const override;

  // Operators
  CounterMap &operator=(CounterMap const &) = delete;
  CounterMap &operator=(CounterMap &&) = delete;

private:
  using Counters = std::unordered_map<Labels, CounterPtr>;

  CounterPtr LookupCounter(Labels const &keys);

  mutable std::mutex lock_;
  Counters           counters_;
};

}  // namespace telemetry
}  // namespace fetch
