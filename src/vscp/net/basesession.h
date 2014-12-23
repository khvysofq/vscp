// Vision Zenith System Communication Protocol (Project)
#ifndef VSCP_NET_BASE_SESSION_H_
#define VSCP_NET_BASE_SESSION_H_

#include "vscp/base/basicdefines.h"
#include "vscp/net/writehandler.h"
#include "boost/timer.hpp"
//#include "vscp/net/packettransmitter.h"

namespace vscp {

  class PacketTransmitter;

  enum SessionType{
    SESSION_TYPE_UNDEF,
    SESSION_TYPE_SERVER,
    SESSION_TYPE_CLIENT
  };

  static const int PING_TIMEOUT = 30;
  static const int PING_CHECK_TIMES = 3;

  class BaseSession : public boost::noncopyable,
    public boost::enable_shared_from_this< BaseSession >{
  public:

    typedef boost::shared_ptr<BaseSession> SessionPtr;
    typedef boost::weak_ptr<BaseSession> SessionWptr;

    explicit BaseSession(boost::asio::io_service &io_service,
      int ping_timeout = PING_TIMEOUT);

    virtual ~BaseSession();
    // Thread safe
    void CloseSession();
    // Thread safe
    bool AsyncWritePacket(const char* buffer, int buffer_size, 
      WriteHandler *write_handler = NULL);

    // Shouldn't use this method
    bool SynchWritePacket(const char* buffer, int buffer_size);

  protected:

    virtual void RealStartSession(SessionType session_type);
    boost::asio::io_service& GetIoService(){ return io_service_; }

  private:
    friend class VscpServices;
    friend class VscpServer;

    void InitServerSession();
    void InitPacketTransmitter();
    void StartConnectServer(std::string server_address, uint16 server_port);

    void HandleConnect(const boost::system::error_code& err);
    void HandleClose();
    void HandleAsyncWritePacket(const char* buffer, int buffer_size,
      WriteHandler *write_handler);
    boost::asio::ip::tcp::socket& Socket() {
      return socket_;
    };

    virtual void OnPrepareStart(const boost::system::error_code& err) = 0;

    // This call back function indicate that this connect was closed.
    // Used the err.message() to show more information about this error
    // On Linx and like systems see: 
    // http://www-numi.fnal.gov/offline_software/srt_public_context/WebDocs/Errors/unix_system_errors.html
    // On Windows see: 
    // http://msdn.microsoft.com/en-us/library/windows/desktop/ms740668(v=vs.85).aspx
    virtual void OnSocketError(BaseSession::SessionWptr session,
      const boost::system::error_code& err) = 0;

    virtual void OnPacketRead(BaseSession::SessionWptr session, const char* buffer,
      int buffer_size, const boost::system::error_code& err) = 0;

    virtual void OnPacketWriteComplete(BaseSession::SessionWptr session,
      const boost::system::error_code& err) = 0;

  private:

    enum{
      STATE_DISABLE,
      STATE_ENABLE
    }state_;

    boost::asio::ip::tcp::socket socket_;
    boost::asio::io_service &io_service_;
    PacketTransmitter *packet_transmitter_;

    int ping_check_seconds_;
    boost::mutex close_socket_mutex_;
  }; // class BaseSession 

}; // namespace vscp

#endif // VSCP_NET_BASE_SESSION_H_