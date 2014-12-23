#include <iostream>
#include "zmq.h"
#include "glog/logging.h"

void TestRecvThenSend(void *socket){
  while (1){
    char buffer[512];
    zmq_recv(socket, buffer, 512, 0);
    std::cout << "recv data" << std::endl;
#ifdef WIN32
    Sleep(500);
#else
    sleep(500);
#endif
    zmq_send(socket, "World", 5, 0);
    std::cout << "send data" << std::endl;
  }
}

void TestSendThenRecv(void *socket){
  while (1){
    char buffer[512];
    zmq_send(socket, "World", 5, 0);
    std::cout << "send data" << std::endl;
#ifdef WIN32
    Sleep(500);
#else
    sleep(500);
#endif
    zmq_recv(socket, buffer, 512, 0);
    std::cout << "recv data" << std::endl;
  }
}


void TestRecvTwiceThenSend(void *socket){
  while (1){
    char buffer[512];
    zmq_recv(socket, buffer, 512, 0);
    std::cout << "recv data" << std::endl;
    zmq_recv(socket, buffer, 512, 0);
    std::cout << "recv data" << std::endl;
#ifdef WIN32
    Sleep(500);
#else
    sleep(500);
#endif
    zmq_send(socket, "World", 5, 0);
    std::cout << "send data" << std::endl;
  }
}


void TestSendTwiceThenRecv(void *socket){
  while (1){
    char buffer[512];
    zmq_send(socket, "World", 5, 0);
    std::cout << "send data" << std::endl;
    zmq_send(socket, "World", 5, 0);
    std::cout << "send data" << std::endl;
#ifdef WIN32
    Sleep(500);
#else
    sleep(500);
#endif
    zmq_recv(socket, buffer, 512, 0);
    std::cout << "recv data" << std::endl;
  }
}


int main(int argc, char *argv[]){
  google::InitGoogleLogging(argv[0]);
  FLAGS_stderrthreshold = 0;
  FLAGS_colorlogtostderr = true;

  void *context = zmq_ctx_new();
  void *ipc_socket_ = zmq_socket(context, ZMQ_PAIR);
  int rc = zmq_bind(ipc_socket_, "tcp://*:5298");

  if (rc != 0){
    LOG(ERROR) << "This IPC mechanism only implemented on UNIX " 
      << rc << " " << zmq_errno();
    std::cout << "This IPC mechanism only implemented on UNIX " 
      << rc << " " << zmq_errno() << std::endl;
  }
  else{
    LOG(INFO) << "This System supported IPC " << rc;
    std::cout << "This System supported IPC " << rc << std::endl;
  }
  TestRecvTwiceThenSend(ipc_socket_);
  zmq_close(ipc_socket_);
  zmq_ctx_destroy(context);
  return 0;
}