
#include "network/generics/network_node_core.hpp"
#include "network/swarm/swarm_agent_api_impl.hpp"
#include "network/swarm/swarm_http_interface.hpp"
#include "network/swarm/swarm_random.hpp"

#include <string>
#include <time.h>
#include <unistd.h>

namespace fetch {
namespace swarm {

class PythonWorker
{
public:
  fetch::network::ThreadPool tm_;
  using mutex_type = std::recursive_mutex;
  using lock_type  = std::lock_guard<std::recursive_mutex>;
  mutex_type                                       mutex_;
  std::shared_ptr<fetch::network::NetworkNodeCore> nnCore_;

  virtual void Start()
  {
    lock_type lock(mutex_);
    tm_->Start();
  }

  virtual void Stop()
  {
    lock_type lock(mutex_);
    tm_->Stop();
  }

  void UseCore(std::shared_ptr<fetch::network::NetworkNodeCore> nnCore) { nnCore_ = nnCore; }

  template <typename F>
  void Post(F &&f)
  {
    tm_->Post(f);
  }
  template <typename F>
  void Post(F &&f, uint32_t milliseconds)
  {
    tm_->Post(f, milliseconds);
  }

  PythonWorker() { tm_ = fetch::network::MakeThreadPool(1); }

  virtual ~PythonWorker() { Stop(); }
};

}  // namespace swarm
}  // namespace fetch