// Vision Zenith System Communication Protocol (Project)
#include "test/test_vscp_client/clientsession.h"

ClientSession::ClientSession(boost::asio::io_service &io_service,
  std::string server_addr, uint16 server_port)
  :BaseSession(io_service, vscp::SESSION_TYPE_CLIENT,300),
  server_point_(boost::asio::ip::tcp::endpoint(
  boost::asio::ip::address().from_string(server_addr), server_port)),
  send_buffer_(new char[8192]){

}

void ClientSession::StartSession(){

  Socket().async_connect(server_point_,
    boost::bind(&ClientSession::HandleConnect, this,
    boost::asio::placeholders::error));

}

void ClientSession::HandleConnect(const boost::system::error_code& err){

  LOG(INFO) << "Connect server succeed ";
  if (err){
    LOG(INFO) << __FUNCTION__ << "Connect server error";
    return;
  }

  StartReadPacket();

  //AsyncWritePacket(send_buffer_.get(), 4096);

}

void ClientSession::OnSocketError(BaseSession::SessionWptr session,
  const boost::system::error_code& err){
  //  LOG(ERROR) << __FUNCTION__ << "Socket Error";
}

void ClientSession::OnPacketRead(BaseSession::SessionWptr session,
  const char* buffer, int buffer_size, const boost::system::error_code& err){
  //  LOG(INFO) << __FUNCTION__ << "data size " << buffer_size;
}

void ClientSession::OnPacketWriteComplete(BaseSession::SessionWptr session,
  const boost::system::error_code& err){
  //  LOG(INFO) << __FUNCTION__ << "packet write complete";
  //packet_translate_->AsyncWritePacket(&send_buffer_[0], 4096);
}