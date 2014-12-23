// Vision Zenith System Communication Protocol (Project)
#ifndef VSCP_NET_SERVER_SESSION_H_
#define VSCP_NET_SERVER_SESSION_H_

#include "vscp/base/basicdefines.h"
#include "vscp/net/basesession.h"


class ServerSession : public vscp::BaseSession {
public:
  static vscp::BaseSession::SessionPtr Create(
    boost::asio::io_service &io_service){
    return vscp::BaseSession::SessionPtr(
      new ServerSession(io_service));
  }

  virtual void StartSession();

private:
  ServerSession(boost::asio::io_service &io_service);

  virtual void OnSocketError(vscp::BaseSession::SessionWptr session,
    const boost::system::error_code& err);

  virtual void OnPacketRead(vscp::BaseSession::SessionWptr session, const char* buffer,
    int buffer_size, const boost::system::error_code& err);

  virtual void OnPacketWriteComplete(vscp::BaseSession::SessionWptr session,
    const boost::system::error_code& err);


private:
  boost::scoped_array<char> send_buffer_;
}; // class session

#endif // VSCP_NET_SERVER_SESSION_H_