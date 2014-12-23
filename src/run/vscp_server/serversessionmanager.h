#ifndef VSCP_SERVER_SERVERSESSIONMANAGER_H_
#define VSCP_SERVER_SERVERSESSIONMANAGER_H_

#include "vscp/vscp/vscpsessionmanager.h"
#include "vzipc/base/vzipcserver.h"
#include <map>

class ServerSessionManager : public vscp::VscpSessionManager{
public:
  ServerSessionManager();
  bool InitSessionManager(vzipc::VZSocketServer *socket_server, int proto_port);
private:
  // With basesessionmanager
  virtual void OnClientWantLogin(vscp::VscpBaseSession* vscp_session,
    const vscp::Vsid& client_vsid, const std::string& pass);

  virtual void OnClientLogined(vscp::VscpBaseSession* vscp_session,
    const vscp::Vsid& client_vsid);

  virtual void OnClientLogout(vscp::VscpBaseSession* vscp_session,
    const vscp::Vsid& client_vsid);

  virtual void OnSessionVscpData(vscp::VscpBaseSession* vscp_session,
    const char* buffer, int len);

  // With IpcSocket
  void OnPacketRecived(vzipc::VzIpcSocket *socket, const char* data, int len);

  void OnStateChange(vzipc::VzIpcSocket *socket, vzipc::ConnState state);

  void OnSocketError(vzipc::VzIpcSocket *socket, int err);

private:
  typedef std::map<std::string ,vscp::VscpBaseSession*> ServerSessions;
  ServerSessions server_sessions_;
  vzipc::VzIpcSocket *ipc_socket_;
}; // class ServerSessionManager

#endif // VSCP_SERVER_SERVERSESSIONMANAGER_H_