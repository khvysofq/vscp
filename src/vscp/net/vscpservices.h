#ifndef VSCP_NET_VSCP_SERVICES_H_
#define VSCP_NET_VSCP_SERVICES_H_

#include "vscp/base/basicdefines.h"
#include "vscp/net/basesessionmanager.h"
#include "vscp/net/vscpserver.h"

namespace vscp {

  class VscpServices : public boost::noncopyable {
  public:
    VscpServices();

    // Not thread safe
    bool InitVscpServices();
    // Not thread safe, vscpservices will run at this call thread
    bool RunVscpServices(int millisecond = FOREVER);

    // thread safe
    void StopVscpServices();
    // thread safe
    bool PostHandler(boost::function<void()> callback_handler);
    
    // 
    virtual VscpServerPtr CreateVscpServer(BaseSessionManager* session_manager, 
      const std::string server_address, uint16 server_port);
    virtual BaseSession::SessionPtr CreateClientSession(
      BaseSessionManager* session_manager, 
      const std::string server_address, uint16 server_port);
  private:
    void HandleTimeout(boost::shared_ptr<boost::asio::deadline_timer> timeout,
      const boost::system::error_code& err);
    void HandleClientSessionInit(BaseSession::SessionPtr client_session,
      std::string server_address, uint16 server_port);

    enum{
      STATE_NON = 1,
      STATE_INITED,
      STATE_STARTED,
      STATE_DISABLE
    }state_;

    boost::asio::io_service io_service_;
    boost::scoped_ptr<boost::asio::io_service::work> io_service_work_;
    bool is_time_out_;
  };

}; // namespace vscp

#endif // VSCP_NET_VSCP_SERVICES_H_