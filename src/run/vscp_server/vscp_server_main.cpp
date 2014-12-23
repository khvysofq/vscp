#include <iostream>
#include "run/vscp_server/serversessionmanager.h"
#include "vscp/net/vscpserver.h"
#include "vzipc/base/vzipcserver.h"

int main(int argc, char *argv[]){
  google::InitGoogleLogging(argv[0]);
  FLAGS_stderrthreshold = 0;
  FLAGS_colorlogtostderr = true;

  vzipc::VZSocketServer *socket_server = vzipc::InitVZNetwork();
  ServerSessionManager session_manager;

  session_manager.InitSessionManager(socket_server, 5299);
  vscp::VscpServer vscp_server(&session_manager,"",5298);
  vscp_server.InitVscpServer();

  while (1){ 
    vscp_server.Run(100);      // With boost asio
    socket_server->Wait(100);  // With zmq_poll
  }
  return 0;
}