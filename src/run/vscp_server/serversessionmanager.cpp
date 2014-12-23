
#include "run/vscp_server/serversessionmanager.h"
#include "glog/logging.h"


ServerSessionManager::ServerSessionManager()
  :ipc_socket_(NULL){

}

bool ServerSessionManager::InitSessionManager(
  vzipc::VZSocketServer *socket_server, int proto_port){

  ipc_socket_ = socket_server->CreateVZClientSocket();

  ipc_socket_->SignalPacketRecived.connect(
    boost::bind(&ServerSessionManager::OnPacketRecived, this, _1, _2, _3));
  ipc_socket_->SignalStateChange.connect(
    boost::bind(&ServerSessionManager::OnStateChange, this, _1, _2));
  ipc_socket_->SignalSocketError.connect(
    boost::bind(&ServerSessionManager::OnSocketError, this, _1, _2));

  if (vzipc::VZ_SOCKET_ERROR == ipc_socket_->Connect(proto_port)){
    return false;
  }
  ipc_socket_->AsyncWrite("Hello", 5);
  return true;
}

void ServerSessionManager::OnClientWantLogin(vscp::VscpBaseSession* vscp_session,
  const vscp::Vsid& client_vsid, const std::string& pass){
  LOG(INFO) << __FUNCTION__;
  //TODO, Reject or Accept this session, there is no interface
}

void ServerSessionManager::OnClientLogined(vscp::VscpBaseSession* vscp_session,
  const vscp::Vsid& client_vsid){
  LOG(INFO) << __FUNCTION__;
  server_sessions_[client_vsid.Str()] = vscp_session;
}

void ServerSessionManager::OnClientLogout(vscp::VscpBaseSession* vscp_session,
  const vscp::Vsid& client_vsid){
  LOG(INFO) << __FUNCTION__;
  ServerSessions::iterator iter = server_sessions_.find(client_vsid.Str());
  if (iter != server_sessions_.begin()){
    server_sessions_.erase(iter);
  }
}

void ServerSessionManager::OnSessionVscpData(vscp::VscpBaseSession* vscp_session,
  const char* buffer, int len){
  LOG(INFO) << __FUNCTION__;
  ipc_socket_->AsyncWrite(buffer, len);
}

void ServerSessionManager::OnPacketRecived(vzipc::VzIpcSocket *socket, 
  const char* data, int len){
  //LOG(INFO) << ".";
  CHECK(socket == ipc_socket_);
  // Find a best way to search session
  ipc_socket_->AsyncWrite(data, len);
  
}

void ServerSessionManager::OnStateChange(vzipc::VzIpcSocket *socket, 
  vzipc::ConnState state){
  LOG(INFO) << __FUNCTION__;

}

void ServerSessionManager::OnSocketError(vzipc::VzIpcSocket *socket, int err){
  LOG(INFO) << __FUNCTION__;
}