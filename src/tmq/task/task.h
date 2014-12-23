#ifndef TASK_MANAGER_TASK_H_
#define TASK_MANAGER_TASK_H_

#include <string>
#include <map>
#include "tmq/base/boostincludes.h"
#include "tmq/base/basictypes.h"
#include "tmq/base/thread.h"

#include "tmq/task/basedefine.h"

namespace tmq {

  class TaskManager;

  // One task only belong to a singal thread (named worker_thread at here), but
  // a thread can have multiple tasks.
  // All of the task managed by TaskManager, the TaskManager revice the message 
  // data and dispatch to corresponding task.
  // A task can listen more than one different types of message, when the task
  // destory, it must to remove this task listen.

  // In addition the blown points need move attention:

  // 1. The BaseTask create by user, but it delete by TaskManager
  // 2. After create BaseTask object, the next step is to call the StartTask.
  //   If you want to end this task, call StopTask.
  // 3. the pure virual method, TaskInit, TaskProcess, TaskDestory, only called
  //   by the worker_thread. So attention with the thread safety
  class BaseTask : public tmq::MessageHandler{
  public:
    BaseTask(TaskManager *task_manager, tmq::Thread *worker_thread);
    virtual ~BaseTask();

    // The StartTask only called once with a task
    // The StartTask never return false
    bool StartTask();

    // The method will be destory the task object, for more information 
    // see the document.
    // Call the stop class immediately stop the task process
    bool StopTask();

    // Test the task state, only TASK_PROCESS alown to process message
    bool IsEnable() const{
      return state_ == TASK_PROCESS;
    }

    // The method used by sub class to check whether the task is run at correct
    // thread
    tmq::Thread *get_worker_thread() const{
      return worker_thread_;
    }

    // See the calss head commit, each task can listen multiple message type
    // TODO: guangleihe, change the strategy, 
    bool AddMsgListen(uint32 msg_id);

    // Remove a message listen
    // TODO: guangleihe, when you stop this task, it delete all the task listen.
    // so, maybe can't need this method
    void RemoveMsgListen(uint32 msg_id);

    // Send a noraml message to the task manager
    // time_sensitive, indicat that the message time sensitive, never use it
    void SendTaskMsg(uint32 id = 0, TaskMsgData *pdata = NULL,
      bool time_sensitive = false);
    // Send a delayed message to the task manager
    // cmsDelay is the milliscond time
    void SendDelayedTaskMsg(int cmsDelay, uint32 id = 0,
      TaskMsgData *pdata = NULL);

    // Pure virual method
    // -------------------------------------------------------------------------
    // GetTaskName, return the subcall name, used to debug
#ifdef _DEBUG
    virtual const std::string GetTaskName() = 0;
#endif

    // When you start the task, the next thread message will be call this TaskInit
    // Initialization the task resouce at this method, 
    // Return "false", the task will be destory.
    // Return "true", is the normal station
    // You can add listen at here
    virtual bool OnTaskInit() = 0;

    // When the listen message posted, the method will be get the message.
    // Process the message at this method
    virtual void OnTaskProcess(uint32 msg_id, TaskMsgData *msg_data) = 0;

    // When stop task or other exception occur, this method will be called.
    // Release the resouce at this method, but don't delete task at here
    virtual void OnTaskDestory() = 0;
    // -------------------------------------------------------------------------

  protected:
    // Post a message to the task, it run by worker thread.
    // This method only called by taskManager, don't call it, unless you known
    // what happend
    void ReceiveTaskMsg(uint32 id = 0, TaskMsgData *pdata = NULL,
      bool time_sensitive = false);
    // Post a delayed message to the task, see above commit
    void ReceiveDelayedTaskMsg(int cmsDelay, uint32 id = 0, TaskMsgData *pdata = NULL);

  private:
    // Allow the TaskManager, accees the protected method
    friend class TaskManager;

    // Help function with StopTask, when the method called,
    // the state is TASK_DESTORING
    void TaskEnding();

    // Inheritance with tmq::MessageHandler
    // To receive thread message
    virtual void OnMessage(tmq::Message *msg);

    enum{
      TASK_INIT,        // Initilize state
      TASK_START,       // First call StartTask, get this state 
      TASK_PROCESS,     // Initilize complete, process message state
      // TASK_DESTORING,   // First call StopTask, getting this state
      TASK_DESTORED,    // The system call TaskDestory, getting this state
    }state_;

    TaskManager *task_manager_;
    tmq::Thread *worker_thread_;
  };

  // The TaskManager object, manager all of the task object.
  // dispatcher the task message
  class TaskManager : public tmq::MessageHandler{
  public:
    TaskManager(tmq::Thread *worker_thread = NULL);
    virtual ~TaskManager();

    // Send message to the TaskManager
    void PostTaskMsg(uint32 id = 0, TaskMsgData *pdata = NULL,
      bool time_sensitive = false);
    // Send delayed message to the TaskManager
    void PostDelayedTaskMsg(int cmsDelay, uint32 id = 0,
      TaskMsgData *pdata = NULL);

  protected:
    // The method will destory the input base_task object at MSG_TASK_DESTORING
    // state
    void DestoryTask(BaseTask *base_task);

    bool AddTaskListen(uint32 msg_id, BaseTask *task);
    void RemoveTaskListen(uint32 msg_id, BaseTask *task);

  private:

    // Allow the BaseTask to access the protected method
    friend class BaseTask;

    // Belown is the help function, don't change it
    void OnDestoryTask(TaskMsgData *msg_data);
    void OnDestoryFinal(TaskMsgData *msg_data);

    // Delete the base_task listen lists
    void DeleteAllTaskListen(BaseTask *base_task);
    void DispatcherMsg(uint32 msg_id, TaskMsgData *msg_data);

    // Inheritance with tmq::MessageHandler
    // To receive thread message
    virtual void OnMessage(tmq::Message *msg);

  private:
    typedef std::map<uint32, BaseTask *> TaskSet;
    TaskSet task_set_;
    tmq::Thread *worker_thread_;
    tmq::CriticalSection task_manager_crit_;
  };

}; // namespace tmq

#endif // TASK_MANAGER_TASK_H_