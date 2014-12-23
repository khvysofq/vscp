
#include <iostream>
#include "vscp/net/vscpservices.h"
#include "pktclientsession.h"

class TestManager {
public:
  TestManager(vscp::VscpServices* vscp_services,
    const std::string server_addr,
    int port)
    :vscp_services_(vscp_services),
    server_addr_(server_addr),
    port_(port){

    sncm.SignalTaskComplete.connect(
      boost::bind(&TestManager::OnNromalCloseManagerComplete, this, _1));

    dsm.SignalTaskComplete.connect(
      boost::bind(&TestManager::OnDlownloadSessionComplete, this, _1));

    drsm.SignalTaskComplete.connect(
      boost::bind(&TestManager::OnDlownloadRandomSessionComplete, this, _1));
  }

  void StartEchoSessionTest(int session_count){
    for (int i = 0; i < session_count; i++){
      vscp_services_->CreateClientSession(&dsm, server_addr_, port_);
    }
  }

  void StartNormalCloseTest(int session_count){
    for (int i = 0; i < session_count; i++){
      vscp_services_->CreateClientSession(&sncm, server_addr_, port_);
    }
  }


  void StartDownloadRandomTest(int session_count){
    for (int i = 0; i < session_count; i++){
      vscp_services_->CreateClientSession(&drsm, server_addr_, port_);
    }
  }

  void OnNromalCloseManagerComplete(SessionNormalCloseManager* snc){
    LOG(INFO) << "normal close manager complete";
    StartEchoSessionTest(8);
  }
  // ECHO
  void OnDlownloadSessionComplete(DownloadSessionManager* dsm){
    LOG(INFO) << "normal close manager complete";
    StartDownloadRandomTest(8);
  }

  void OnDlownloadRandomSessionComplete(DownloadRandomSessionManager* drsm){
    LOG(INFO) << "normal close manager complete";
    StartNormalCloseTest(16);
  }

private:
  vscp::VscpServices* vscp_services_;
  SessionNormalCloseManager sncm;
  DownloadSessionManager dsm;
  DownloadRandomSessionManager drsm;
  const std::string server_addr_;
  int port_;
};

int main(int argv, char* argc[]){
  google::SetLogDestination(google::GLOG_ERROR, "client_log");
  google::InitGoogleLogging(argc[0]);

  vscp::VscpServices vscp_services;
  if (!vscp_services.InitVscpServices()){
    LOG(ERROR) << "InitVscpServices getting an error";
    return -1;
  }

  TestManager tm(&vscp_services, argc[1], 5298);
  tm.StartEchoSessionTest(1);
  tm.StartNormalCloseTest(10);
  tm.StartDownloadRandomTest(1);

  vscp_services.RunVscpServices();
  vscp_services.StopVscpServices();

  return 0;
}