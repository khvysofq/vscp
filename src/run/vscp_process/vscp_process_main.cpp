#include <iostream>
#include "vzipc/base/vzipcserver.h"
#include "tmq/base/socketserver.h"
#include "tmq/task/task.h"
#include "glog/logging.h"

class EventIpcServer : public tmq::SocketServer{
public:
  EventIpcServer(vzipc::VZSocketServer *socket_server)
    :socket_server_(socket_server){
  }
  // Sleeps until:
  //  1) cms milliseconds have elapsed (unless cms == kForever)
  //  2) WakeUp() is called
  // While sleeping, I/O is performed if process_io is true.
  virtual bool Wait(int cms, bool process_io) {
    socket_server_->Wait(cms);
    return true;
  }

  // Causes the current wait (if one is in progress) to wake up.
  virtual void WakeUp() {
    socket_server_->WakeUp();
  }
private:
  vzipc::VZSocketServer *socket_server_;
};

class TestMessageData : public tmq::TaskMsgData{
public:
  TestMessageData(const std::string &msg) :msg_(msg){
  }
  ~TestMessageData(){
    //LOG(ERROR) << "Destory Test Message Data ... ...";
  }
public:
  std::string msg_;
};

const int MSG_TEST_DATA = tmq::MSG_USER + 1;

class IpcServerTask : public tmq::BaseTask {
public:
  IpcServerTask(vzipc::VZSocketServer *socket_server,
    tmq::TaskManager *task_manager, tmq::Thread *worker_thread)
    :tmq::BaseTask(task_manager, worker_thread),
    socket_server_(socket_server),
    ipc_socket_(NULL){

  }
  virtual ~IpcServerTask(){
    DLOG(INFO) << "Test with delete task";
  }

#ifdef _DEBUG
  virtual const std::string GetTaskName(){
    BOOST_ASSERT(tmq::Thread::Current() == get_worker_thread());
    return "TestTask";
  }
#endif
  virtual bool OnTaskInit(){
    BOOST_ASSERT(tmq::Thread::Current() == get_worker_thread());
    AddMsgListen(MSG_TEST_DATA);
    DLOG(INFO) << "Task Init";
    Start();
    return true;
  }

  virtual void OnTaskProcess(uint32 msg_id, tmq::TaskMsgData *msg_data){
    BOOST_ASSERT(tmq::Thread::Current() == get_worker_thread());
    switch (msg_id)
    {
    case MSG_TEST_DATA:
    {
      TestMessageData *tmd = static_cast<TestMessageData *>(msg_data);
      DLOG(INFO) << tmd->msg_;
      //delete tmd;
      //EndTask();
      break;
    }
    default:
      break;
    }
  }

  virtual void OnTaskDestory(){
    DLOG(INFO) << "Task Destory";
  }
  //////////////////////////////////////////////////////////////////////////////
  void Start(){
    BOOST_ASSERT(tmq::Thread::Current() == get_worker_thread());
    ipc_socket_ = socket_server_->CreateVZServerSocket();

    ipc_socket_->SignalPacketRecived.connect(
      boost::bind(&IpcServerTask::OnPacketRecived, this, _1, _2, _3));
    ipc_socket_->SignalStateChange.connect(
      boost::bind(&IpcServerTask::OnStateChange, this, _1, _2));
    ipc_socket_->SignalSocketError.connect(
      boost::bind(&IpcServerTask::OnSocketError, this, _1, _2));

    if (vzipc::VZ_SOCKET_ERROR == ipc_socket_->Bind(5299)){
      CHECK(0);
    }
    //ipc_socket_->AsyncWrite("Hello", 5);
  }

  void OnPacketRecived(vzipc::VzIpcSocket *socket, const char* data, int len){
    BOOST_ASSERT(tmq::Thread::Current() == get_worker_thread());
    LOG(INFO) << ".";
    CHECK(socket == ipc_socket_);
    ipc_socket_->AsyncWrite(data, len);
  }

  void OnStateChange(vzipc::VzIpcSocket *socket, vzipc::ConnState state){
    BOOST_ASSERT(tmq::Thread::Current() == get_worker_thread());
    LOG(INFO) << __FUNCTION__;

  }

  void OnSocketError(vzipc::VzIpcSocket *socket, int err){
    BOOST_ASSERT(tmq::Thread::Current() == get_worker_thread());
    LOG(INFO) << __FUNCTION__;
  }
private:
  vzipc::VZSocketServer *socket_server_;
  vzipc::VzIpcSocket *ipc_socket_;
};

class TestTask : public tmq::BaseTask{
public:
  TestTask(tmq::TaskManager *task_manager, tmq::Thread *worker_thread)
    :tmq::BaseTask(task_manager, worker_thread), count_(0){

  }
  virtual ~TestTask(){
    DLOG(INFO) << "Test with delete task";
  }

#ifdef _DEBUG
  virtual const std::string GetTaskName(){
    BOOST_ASSERT(tmq::Thread::Current() == get_worker_thread());
    return "TestTask";
  }
#endif
  virtual bool OnTaskInit(){
    BOOST_ASSERT(tmq::Thread::Current() == get_worker_thread());
    AddMsgListen(MSG_TEST_DATA + 1);
    DLOG(INFO) << "Task Init";
    return true;
  }
  virtual void OnTaskProcess(uint32 msg_id, tmq::TaskMsgData *msg_data){
    BOOST_ASSERT(tmq::Thread::Current() == get_worker_thread());
    DLOG(INFO) << "Task Process " << ++count_;
    switch (msg_id)
    {
    case MSG_TEST_DATA:
    {
      TestMessageData *tmd = static_cast<TestMessageData *>(msg_data);
      DLOG(INFO) << tmd->msg_;
      //delete tmd;
      //EndTask();
      break;
    }
    default:
      break;
    }
  }
  virtual void OnTaskDestory(){
    BOOST_ASSERT(tmq::Thread::Current() == get_worker_thread());
    DLOG(INFO) << "Task Destory";
  }
private:
  int count_;
};

int main(int argc, char *argv[]){
  google::InitGoogleLogging(argv[0]);
  FLAGS_stderrthreshold = 0;
  FLAGS_colorlogtostderr = true;

  vzipc::VZSocketServer *socket_server = vzipc::InitVZNetwork();
  EventIpcServer event_ipc_server(socket_server);

  tmq::Thread *singal_thread = tmq::Thread::Current();
  tmq::Thread *worker_thread1 = new tmq::Thread(&event_ipc_server);

  tmq::Thread *worker_thread2 = new tmq::Thread();

  tmq::TaskManager *task_manager = new tmq::TaskManager(singal_thread);
  tmq::BaseTask *testTask1 = new IpcServerTask(socket_server,
    task_manager, worker_thread1);

  tmq::BaseTask *testTask2 = new TestTask(task_manager, worker_thread1);

  testTask1->StartTask();
  testTask2->StartTask();
  worker_thread1->Start();
  worker_thread2->Start();

  singal_thread->Run();

  return 0;
}
