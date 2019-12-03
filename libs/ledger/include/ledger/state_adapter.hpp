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

#include "ledger/storage_unit/storage_unit_interface.hpp"
#include "vm/io_observer_interface.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace fetch {

namespace byte_array {
class ConstByteArray;
}

namespace ledger {

/**
 * Adapter between the VM IO interface and the main ledger state database.
 */
class StateAdapter : public vm::IoObserverInterface
{
public:
  using ScopeType = byte_array::ConstByteArray;

  // Resource Mapping
  static storage::ResourceAddress CreateAddress(ScopeType const &                scope,
                                                byte_array::ConstByteArray const &key);

  enum class Mode
  {
    READ_ONLY,
    READ_WRITE
  };

  // Construction / Destruction
  StateAdapter(StorageInterface &storage, ScopeType scope);
  ~StateAdapter() override = default;

  /// @name Io Observer Interface
  /// @{
  Status Read(std::string const &key, void *data, uint64_t &size) override;
  Status Write(std::string const &key, void const *data, uint64_t size) override;
  Status Exists(std::string const &key) override;
  /// @}

  void PushContext(ScopeType const &scope);
  void PopContext();

protected:
  ScopeType CurrentScope() const;

  // Protected construction
  StateAdapter(StorageInterface &storage, ScopeType scope, Mode mode);

  StorageInterface &      storage_;
  std::vector<ScopeType>  scope_;
  Mode const              mode_;
};

}  // namespace ledger
}  // namespace fetch
