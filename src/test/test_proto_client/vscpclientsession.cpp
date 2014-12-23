// Vision Zenith System Communication Protocol (Project)

#include "test/test_proto_client/vscpclientsession.h"

VscpClientSession::VscpClientSession(boost::asio::io_service &io_service,
  std::string server_addr, uint16 server_port)
  :VscpBaseSession(io_service, PROTO_CLIENT),
  server_point_(boost::asio::ip::tcp::endpoint(
  boost::asio::ip::address().from_string(server_addr), server_port)){

}

VscpClientSession::~VscpClientSession(){
#ifdef _DEBUG
  std::cout << "Delete vscp client session ... ..." << std::endl;
#endif
}


void VscpClientSession::StartSession(){
  StartConnectServer();
}

void VscpClientSession::StartConnectServer(){

  Socket().async_connect(server_point_,
    boost::bind(&VscpClientSession::HandleConnect, this,
    boost::asio::placeholders::error));
}

void VscpClientSession::HandleConnect(const boost::system::error_code& err){

  LOG(INFO) << "Connect server succeed ";
  if (err){
    LOG(INFO) << __FUNCTION__ << "Connect server error";
    return;
  }

  VscpBaseSession::StartSession();
  //StartReadPacket();
  //InitProtoAnalyzer();
}