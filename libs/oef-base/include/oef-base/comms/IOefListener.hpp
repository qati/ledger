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

#include <functional>
#include <memory>

template <class IOefTaskFactory, class OefEndpoint>
class IOefListener
{
public:
  using FactoryCreator =
      std::function<std::shared_ptr<IOefTaskFactory>(std::shared_ptr<OefEndpoint>)>;

  /// @{
  IOefListener()                          = default;
  IOefListener(IOefListener const &other) = delete;
  virtual ~IOefListener()                 = default;
  /// @}

  /// @{
  IOefListener &operator=(IOefListener const &other)  = delete;
  bool          operator==(IOefListener const &other) = delete;
  bool          operator<(IOefListener const &other)  = delete;
  /// @}

  // TODO: Should be private variable with accessor function
  FactoryCreator factoryCreator;

private:
};