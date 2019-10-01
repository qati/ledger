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

#include "network/fetch_asio.hpp"

#include <iostream>
#include <vector>

class RingBuffer
{
public:
  using Mutex          = std::mutex;
  using Lock           = std::lock_guard<Mutex>;
  using buffer         = asio::const_buffer;
  using mutable_buffer = asio::mutable_buffer;
  using byte           = std::uint8_t;
  using SignalReady    = std::function<void()>;

protected:
  void ignored()
  {}

public:
  RingBuffer(RingBuffer const &other) = delete;
  RingBuffer &operator=(RingBuffer const &other)  = delete;
  bool        operator==(RingBuffer const &other) = delete;
  bool        operator<(RingBuffer const &other)  = delete;

  RingBuffer(std::size_t size)
  {
    this->size = size;
    store      = (byte *)malloc(size);
    clear();
  }

  virtual ~RingBuffer()
  {
    free(store);
  }

  void clear(void)
  {
    this->freeSpace = this->size;
    this->writep    = 0;
    this->readp     = 0;
  }

  bool empty(void) const
  {
    return getFreeSpace() == size;
  }

  mutable_buffer getSpaceBuffer()
  {
    Lock lock(mut);
    if (getFreeSpace() == 0)
      return mutable_buffer(0, 0);
    return mutable_buffer(addressOf(writep % size),
                          std::min(writep + lockless_getFreeSpace(), size) - writep);
  }

  buffer getDataBuffer()
  {
    Lock lock(mut);
    if (getFreeSpace() == size)
      return buffer(0, 0);
    return buffer(addressOf(readp % size),
                  std::min(readp + lockless_getDataAvailable(), size) - readp);
  }

  std::vector<mutable_buffer> getSpaceBuffers()
  {
    Lock                        lock(mut);
    std::vector<mutable_buffer> r;
    if (lockless_getFreeSpace() > 0)
    {
      std::size_t buffer1size = std::min(writep + lockless_getFreeSpace(), size) - writep;
      std::size_t buffer2size = lockless_getFreeSpace() - buffer1size;
      r.push_back(mutable_buffer(addressOf(writep), buffer1size));
      if (buffer2size)
      {
        r.push_back(mutable_buffer(addressOf(0), buffer2size));
      }
    }
    return r;
  }

  std::vector<buffer> getDataBuffers()
  {
    Lock                lock(mut);
    std::vector<buffer> r;
    if (lockless_getDataAvailable() < size)
    {
      std::size_t buffer1size = std::min(readp + lockless_getDataAvailable(), size) - readp;
      std::size_t buffer2size = lockless_getDataAvailable() - buffer1size;
      r.push_back(buffer(addressOf(readp), buffer1size));
      if (buffer2size)
      {
        r.push_back(buffer(addressOf(0), buffer2size));
      }
    }
    return r;
  }

  void markSpaceUsed(std::size_t amount)
  {
    std::size_t prevAvail = 0;
    {
      Lock lock(mut);
      writep += amount;
      writep %= size;
      prevAvail = lockless_getDataAvailable();
      freeSpace -= amount;
    }
    if (!prevAvail)
    {
      signalDataReady();
    }
  }

  void markDataUsed(std::size_t amount)
  {
    std::size_t prevSpace = 0;
    {
      Lock lock(mut);
      readp += amount;
      readp %= size;
      prevSpace = lockless_getFreeSpace();
      freeSpace += amount;
    }
    if (!prevSpace)
    {
      signalSpaceReady();
    }
  }

  const void *addressOf(std::size_t index) const
  {
    return (byte *)store + index;
  }
  void *addressOf(std::size_t index)
  {
    return (byte *)store + index;
  }

  std::size_t getFreeSpace() const
  {
    Lock lock(mut);
    return lockless_getFreeSpace();
  }
  std::size_t getDataAvailable() const
  {
    Lock lock(mut);
    return lockless_getDataAvailable();
  }

  bool hasFreeSpace() const
  {
    Lock lock(mut);
    return lockless_getFreeSpace() > 0;
  }
  bool hasDataAvailable() const
  {
    Lock lock(mut);
    return lockless_getDataAvailable() > 0;
  }

protected:
  std::size_t   size;
  std::size_t   freeSpace;
  std::size_t   readp, writep;
  byte *        store;
  mutable Mutex mut;

  SignalReady signalSpaceReady = []() {};
  SignalReady signalDataReady  = []() {};

  std::size_t lockless_getFreeSpace() const
  {
    return freeSpace;
  }
  std::size_t lockless_getDataAvailable() const
  {
    return size - freeSpace;
  }
};