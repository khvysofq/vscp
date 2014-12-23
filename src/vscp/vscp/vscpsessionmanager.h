// Vision Zenith System Communication Protocol (Project)

#ifndef VSCP_VSCPSESSIONMANAGER_H_
#define VSCP_VSCPSESSIONMANAGER_H_

#include "vscp/net/basesessionmanager.h"
#include "vscp/vscp/vscpbasesession.h"

namespace vscp {

  class VscpSessionManager : public BaseSessionManager {
  public:

  protected:

  private:

    virtual BaseSession::SessionPtr CreateSession(
      boost::asio::io_service &io_service);
    
    friend class VscpServer;

    virtual void OnClientWantLogin(VscpBaseSession* vscp_session,
      const Vsid& client_vsid, const std::string& pass) = 0;

    virtual void OnClientLogined(VscpBaseSession* vscp_session,
      const Vsid& client_vsid) = 0;

    virtual void OnClientLogout(VscpBaseSession* vscp_session,
      const Vsid& client_vsid) = 0;

    virtual void OnSessionVscpData(VscpBaseSession* vscp_session,
      const char* buffer, int len) = 0;
  };

}; // namespace vscp
#endif