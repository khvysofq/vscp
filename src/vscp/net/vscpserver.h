// Vision Zenith System Communication Protocol (Project)
#ifndef VSCP_NET_VSCP_SERVER_H_
#define VSCP_NET_VSCP_SERVER_H_

#include <string>
#include <deque>


#include "vscp/base/basicdefines.h"
#include "vscp/net/basesessionmanager.h"

namespace vscp {

  class VscpServer : public boost::noncopyable ,
    public boost::enable_shared_from_this<VscpServer>{
  public:
    VscpServer(boost::asio::io_service& io_service,
      BaseSessionManager *session_manager,
      const std::string server_address ,
      uint16 server_port);

    bool StartServer();
    bool StopServer();
  private:

    bool SetStartServer(){
      boost::mutex::scoped_lock close_mutex(is_stop_mutex_);
      if (is_stop_server_){
        return false;
      }
      is_stop_server_ = true;
      return false;
    }

    bool SetStopServer(){
      boost::mutex::scoped_lock close_mutex(is_stop_mutex_);
      if (!is_stop_server_){
        return false;
      }
      is_stop_server_ = true;
      return false;
    }

    void StartAccept();
    void HandleAccept(BaseSession::SessionPtr new_session,
      const boost::system::error_code& err);
    void HandleStopAccept();

  private:
    typedef std::deque<io_service_ptr> ios_deque;

    boost::asio::io_service& io_service_;

    std::string server_address_;
    uint16 server_port_;
    const boost::asio::ip::tcp::endpoint endpoint_;
    boost::scoped_ptr<boost::asio::ip::tcp::acceptor> acceptor_;
    boost::scoped_ptr<BaseSessionManager> session_manager_;

    boost::mutex is_stop_mutex_;
    bool is_stop_server_;
  };

  typedef boost::shared_ptr<VscpServer> VscpServerPtr;

}; // namespace vscp

#endif // VSCP_NET_VSCP_SERVER_H_