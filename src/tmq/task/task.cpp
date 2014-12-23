#include "tmq/task/task.h"

namespace tmq {

  BaseTask::BaseTask(TaskManager *task_manager, tmq::Thread *worker_thread)
    :task_manager_(task_manager),
    worker_thread_(worker_thread),
    state_(TASK_INIT){
  }

  BaseTask::~BaseTask(){

  }

  bool BaseTask::StartTask(){

    BOOST_ASSERT(state_ == TASK_INIT);
    BOOST_ASSERT(worker_thread_ != NULL);

    state_ = TASK_START;
    TaskStartMsg *start_msg = new TaskStartMsg(this);
    task_manager_->PostTaskMsg(MSG_TASK_START, start_msg);
    return true;
  }

  bool BaseTask::StopTask(){

    // TODO, maybe don't change the state_
    // state_ = TASK_DESTORING;
    DLOG(INFO) << "Call StopTask, send MSG_TASK_DESTORY_START to TaskManager";
    DeleteTaskMsg *dtm = new DeleteTaskMsg(this);
    task_manager_->PostTaskMsg(MSG_TASK_DESTORY_START, dtm);
    return true;
  }

  bool BaseTask::AddMsgListen(uint32 msg_id){

    if (task_manager_){
      return task_manager_->AddTaskListen(msg_id, this);
    }
    return false;
  }

  void BaseTask::RemoveMsgListen(uint32 msg_id){
    if (task_manager_){
      task_manager_->RemoveTaskListen(msg_id, this);
    }
  }

  void BaseTask::SendTaskMsg(uint32 id, TaskMsgData *pdata,
    bool time_sensitive){
    task_manager_->PostTaskMsg(id, pdata, time_sensitive);
  }

  void BaseTask::SendDelayedTaskMsg(int cmsDelay, uint32 id, TaskMsgData *pdata){
    task_manager_->PostDelayedTaskMsg(cmsDelay, id, pdata);
  }

  void BaseTask::ReceiveTaskMsg(uint32 id, TaskMsgData *pdata, bool time_sensitive){

    if (pdata){
      pdata->AddRef();
    }
    worker_thread_->Post(this, id, pdata, time_sensitive);
  }

  void BaseTask::ReceiveDelayedTaskMsg(int cmsDelay, uint32 id, TaskMsgData *pdata){
    worker_thread_->PostDelayed(cmsDelay, this, id, pdata);
  }

  void BaseTask::TaskEnding(){

    DLOG(INFO) << "Receive the MSG_TASK_DESOTRY_PROCESS call OnTaskDestory, and "
      << "send MSG_TASK_DESTORY_FINAL to TaskManager";
    OnTaskDestory();
    task_manager_->DestoryTask(this);
    state_ = TASK_DESTORED;
  }

  void BaseTask::OnMessage(tmq::Message *msg){
    TaskMsgData * task_msg = static_cast<TaskMsgData *>(msg->pdata);

    switch (msg->message_id)
    {
    case MSG_TASK_START:
      if (state_ != TASK_START){
        LOG(ERROR) << "The task state is not TASK_START";
        break;
      }
      if (OnTaskInit()){
        state_ = TASK_PROCESS;
      }
      else{
        StopTask();
      }
      break;
    case MSG_TASK_DESTORY_PROCESS:
      TaskEnding();
      break;
    default:
      if (state_ == TASK_PROCESS){
        OnTaskProcess(msg->message_id, task_msg);
      }
      else{
        LOG(ERROR) << "Task state is not TASK_PROCESS";
      }
      break;
    }
    if (task_msg){
      //LOG(ERROR) << msg->message_id << "\t" << (int)(task_msg);
      task_msg->Destory();
    }
  }


  ////////////////////////////////////////////////////////////////////////////////
  TaskManager::TaskManager(tmq::Thread *worker_thread)
    :worker_thread_(worker_thread)
  {
    if (!worker_thread_){
      worker_thread_ = tmq::Thread::Current();
    }
  }

  TaskManager::~TaskManager(){
    task_set_.clear();
  }

  void TaskManager::PostTaskMsg(uint32 id, TaskMsgData *pdata, bool time_sensitive){

    // Add refrence with TaskMsgData, it will be auto delete this
    if (pdata){
      pdata->AddRef();
    }
    worker_thread_->Post(this, id, pdata, time_sensitive);
  }

  void TaskManager::PostDelayedTaskMsg(int cmsDelay, uint32 id, TaskMsgData *pdata){
    worker_thread_->PostDelayed(cmsDelay, this, id, pdata);
  }

  void TaskManager::DestoryTask(BaseTask *base_task){
    DeleteTaskMsg *dtm = new DeleteTaskMsg(base_task);
    PostTaskMsg(MSG_TASK_DESTORY_FINAL, dtm);
  }

  bool TaskManager::AddTaskListen(uint32 msg_id, BaseTask *task){

    tmq::CritScope cs(&task_manager_crit_);

    // Find this task, current system only post one-one
    // TODO: guangleihe, instance of std::set<uint32, BaseTask *>
    // or std::map<uint32, std::vector<BastTask *>>
    TaskSet::iterator pos = task_set_.find(msg_id);
    if (pos != task_set_.end()){
      LOG(ERROR) << "The message type id registerd by other task";
      return false;
    }

    task_set_.insert(std::pair<uint32, BaseTask *>(msg_id, task));
    return true;
  }

  void TaskManager::RemoveTaskListen(uint32 msg_id, BaseTask *task){

    tmq::CritScope cs(&task_manager_crit_);

    // Find this task, current system only post one-one
    // TODO: guangleihe, instance of std::set<uint32, BaseTask *>
    TaskSet::iterator pos = task_set_.find(msg_id);
    if (pos == task_set_.end()){
      LOG(ERROR) << "Can't find this msg type";
    }

    task_set_.erase(pos);
  }

  void TaskManager::OnDestoryTask(TaskMsgData *msg_data){

    DLOG(INFO) << "Receive MSG_TASK_DESTORY_START, delete and "
      << "send MSG_TASK_DESTORY_PROCESS back";
    DeleteTaskMsg *dtm = static_cast<DeleteTaskMsg *>(msg_data);
    BaseTask *task = dtm->get_base_task();

    DeleteAllTaskListen(task);

    // Release the DeleteTaskMsg
    // delete dtm;
    task->ReceiveTaskMsg(MSG_TASK_DESTORY_PROCESS, msg_data);
  }

  void TaskManager::OnDestoryFinal(TaskMsgData *msg_data){

    DLOG(INFO) << "Reveice the MSG_TASK_DESTORY_FINAL, delete the task";
    DeleteTaskMsg *dtm = static_cast<DeleteTaskMsg *>(msg_data);
    BaseTask *task = dtm->get_base_task();

    // All the Tasks delete at here
    delete task;
    // delete dtm;
  }

  void TaskManager::DeleteAllTaskListen(BaseTask *base_task){

    TaskSet::iterator iter = task_set_.begin();
    while (iter != task_set_.end()){
      if (iter->second == base_task){
        task_set_.erase(iter++);
      }
      else{
        ++iter;
      }
    }
  }

  void TaskManager::DispatcherMsg(uint32 msg_id, TaskMsgData *msg_data){

    tmq::CritScope cs(&task_manager_crit_);
    TaskSet::iterator iter = task_set_.find(msg_id);
    if (iter == task_set_.end()){
      LOG(WARNING) << "Non task linsten this message";
      return;
    }

    // below commit code allow the task StopTask revice message
    //if (iter->second->IsEnable()){
    //// Msg memory manager, 
    //if (msg_data){
    //  LOG(ERROR) << msg_id;
    //  msg_data->AddRef();
    //}
    iter->second->ReceiveTaskMsg(msg_id, msg_data);
    //}
  }

  void TaskManager::OnMessage(tmq::Message *msg){

    BOOST_ASSERT(tmq::Thread::Current() == worker_thread_);

    //LOG(INFO) << "msg id is " << msg->message_id;

    TaskMsgData * task_msg = static_cast<TaskMsgData *>(msg->pdata);
    switch (msg->message_id)
    {
    case MSG_TASK_DESTORY_START:
      OnDestoryTask(task_msg);
      break;
    case MSG_TASK_DESTORY_FINAL:
      OnDestoryFinal(task_msg);
      break;
    case MSG_TASK_START:
    {
      //LOG(ERROR) << msg->message_id;
      TaskStartMsg *start_msg = static_cast<TaskStartMsg *>(msg->pdata);
      start_msg->base_task_->ReceiveTaskMsg(MSG_TASK_START);
      break;
    }
    default:
      DispatcherMsg(msg->message_id, task_msg);
      break;
    }
    if (task_msg){
      task_msg->Destory();
    }
  }
}; // namesapce tmq