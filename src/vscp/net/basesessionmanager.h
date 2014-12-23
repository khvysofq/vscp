// Vision Zenith System Communication Protocol (Project)
#ifndef VSCP_NET_SESSION_MANAGER_H_
#define VSCP_NET_SESSION_MANAGER_H_

#include <vector>

#include "vscp/base/basicdefines.h"
#include "vscp/net/basesession.h"

namespace vscp {

  class BaseSessionManager : public boost::noncopyable {
  public:
    BaseSessionManager(){};

    virtual BaseSession::SessionPtr CreateServerSession(
      boost::asio::io_service &io_service) = 0;
    virtual BaseSession::SessionPtr CreateClientSession(
      boost::asio::io_service &io_service) = 0;
  }; // class SessionManager
}; // namespace vscp

// 
#endif // VSCP_NET_SESSION_MANAGER_H_