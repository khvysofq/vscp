#include <iostream>
#include "tmq/base/thread.h"
#include "glog/logging.h"
#include "tmq/task/task.h"

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
    return "TestTask";
  }
#endif
  virtual bool OnTaskInit(){
    AddMsgListen(MSG_TEST_DATA);
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
    DLOG(INFO) << "Task Destory";
  }
private:
  int count_;
};

void TestWithSignalTask(){

  const int MAX_DATA = 10;

  tmq::Thread *singal_thread = tmq::Thread::Current();
  tmq::Thread *worker_thread1 = new tmq::Thread();
  tmq::Thread *worker_thread2 = new tmq::Thread();

  tmq::TaskManager *task_manager = new tmq::TaskManager(singal_thread);
  tmq::BaseTask *testTask1 = new TestTask(task_manager, worker_thread1);
  tmq::BaseTask *testTask2 = new TestTask(task_manager, worker_thread1);

  testTask1->StartTask();
  worker_thread1->Start();
  worker_thread2->Start();
  //Failure
  tmq::TaskMsgData *msg_data = new TestMessageData("Hello world");
  for (int i = 0; i < MAX_DATA; i++){
    DLOG(INFO) << "Send Task Message ... ...";
    testTask1->SendTaskMsg(MSG_TEST_DATA, msg_data);
  }

  testTask1->StopTask();

  for (int i = 0; i < MAX_DATA; i++){
    DLOG(INFO) << "Send Task Message ... ..." ;
    tmq::TaskMsgData *msg_data = new TestMessageData("Hello world");
    testTask1->SendTaskMsg(MSG_TEST_DATA, msg_data);
  }
  singal_thread->Run();
}

int main(int argc, char *argv[]){
  google::InitGoogleLogging(argv[0]);
  FLAGS_stderrthreshold = 0;
  FLAGS_colorlogtostderr = true;
  TestWithSignalTask();

  return 0;
}