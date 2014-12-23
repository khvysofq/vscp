#ifndef TASK_BASE_DEFINE_H_
#define TASK_BASE_DEFINE_H_

#include "tmq/base/messagequeue.h"

namespace tmq {

  const int MSG_USER = 0x0F;

  const int MSG_TASK_START = MSG_USER - 1;
  const int MSG_TASK_DESTORY = MSG_USER - 2;

  const int MSG_TASK_DESTORY_START = MSG_USER - 2;
  const int MSG_TASK_DESTORY_PROCESS = MSG_USER - 3;
  const int MSG_TASK_DESTORY_FINAL = MSG_USER - 4;

  class BaseTask;

  // TODO(Guangleihe): the best way is to use shared_prt<>, but the implementation
  // of libjingle thread message loop can't translate a smart pointer
  // fix this problem by smart pointer
  class TaskMsgData : public tmq::MessageData{
  public:
    TaskMsgData() :ref_count_(0){}
    virtual ~TaskMsgData(){
    }
    void AddRef(){
      tmq::CritScope cs(&crit_);
      ref_count_++;
    }
    void Destory(){
      tmq::CritScope cs(&crit_);
      ref_count_--;
      if (ref_count_){
        // Do nothing
      }
      else{
        delete this;
      }
    }
  private:
    int ref_count_;
    tmq::CriticalSection crit_;
  };

  class TaskStartMsg : public TaskMsgData{
  public:
    TaskStartMsg(BaseTask *base_task) : base_task_(base_task){}
    virtual ~TaskStartMsg(){}
    BaseTask *base_task_;
  };

  // Delete message data
  class DeleteTaskMsg : public TaskMsgData {
  public:
    DeleteTaskMsg(BaseTask *base_task) :base_task_(base_task){}
    BaseTask *get_base_task() const{
      return base_task_;
    }
    virtual ~DeleteTaskMsg(){};
  private:
    BaseTask *base_task_;
  };

}; // namespace tmq
#endif // TASK_BASE_DEFINE_H_