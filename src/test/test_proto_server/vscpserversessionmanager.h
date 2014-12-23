#ifndef VSCP_TEST_VSCPSERVERSESSIONMANAGER_H_
#define VSCP_TEST_VSCPSERVERSESSIONMANAGER_H_
#include "vscp/net/basesessionmanager.h"
#include "test/test_proto_server/vscpserversession.h"

class VscpServerSessionManager :public vscp::BaseSessionManager{
public:
  virtual vscp::BaseSession::SessionPtr CreateSession(
    boost::asio::io_service &io_service) ;


};

#endif // VSCP_TEST_VSCPSERVERSESSIONMANAGER_H_