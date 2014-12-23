
#include "test/test_vscp_server/serversessionmanager.h"

vscp::BaseSession::SessionPtr ServerSessionManager::CreateSession(
  boost::asio::io_service &io_service){
  return ServerSession::Create(io_service);
}