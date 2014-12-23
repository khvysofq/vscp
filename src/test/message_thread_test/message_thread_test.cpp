#include <iostream>
#include "tmq/base/thread.h"
#include "glog/logging.h"

int main(int argc, char *argv[]){
  google::InitGoogleLogging(argv[0]);

  tmq::Thread *thread = tmq::Thread::Current();

  std::cout << "start process messages 5 seconds" << std::endl;
  thread->ProcessMessages(5000);
  std::cout << "end process messages seconds" << std::endl;

  return 0;
}