#ifndef VSCP_NET_PACKET_TRANSLATE_H_
#define VSCP_NET_PACKET_TRANSLATE_H_

#include <vector>

#include "vscp/base/basicdefines.h"
#include "vscp/net/basesession.h"
#include <queue>

namespace vscp {

  //////////////////////////////////////////////////////////////////////////////
  typedef boost::function < void(int bytes_transferred,
    const boost::system::error_code& err) > WriteCompleteHandler;

  class SocketTask : public boost::noncopyable,
  public boost::enable_shared_from_this<SocketTask>{
  public:
    SocketTask(boost::asio::ip::tcp::socket& socket,
      const std::vector<boost::asio::const_buffer>& buffers,
      WriteCompleteHandler write_complete_handler);

    SocketTask(boost::asio::ip::tcp::socket& socket,
      const char* buffer, int buffer_size,
      WriteCompleteHandler write_complete_handler);
    ~SocketTask(){
      //LOG(WARNING) << "Delele send task ... ...";
    }
    boost::signals2::signal < void(SocketTask* socket_task,
      const boost::system::error_code& err) > SignalTaskComplete;
    void StartTask();
  private:

    void HandleWriteComplete(int send_size, const boost::system::error_code& err);

    boost::asio::ip::tcp::socket& socket_;
    WriteCompleteHandler write_complete_handler_;

    std::vector<boost::asio::const_buffer> buffers_;
    const char* buffer_;
    int buffer_size_;
  };

  typedef boost::shared_ptr<SocketTask> SocketTaskPtr;

  //////////////////////////////////////////////////////////////////////////////
  class SocketWriteTaskManager{
  public:
    SocketWriteTaskManager() :is_task_run_(false){
    }
    void AsyncWriteTask(boost::asio::ip::tcp::socket& socket, 
      const std::vector<boost::asio::const_buffer>& buffers,
      WriteCompleteHandler write_complete_handler);

    void AsyncWriteTask(boost::asio::ip::tcp::socket& socket,
      const char* buffer, int buffer_size,
      WriteCompleteHandler write_complete_handler);

  private:
    void OnTaskComplete(SocketTask* socket_task,
      const boost::system::error_code& err);

    void RemoveAllTask(){
      int size = socket_tasks_.size();
      for (int i = 0; i < size; i++){
        socket_tasks_.pop();
      }
    };

    std::queue<SocketTaskPtr> socket_tasks_;
    bool is_task_run_;
  };

  //////////////////////////////////////////////////////////////////////////////
  class PacketTransmitter : public boost::noncopyable {
  public:
    PacketTransmitter(boost::asio::io_service &io_service,
      boost::asio::ip::tcp::socket& socket,
      int ping_timeout);

    boost::signals2::signal < void(BaseSession::SessionWptr session,
      const boost::system::error_code& err) > SignalSocketError;

    boost::signals2::signal < void(BaseSession::SessionWptr session, const char* buffer,
      int buffer_size, const boost::system::error_code& err) > SignalPacketRead;

    boost::signals2::signal < void(BaseSession::SessionWptr session,
      const boost::system::error_code& err) > SignalPacketWriteComplete;
    // Run with only io services thread
    bool AsyncWritePacket(BaseSession::SessionPtr session, const char* buffer,
      int buffer_size, WriteHandler* write_handler = NULL);

    // Shouldn't use this method
    bool SynchWritePacket(BaseSession::SessionPtr session, const char* buffer,
      int buffer_size);

    // Run with only io services thread
    void StartReadPacket(BaseSession::SessionPtr session, 
      SessionType session_type);
    void SetClose(){
      state_ = STATE_ERROR;
      LOG(WARNING) << state_;
    };
    void SetPingPeriodTime(int period_time){
      ping_check_seconds_ = period_time;
    }
    ~PacketTransmitter();
  private:

    void SendPingMessage(BaseSession::SessionPtr session);
    void HandlePingComplete(int len,const boost::system::error_code& err, 
      BaseSession::SessionPtr session);

    virtual void StartClientPingRequest(BaseSession::SessionPtr session);
    virtual void StartServerPingCheck(BaseSession::SessionPtr session);

    virtual void HandlePingTimeout(BaseSession::SessionPtr session,
      const boost::system::error_code& err);
    virtual void HandlePongTimeout(BaseSession::SessionPtr session,
      const boost::system::error_code& err);
    void UpdatePingTimer();
    bool IsTimeOut();

    enum WritePacketState{
      PACKET_UNSEND,
      PACKET_ERROR,
      PACKET_SUCCEED
    }; // WritePacketState

    bool AsyncReadPacket(BaseSession::SessionPtr session,
      boost::asio::ip::tcp::socket& socket);

    void SocketError(BaseSession::SessionPtr session,
      const boost::system::error_code& err);

    void HandleWriteComplete(BaseSession::SessionPtr session, int len,
      const boost::system::error_code& err, WriteHandler* write_handler);

    void HandleReadHeaderComplete(BaseSession::SessionPtr session,
      int len, const boost::system::error_code& err);

    void HandleReadDataComplete(BaseSession::SessionPtr session,
      int len, const boost::system::error_code& err);

    void HandleReadDataTimeout(BaseSession::SessionPtr session,
      const boost::system::error_code& err);

    void HandleSynchWritePacket(BaseSession::SessionPtr session,
      int len, const boost::system::error_code& err, 
      WritePacketState& write_state);

  private:

    enum {
      STATE_START,
      STATE_ERROR
    }state_;

    static const int PACKET_SIZE_LENGTH = 8;  // sizeof int32
    static const int HEADER_BEAT_SIZE = 8;
    static const int APPLICATION_DATA = 0;
    static const int PING_DATA = 1;
    static const int PONG_DATA = 2;
    static const int DEFAULT_READ_DATA_TIMEOUT = 10;
    boost::asio::deadline_timer read_time_out_;
    boost::asio::io_service &io_service_;
    boost::asio::ip::tcp::socket& socket_;

    char outbound_header_[PACKET_SIZE_LENGTH];
    std::string outbound_data_;

    char inbound_header_[PACKET_SIZE_LENGTH];
    char header_beat_[HEADER_BEAT_SIZE];
    std::vector<char> inbound_data_;

    boost::scoped_ptr<boost::asio::deadline_timer> ping_timer_;
    int ping_check_seconds_;
    SessionType session_type_;
    time_t ping_start_timer_;

    boost::scoped_ptr<SocketWriteTaskManager> socket_write_task_manager_;
  };
}; // namespace vscp

#endif // VSCP_NET_PACKET_TRANSLATE_H_