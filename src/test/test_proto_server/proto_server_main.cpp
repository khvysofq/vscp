#include <iostream>
#include "vscp/net/vscpserver.h"
#include "test/test_proto_server/vscpserversessionmanager.h"
#include "glog/logging.h"

int main(int argc, char *argv[]){
  google::InitGoogleLogging(argv[0]);
  FLAGS_stderrthreshold = 1;
  FLAGS_colorlogtostderr = true;

  google::SetLogDestination(google::GLOG_ERROR, "./server.log");

  uint16 port = 5298;
  if (argc == 2){
    port = std::atoi(argv[1]);
  }
  vscp::BaseSessionManager *session_manager = 
    new VscpServerSessionManager();
  vscp::VscpServer vscp_server(session_manager, "", port);
  vscp_server.InitVscpServer();
  vscp_server.Run();
  vscp_server.Stop();

  return 0;
}