#include <iostream>
#include "test/test_vscp_client/clientsession.h"
#include "boost/asio.hpp"
#include "glog/logging.h"
#include "boost/timer.hpp"

int main(int argc, char *argv[]){
  google::InitGoogleLogging(argv[0]);
  FLAGS_stderrthreshold = 0;
  FLAGS_colorlogtostderr = true;

  boost::asio::io_service io_service;

  vscp::BaseSession::SessionPtr client_session = ClientSession::Create(
    io_service, "192.168.1.28", 5298);
  client_session->InitSession();
  //boost::asio::io_service::work work(io_service);
  io_service.run();

  return 0;
}
