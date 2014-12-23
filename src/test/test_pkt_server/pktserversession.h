#ifndef TEST_PACKET_SERVER_SESSION_H_
#define TEST_PACKET_SERVER_SESSION_H_

#include "vscp/net/basesessionmanager.h"

// This is a echo server session
class ServerSession : public vscp::BaseSession{
public:
  ServerSession(boost::asio::io_service &io_service,
    int ping_timeout);
  virtual void OnPrepareStart(const boost::system::error_code& err);
  virtual void OnSocketError(BaseSession::SessionWptr session,
    const boost::system::error_code& err);
  virtual void OnPacketRead(BaseSession::SessionWptr session, const char* buffer,
    int buffer_size, const boost::system::error_code& err);
  virtual void OnPacketWriteComplete(BaseSession::SessionWptr session,
    const boost::system::error_code& err);
private:
};

class ServerSessionManager : public vscp::BaseSessionManager{
public:
  virtual vscp::BaseSession::SessionPtr CreateServerSession(
    boost::asio::io_service &io_service){
    // ping timeout with 10 s
    return vscp::BaseSession::SessionPtr(new ServerSession(io_service, 10));
  }
  // return None, only service with server session
  virtual vscp::BaseSession::SessionPtr CreateClientSession(
    boost::asio::io_service &io_service){
    return vscp::BaseSession::SessionPtr();
  };
};

#endif // TEST_PACKET_SERVER_SESSION_H_
