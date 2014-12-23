// Vision Zenith System Communication Protocol (Project)
#ifndef VSCP_NET_CLIENT_SESSION_H_
#define VSCP_NET_CLIENT_SESSION_H_

#include "vscp/base/basicdefines.h"
#include "vscp/vscp/vscpbasesession.h"

using namespace vscp;

class VscpClientSession : public vscp::VscpBaseSession {
public:
  VscpClientSession(boost::asio::io_service &io_service, std::string server_addr,
    uint16 server_port);
  virtual ~VscpClientSession();

  virtual void StartSession();

private:
  void StartConnectServer();
  void HandleConnect(const boost::system::error_code& err);

private:
  const boost::asio::ip::tcp::endpoint server_point_;

}; // class ClientSession

#endif // VSCP_NET_CLIENT_SESSION_H_