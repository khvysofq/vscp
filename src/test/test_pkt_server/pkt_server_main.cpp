
#include <iostream>
#include "vscp/net/vscpservices.h"
#include "pktserversession.h"

int main(int argv, char* argc[]){
  //FLAGS_logtostderr = true;
  google::SetLogDestination(google::GLOG_ERROR, "server_log");
  google::InitGoogleLogging(argc[0]);

  ServerSessionManager ssm;
  vscp::VscpServices vscp_services;
  if (!vscp_services.InitVscpServices()){
    LOG(ERROR) << "InitVscpServices getting an error";
    return -1;
  }

  vscp::VscpServerPtr vscp_server = vscp_services.CreateVscpServer(
    &ssm,"0.0.0.0", 5298);
  if (!vscp_server){
    LOG(ERROR) << "Create VSCP Server getting an error";
    return -1;
  }
  vscp_server->StartServer();
  vscp_services.RunVscpServices();
  vscp_services.StopVscpServices();

  return 0;
}