// Vision Zenith System Communication Protocol (Project)

#include "vscp/vscp/vscpsessionmanager.h"

namespace vscp {

  BaseSession::SessionPtr VscpSessionManager::CreateSession(
    boost::asio::io_service &io_service){

    VscpBaseSession *session = new VscpBaseSession(io_service, PROTO_SERVER);
    session->SignalClientWantLogin.connect(
      boost::bind(&VscpSessionManager::OnClientWantLogin, this, _1, _2, _3));
    session->SignalClientLogined.connect(
      boost::bind(&VscpSessionManager::OnClientLogined, this, _1, _2));
    session->SignalClientLogout.connect(
      boost::bind(&VscpSessionManager::OnClientLogout, this, _1, _2));
    session->SignalSessionVscpData.connect(
      boost::bind(&VscpSessionManager::OnSessionVscpData, this, _1, _2, _3));

    return BaseSession::SessionPtr(session);
  }

}; // namespace vscp