#ifndef VZ_ZMQ_SOCKET_SERVER_H_
#define VZ_ZMQ_SOCKET_SERVER_H_

#include <vector>
#include "boost/signals2.hpp"

namespace vzipc {

  typedef unsigned int uint32;
  typedef unsigned short uint16;
  ////////////////////////////////////////////////////////////////////////////////
  //SOCKET ERROR CODE DEFINE
  static const int KForever = -1;


  // Basic define
  static const int VZ_SOCKET_SUCCEED = 0;
  static const int VZ_SOCKET_ERROR = -1;
  static const int MAX_MESSAGE_TYPE_SIZE = 256;
  static const int MAX_MESSAGE_SIZE  = 1024 * 1024;
  // The socket has no this operator
  const int VZ_INVALUE_OPERATOR   =   0X0FFF0001;
  // The socket bind getting error. if used the zmq socket ,set the logging message level 
  // To LS_ERROR you will getting the zmq error code at the log message.
  const int VZ_SOCKET_BIND_ERROR  =   0X0FFF0002;
  // The socket was closed, you cann't call the close agan.
  const int VZ_SOCKET_WAS_CLOSED  =   0X0FFF0003;
  // The socket is not connect, then it's not alow to send data
  const int VZ_SOCKET_NOT_SEND    =   0X0FFF0004;
  // The socket read data function getting error
  const int VZ_SOCKET_RECV        =   0X0FFF0005;
  // The socket more size error
  const int VZ_SOCKET_MSG_STRUCT  =   0X0FFF0006;
  // Init the message error
  const int VZ_SOCKET_MSG_ERROR   =   0X0FFF0007;
  // Send the data error
  const int VZ_SOCKET_SEND        =   0X0FFF0008;
  // Connect the server error
  const int VZ_SOCKET_CONNECT     =   0X0FFF0009;
  // The message type size is great than MAX_MESSAGE_TYPE_SIZE=256 or MAX_MESSAGE 4096
  const int VZ_SOCKET_MESSAGE_SIZE=   0X0FFF0010;
  ////////////////////////////////////////////////////////////////////////////////

  enum ConnState {
    CS_CLOSED,
    CS_CONNECTING,
    CS_CONNECTED
  };

  class VzIpcSocket : public boost::noncopyable {
  public:
    VzIpcSocket() :error_(0), socket_state_(CS_CLOSED){}

    boost::signals2::signal < void(VzIpcSocket *socket,
      const char* data, int len) > SignalPacketRecived;

    boost::signals2::signal < void(VzIpcSocket *socket,
      ConnState state) > SignalStateChange;

    boost::signals2::signal < void(VzIpcSocket *socket,
      int err) > SignalSocketError;

    virtual int Bind(boost::uint16_t port) { return VZ_SOCKET_ERROR; }
    virtual int Connect(boost::uint16_t port) { return VZ_SOCKET_ERROR; }
    virtual int AsyncWrite(const char* data, int len) = 0;
    virtual int CloseSocket() = 0;
    ConnState GetSocketState() const{ return socket_state_; }
    virtual int GetError() const { return error_; };

  protected:
    void SetSocketState(ConnState socket_state){ socket_state_ = socket_state; }
    virtual void SetError(int error) { error_ = error; };

  private:
    int error_;
    ConnState socket_state_;
  }; // class VzIpcSocket

  class VZSocketServer{
  public:
    virtual ~VZSocketServer(){};

    // By VZSocket Factory
    virtual VzIpcSocket    *CreateVZServerSocket() = 0;
    virtual VzIpcSocket    *CreateVZClientSocket() = 0;

    // This interface call when the socket process run at singal thread
    // otherwise, don't call this message
    virtual void Wait(int cmsWait) = 0; 
    virtual void WakeUp() = 0;
    
  };

  // with_new_thread = true, It create a thread that run this socket select process
  // with_new_thread = false, It not create a thread run this socket select process
  VZSocketServer *InitVZNetwork(bool with_new_thread = true);
}; // namespace zipc


#endif