
#include "pktclientsession.h"


ClientSession::ClientSession(boost::asio::io_service &io_service, int ping_timeout)
  :BaseSession(io_service, ping_timeout),
  buffer_(new char[MAX_PACKET_SIZE]){

}

void ClientSession::OnPrepareStart(const boost::system::error_code& err){
  if (err){
    LOG(ERROR) << "Connect Server getting an error";
    return;
  }
  AsyncWritePacket(buffer_.get(), 4096);
}
// visionzenith
void ClientSession::OnSocketError(vscp::BaseSession::SessionWptr session,
  const boost::system::error_code& err){

}

void ClientSession::OnPacketRead(vscp::BaseSession::SessionWptr session,
  const char* buffer, int buffer_size, const boost::system::error_code& err){
  // Send any data back
  AsyncWritePacket(buffer, buffer_size);
}

void ClientSession::OnPacketWriteComplete(vscp::BaseSession::SessionWptr session,
  const boost::system::error_code& err){

}