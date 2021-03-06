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

#include "oef-core/karma/IKarmaPolicy.hpp"

#include <memory>

#include "oef-base/threading/Task.hpp"
#include "oef-core/karma/KarmaAccount.hpp"

// because friendship is not heritable.
void IKarmaPolicy::changeAccountNumber(KarmaAccount *acc, std::size_t number)
{
  acc->id = number;
}

// because friendship is not heritable.
KarmaAccount IKarmaPolicy::mkAccount(std::size_t number, const std::string &name)
{
  return KarmaAccount(number, this, name);
}
