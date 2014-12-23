
#include "pktserversession.h"


ServerSession::ServerSession(boost::asio::io_service &io_service, int ping_timeout)
  :BaseSession(io_service, ping_timeout){

}

void ServerSession::OnPrepareStart(const boost::system::error_code& err){

}

void ServerSession::OnSocketError(vscp::BaseSession::SessionWptr session,
  const boost::system::error_code& err){

}

void ServerSession::OnPacketRead(vscp::BaseSession::SessionWptr session, 
  const char* buffer, int buffer_size, const boost::system::error_code& err){
  // Send any data back
  AsyncWritePacket(buffer, buffer_size);
}

void ServerSession::OnPacketWriteComplete(vscp::BaseSession::SessionWptr session,
  const boost::system::error_code& err){

}