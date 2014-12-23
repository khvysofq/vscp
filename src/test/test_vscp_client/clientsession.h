// Vision Zenith System Communication Protocol (Project)
#ifndef VSCP_NET_CLIENT_SESSION_H_
#define VSCP_NET_CLIENT_SESSION_H_

#include "vscp/base/basicdefines.h"
#include "vscp/net/basesession.h"

using namespace vscp;

class ClientSession : public BaseSession {
public:
  typedef boost::shared_ptr<ClientSession> ClientSession_ptr;
  static BaseSession::SessionPtr Create(boost::asio::io_service &io_service,
    std::string server_addr, uint16 server_port){

    return BaseSession::SessionPtr(new ClientSession(io_service,
      server_addr, server_port));
  }

  void StartSession();

private:
  ClientSession(boost::asio::io_service &io_service, std::string server_addr,
    uint16 server_port);

  void HandleConnect(const boost::system::error_code& err);

  virtual void OnSocketError(BaseSession::SessionWptr session,
    const boost::system::error_code& err);

  virtual void OnPacketRead(BaseSession::SessionWptr session, const char* buffer,
    int buffer_size, const boost::system::error_code& err);

  virtual void OnPacketWriteComplete(BaseSession::SessionWptr session,
    const boost::system::error_code& err);

private:
  boost::scoped_array<char> send_buffer_;
  const boost::asio::ip::tcp::endpoint server_point_;

}; // class ClientSession

#endif // VSCP_NET_CLIENT_SESSION_H_