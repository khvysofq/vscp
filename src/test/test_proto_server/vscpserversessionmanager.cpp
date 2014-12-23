
#include "test/test_proto_server/vscpserversessionmanager.h"

vscp::BaseSession::SessionPtr VscpServerSessionManager::CreateSession(
  boost::asio::io_service &io_service){
  //std::cout << "Create a new server session ... ..." << std::endl;
  return VscpServerSession::Create(io_service);
}