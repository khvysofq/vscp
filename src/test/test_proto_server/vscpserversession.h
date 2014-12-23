// Vision Zenith System Communication Protocol (Project)
#ifndef VSCP_TEST_VSCPSERVERSESSION_H_
#define VSCP_TEST_VSCPSERVERSESSION_H_

#include "vscp/base/basicdefines.h"
#include "vscp/vscp/vscpbasesession.h"


class VscpServerSession : public vscp::VscpBaseSession {
public:
  static vscp::BaseSession::SessionPtr Create(
    boost::asio::io_service &io_service){
    return vscp::BaseSession::SessionPtr(
      new VscpServerSession(io_service));
  }
  virtual ~VscpServerSession();
private:
  VscpServerSession(boost::asio::io_service &io_service);
}; // class session

#endif // VSCP_TEST_VSCPSERVERSESSION_H_