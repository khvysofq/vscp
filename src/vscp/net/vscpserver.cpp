// Vision Zenith System Communication Protocol (Project)
#include "vscp/net/vscpserver.h"

namespace vscp {

  VscpServer::VscpServer(boost::asio::io_service& io_service,
    BaseSessionManager *session_manager,
    const std::string server_address,
    uint16 server_port)
    :io_service_(io_service),
    session_manager_(session_manager),
    server_address_(server_address),
    server_port_(server_port),
    endpoint_(server_address.empty() ?
    (boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), server_port_)) :
    boost::asio::ip::tcp::endpoint(boost::asio::ip::address().from_string(
    server_address_), server_port_)),
    is_stop_server_(false){

  }

  bool VscpServer::StartServer(){
    if (SetStartServer()){
      LOG(ERROR) << "The server is run, you can't start again";
      return false;
    }
    try{
      acceptor_.reset(new boost::asio::ip::tcp::acceptor(
        io_service_, endpoint_));
    }
    catch (std::exception& e){
      LOG(ERROR) << "bind address getting an error " << e.what();
      return false;
    }
    io_service_.post(boost::bind(
      &VscpServer::StartAccept, shared_from_this()));
    return true;
  }

  bool VscpServer::StopServer(){
    if (SetStopServer()){
      LOG(ERROR) << "The server is not run, you can't stop it";
      return false;
    }
    io_service_.post(
      boost::bind(&VscpServer::HandleStopAccept, shared_from_this()));
    return true;
  }

  void VscpServer::StartAccept(){
    // Add session 
    BaseSession::SessionPtr new_session =
      session_manager_->CreateServerSession(io_service_);

    acceptor_->async_accept(new_session->Socket(),
      boost::bind(&VscpServer::HandleAccept, shared_from_this(), new_session,
      boost::asio::placeholders::error));
  }

  void VscpServer::HandleAccept(BaseSession::SessionPtr new_session,
    const boost::system::error_code& err){

    if (err){
      LOG(WARNING) << "Accpet new connect get an error :" << err;
      return;
    }
    static int count = 1;
    std::cout << count++ << "\t";
    // Make sure, the ServerSession::InitSession is non-blocking
    new_session->InitServerSession();
    StartAccept();
  }

  void VscpServer::HandleStopAccept(){
    if (is_stop_server_){
      LOG(ERROR) << "The server flags is not stop state";
      return;
    }
    try{
      // Cancel or close, should test
      // see http://www.boost.org/doc/libs/1_52_0/doc/html/boost_asio/reference/basic_socket/cancel/overload1.html
      if (acceptor_->is_open()){
        acceptor_->close();
      }
    }
    catch (std::exception& e){
      LOG(ERROR) << "Close acceptor getting an exception " << e.what();
    }
  }

}; // namespace vscp