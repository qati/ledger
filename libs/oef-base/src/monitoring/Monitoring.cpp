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

#include "oef-base/monitoring/Monitoring.hpp"

#include <atomic>
#include <iostream>
#include <map>
#include <mutex>
#include <vector>

Monitoring::MonitoringInner *Monitoring::inner = 0;

Monitoring::Monitoring()
{
  if (!inner)
  {
    inner = new MonitoringInner;
  }
}

Monitoring::~Monitoring()
{}

Monitoring::IdType Monitoring::find(const NameType &name)
{
  if (!inner)
  {
    inner = new MonitoringInner;
  }
  return inner->get(name);
}

void Monitoring::add(IdType id, CountType delta)
{
  inner->access(id) += delta;
}

void Monitoring::set(IdType id, CountType delta)
{
  inner->access(id) = delta;
}

void Monitoring::sub(IdType id, CountType delta)
{
  inner->access(id) -= delta;
}

void Monitoring::report(ReportFunc func)
{
  for (const auto &name2id : inner->getNames())
  {
    func(name2id.first, inner->access(name2id.second));
  }
}

void Monitoring::max(IdType /*id*/, CountType /*value*/)
{}  // TODO: Useless function

Monitoring::CountType Monitoring::get(IdType id)
{
  return inner->access(id);
}