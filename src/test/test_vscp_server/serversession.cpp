// Vision Zenith System Communication Protocol (Project)
#include "test/test_vscp_server/serversession.h"



ServerSession::ServerSession(boost::asio::io_service &io_service)
  :vscp::BaseSession(io_service, vscp::SESSION_TYPE_SERVER, 30),
  send_buffer_(new char[8192]){
}

void ServerSession::StartSession(){
  // Do nothing
  StartReadPacket();
}

void ServerSession::OnSocketError(vscp::BaseSession::SessionWptr session,
  const boost::system::error_code& err){
  LOG(ERROR) << __FUNCTION__ << "Socket Error";
}

void ServerSession::OnPacketRead(vscp::BaseSession::SessionWptr session,
  const char* buffer, int buffer_size, const boost::system::error_code& err){
  LOG(INFO) << __FUNCTION__ << "data size " << buffer_size;
  AsyncWritePacket(send_buffer_.get(), 4096);
}

void ServerSession::OnPacketWriteComplete(vscp::BaseSession::SessionWptr session,
  const boost::system::error_code& err){
  //LOG(INFO) << __FUNCTION__ << "packet write complete";
  AsyncWritePacket(send_buffer_.get(), 4096);
}
