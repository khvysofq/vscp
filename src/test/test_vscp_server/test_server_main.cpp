#include <iostream>
#include "vscp/net/vscpserver.h"
#include "test/test_vscp_server/serversessionmanager.h"
#include "glog/logging.h"

void TestHandler(const std::string msg, int count){

  for (int i = 0; i < count; i++){
    std::cout << msg << std::endl;
  }
}

int main(int argc, char *argv[]){
  google::InitGoogleLogging(argv[0]);
  FLAGS_stderrthreshold = 0;
  FLAGS_colorlogtostderr = true;

  vscp::BaseSessionManager *session_manager = 
    new ServerSessionManager();
  vscp::VscpServer vscp_server(session_manager);
  vscp_server.InitVscpServer();
  vscp_server.PostHandler(boost::bind(&TestHandler, "Hello Word", 10));
  vscp_server.Run();
  vscp_server.Stop();

  return 0;
}