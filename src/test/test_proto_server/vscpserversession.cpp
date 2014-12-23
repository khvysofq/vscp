// Vision Zenith System Communication Protocol (Project)
#include "test/test_proto_server/vscpserversession.h"
#include "vscp/proto/protobasicdefines.h"



VscpServerSession::VscpServerSession(boost::asio::io_service &io_service)
  :vscp::VscpBaseSession(io_service, vscp::PROTO_SERVER, 3){
}

VscpServerSession::~VscpServerSession(){
#ifdef _DEBUG
  std::cout << "Delete Vscp Server Session ... ..." << std::endl;
#endif
}