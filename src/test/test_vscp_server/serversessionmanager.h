#ifndef SERVER_SESSION_MANAGER_H_
#define SERVER_SESSION_MANAGER_H_
#include "vscp/net/basesessionmanager.h"
#include "test/test_vscp_server/serversession.h"

class ServerSessionManager :public vscp::BaseSessionManager{
public:
  virtual vscp::BaseSession::SessionPtr CreateSession(
    boost::asio::io_service &io_service) ;


};

#endif