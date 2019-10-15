#pragma once

#include <memory>
#include <queue>
#include <utility>

#include "Node.hpp"
#include "NodeExecutorTask.hpp"
#include "logging/logging.hpp"
#include "oef-base/threading/StateMachineTask.hpp"

class DapManager;

class WithLateDapExecutorTask : virtual public StateMachineTask<WithLateDapExecutorTask>,
                                virtual public NodeExecutorTask
{
public:
  using StateResult    = typename StateMachineTask<WithLateDapExecutorTask>::Result;
  using EntryPoint     = typename StateMachineTask<WithLateDapExecutorTask>::EntryPoint;
  using MessageHandler = std::function<void(std::shared_ptr<IdentifierSequence>)>;
  using ErrorHandler =
      std::function<void(const std::string &, const std::string &, const std::string &)>;
  using Parent = StateMachineTask<WithLateDapExecutorTask>;

  static constexpr char const *LOGGING_NAME = "WithLateDapExecutorTask";

  WithLateDapExecutorTask(std::shared_ptr<NodeExecutorTask> task,
                          std::vector<Node::DapMemento>     late_mementos,
                          std::shared_ptr<DapManager>       dap_manager)
    : Parent()
    , main_task_{std::move(task)}
    , late_mementos_{std::move(late_mementos)}
    , last_output_{nullptr}
    , dap_manager_{std::move(dap_manager)}
  {
    entryPoint.push_back(&WithLateDapExecutorTask::Setup);
    entryPoint.push_back(&WithLateDapExecutorTask::DoLateMementos);
    entryPoint.push_back(&WithLateDapExecutorTask::Done);
    this->entrypoints = entryPoint.data();
    this->state       = this->entrypoints[0];
    this->SetSubClass(this);
    FETCH_LOG_INFO(LOGGING_NAME, "Task created.");
  }

  virtual ~WithLateDapExecutorTask()
  {
    FETCH_LOG_INFO(LOGGING_NAME, "Task gone.");
  }

  WithLateDapExecutorTask(const WithLateDapExecutorTask &other) = delete;
  WithLateDapExecutorTask &operator=(const WithLateDapExecutorTask &other)  = delete;
  bool                     operator==(const WithLateDapExecutorTask &other) = delete;
  bool                     operator<(const WithLateDapExecutorTask &other)  = delete;

  MessageHandler GetMessageHandler()
  {
    auto this_sp = this->template shared_from_base<WithLateDapExecutorTask>();
    std::weak_ptr<WithLateDapExecutorTask> this_wp = this_sp;

    return [this_wp](std::shared_ptr<IdentifierSequence> response) {
      auto sp = this_wp.lock();
      if (sp)
      {
        sp->last_output_ = std::move(response);
      }
      else
      {
        FETCH_LOG_ERROR(LOGGING_NAME, "No shared pointer to WithLateDapExecutorTask");
      }
    };
  }

  ErrorHandler GetErrorHandler()
  {
    auto this_sp = this->template shared_from_base<WithLateDapExecutorTask>();
    std::weak_ptr<WithLateDapExecutorTask> this_wp = this_sp;

    return [this_wp](const std::string &dap_name, const std::string &path, const std::string &msg) {
      auto sp = this_wp.lock();
      if (sp)
      {
        sp->last_output_ = nullptr;
        if (sp->errorHandler)
        {
          sp->errorHandler(dap_name, path, msg);
          sp->wake();
        }
      }
      else
      {
        FETCH_LOG_ERROR(LOGGING_NAME, "No shared pointer to WithLateDapExecutorTask");
      }
    };
  }

  StateResult Setup()
  {
    main_task_->SetMessageHandler(GetMessageHandler());
    main_task_->SetErrorHandler(GetErrorHandler());
    main_task_->submit();

    auto this_sp = this->template shared_from_base<WithLateDapExecutorTask>();
    std::weak_ptr<WithLateDapExecutorTask> this_wp = this_sp;

    if (main_task_->MakeNotification()
            .Then([this_wp]() {
              auto sp = this_wp.lock();
              if (sp)
              {
                sp->MakeRunnable();
              }
            })
            .Waiting())
    {
      FETCH_LOG_INFO(LOGGING_NAME, "Sleeping");
      return StateResult(1, DEFER);
    }

    FETCH_LOG_INFO(LOGGING_NAME, "NOT Sleeping");
    return StateResult(1, COMPLETE);
  }

  StateResult DoLateMementos()
  {
    if (!last_output_)
    {
      FETCH_LOG_ERROR(LOGGING_NAME, "No last output set");
      wake();
      return StateResult(0, ERRORED);
    }

    auto task = std::make_shared<MementoExecutorTask>(late_mementos_, last_output_, dap_manager_);

    task->SetMessageHandler(GetMessageHandler());

    task->SetErrorHandler(GetErrorHandler());

    task->submit();

    auto this_sp = this->template shared_from_base<WithLateDapExecutorTask>();
    std::weak_ptr<WithLateDapExecutorTask> this_wp = this_sp;

    if (task->MakeNotification()
            .Then([this_wp]() {
              auto sp = this_wp.lock();
              if (sp)
              {
                sp->MakeRunnable();
              }
            })
            .Waiting())
    {
      FETCH_LOG_INFO(LOGGING_NAME, "Sleeping");
      return StateResult(2, DEFER);
    }

    FETCH_LOG_INFO(LOGGING_NAME, "NOT Sleeping");
    return StateResult(2, COMPLETE);
  }

  StateResult Done()
  {
    if (messageHandler)
    {
      messageHandler(last_output_);
    }
    wake();
    return StateResult(0, COMPLETE);
  }

  virtual void SetMessageHandler(MessageHandler msgHandler)
  {
    messageHandler = std::move(msgHandler);
  }

  virtual void SetErrorHandler(ErrorHandler errHandler)
  {
    errorHandler = std::move(errHandler);
  }

public:
  ErrorHandler   errorHandler;
  MessageHandler messageHandler;

protected:
  std::shared_ptr<NodeExecutorTask>   main_task_;
  std::vector<Node::DapMemento>       late_mementos_;
  std::shared_ptr<IdentifierSequence> last_output_;
  std::shared_ptr<DapManager>         dap_manager_;

  std::vector<EntryPoint> entryPoint;
};