#include <iostream>
#include "run/vscp_client/vscpclientsession.h"
#include "boost/asio.hpp"
#include "glog/logging.h"

class ClientSessionManager {
public:
  void CreateSession(boost::asio::io_service& io_s, int client_session_number,
    std::string ip_str, int port){
    for (int i = 0; i < client_session_number; i++){
      std::cout << "Create client connect " << ip_str << ":" << port << std::endl;
      boost::shared_ptr<vscp::BaseSession> client_session(
        new VscpClientSession(io_s, ip_str, port));
      sessions_.push_back(client_session);
    }
  }
  void StartSession(){
    size_t size_session = sessions_.size();
    for (size_t i = 0; i < size_session; i++){
      (sessions_[i])->InitSession();
    }
  }
private:
  std::vector<boost::shared_ptr<vscp::BaseSession> > sessions_;
};

int main(int argc, char *argv[]){
  google::InitGoogleLogging(argv[0]);
  FLAGS_stderrthreshold = 0;
  FLAGS_colorlogtostderr = true;
  //google::SetLogDestination(google::GLOG_ERROR, "./client.log");


  int client_count = 1;
  std::string server_ip_str = "192.168.1.225";
  int server_port = 5298;

  if (argc == 3){
    client_count = std::atoi(argv[1]);
    server_ip_str = argv[2];
  }
  else if (argc == 4){
    client_count = std::atoi(argv[1]);
    server_ip_str = argv[2];
    server_port = std::atoi(argv[3]);
  }

  boost::asio::io_service io_service;
  ClientSessionManager csm;
  csm.CreateSession(io_service, client_count, server_ip_str, server_port);
  csm.StartSession();

  io_service.run();

  return 0;
}