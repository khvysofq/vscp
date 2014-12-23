//#define GLOG_NO_ABBREVIATED_SEVERITIES
#include "glog/logging.h"
#include <zmq.h>
#include <sstream>
#include <map>

#include "boost/thread.hpp"
#include "boost/timer.hpp"
#include "boost/bind.hpp"
#include "boost/smart_ptr.hpp"

#include "vzipc/base/vzipcserver.h"

namespace vzipc {

  const static char MSG_DELETE[] = "DS";
  const static int MSG_DELETE_LEN = 3;

  const static char MSG_NEW[] = "NS";
  const static int MSG_NEW_LEN = 3;
  ////////////////////////////////////////////////////////////////////////////////

  const boost::uint32_t LAST = 0xFFFFFFFF;
  const boost::uint32_t HALF = 0x80000000;

  static const boost::int64_t kNumMillisecsPerSec = INT64_C(1000);
  static const boost::int64_t kNumMicrosecsPerSec = INT64_C(1000000);
  static const boost::int64_t kNumNanosecsPerSec = INT64_C(1000000000);

  static const boost::int64_t kNumMicrosecsPerMillisec = kNumMicrosecsPerSec /
    kNumMillisecsPerSec;
  static const boost::int64_t kNumNanosecsPerMillisec = kNumNanosecsPerSec /
    kNumMillisecsPerSec;

  // January 1970, in NTP milliseconds.
  static const boost::int64_t kJan1970AsNtpMillisecs = INT64_C(2208988800000);

  boost::uint64_t TimeNanos() {
    boost::int64_t ticks = 0;
#if defined(OSX) || defined(IOS)
    static mach_timebase_info_data_t timebase;
    if (timebase.denom == 0) {
      // Get the timebase if this is the first time we run.
      // Recommended by Apple's QA1398.
      //VERIFY(KERN_SUCCESS == mach_timebase_info(&timebase));
    }
    // Use timebase to convert absolute time tick units into nanoseconds.
    ticks = mach_absolute_time() * timebase.numer / timebase.denom;
#elif defined(POSIX)
    struct timespec ts;
    // TODO: Do we need to handle the case when CLOCK_MONOTONIC
    // is not supported?
    clock_gettime(CLOCK_MONOTONIC, &ts);
    ticks = kNumNanosecsPerSec * static_cast<int64>(ts.tv_sec) +
      static_cast<int64>(ts.tv_nsec);
#elif defined(WIN32)
    static volatile LONG last_timegettime = 0;
    static volatile boost::int64_t num_wrap_timegettime = 0;
    volatile LONG* last_timegettime_ptr = &last_timegettime;
    DWORD now = timeGetTime();
    // Atomically update the last gotten time
    DWORD old = InterlockedExchange(last_timegettime_ptr, now);
    if (now < old) {
      // If now is earlier than old, there may have been a race between
      // threads.
      // 0x0fffffff ~3.1 days, the code will not take that long to execute
      // so it must have been a wrap around.
      if (old > 0xf0000000 && now < 0x0fffffff) {
        num_wrap_timegettime++;
      }
    }
    ticks = now + (num_wrap_timegettime << 32);
    // TODO: Calculate with nanosecond precision.  Otherwise, we're just
    // wasting a multiply and divide when doing Time() on Windows.
    ticks = ticks * kNumNanosecsPerMillisec;
#endif
    return ticks;
  }

  boost::uint32_t Time() {
    return static_cast<boost::uint32_t>(TimeNanos() / kNumNanosecsPerMillisec);
  }

  ////////////////////////////////////////////////////////////////////////////////
  //basic define
  enum DispatcherState{
    PSTATE_NON,
    PSTATE_ENABLE,
    PSTATE_DISABLE
  }; //
  class VZDispatcher : public VzIpcSocket,
    public boost::enable_shared_from_this < VZDispatcher > {
  public:
    VZDispatcher() :state_(PSTATE_NON){
    }
    virtual ~VZDispatcher(){};

    void SetZmqPollItem(void *socket){
      zmq_pollitem_.reset(new zmq_pollitem_t());
      zmq_pollitem_->events = ZMQ_POLLIN|ZMQ_POLLERR;
      zmq_pollitem_->fd = 0;
      zmq_pollitem_->socket = socket;
      zmq_pollitem_->revents = 0;
    }
    zmq_pollitem_t *GetZmqPollItem() { return zmq_pollitem_.get(); }

    // For socket read event
    virtual void OnEvent(char *msg_str, short revent) {
      if (revent & ZMQ_POLLIN){
        OnReadEvent(msg_str);
      }
      if (revent & ZMQ_POLLOUT){
        OnWriteEvent();
      }
      if (revent & ZMQ_POLLERR){
        OnErrorEvent();
      }
    }

    virtual void OnReadEvent(char *buffer){
    }

    virtual void OnWriteEvent(){
      //LOG(INFO) << __FUNCTION__;
    }

    virtual void OnErrorEvent(){
      LOG(INFO) << __FUNCTION__;
    }
    DispatcherState GetDispatcherState() const{ return state_; }

  protected:
    void SetDispatcherState(DispatcherState state){ state_ = state; }
  private:
    DispatcherState state_;
    boost::scoped_ptr<zmq_pollitem_t> zmq_pollitem_;
  };

  static int ZmqSend(void *socket, const void *data, uint32 len, int flags){
    /* Create a new message, allocating 6 bytes for message content */
    DLOG_IF(WARNING, len == 0) << "the length of the data is 0";
    DLOG_IF(WARNING, data == NULL) << "the data is null";
    zmq_msg_t msg;
    int rc = zmq_msg_init_size(&msg, len);
    if (rc != 0){
      //#if LOGGING
      LOG(ERROR) << "Init the msg getting error" << zmq_errno();
      //#endif
      //SetError(VZ_SOCKET_MSG_ERROR);
      return VZ_SOCKET_ERROR;
    }
    memcpy(zmq_msg_data(&msg), data, len);
    /* Send the message to the socket */
    rc = zmq_msg_send(&msg, socket, flags);
    if (rc == 0){
      //#if LOGGING
      LOG(ERROR) << "Send the zmq data error " << zmq_errno();
      //#endif
      //SetError(VZ_SOCKET_SEND);
      zmq_msg_close(&msg);
      return VZ_SOCKET_ERROR;
    }
    zmq_msg_close(&msg);
    return VZ_SOCKET_SUCCEED;
  }

  //////////////////////////////////////////////////////////////////////////////
  //by VZServerSocket

  class VZZmqServerSocket : public VZDispatcher {
  public:
    //
    VZZmqServerSocket(void *context)
      :zmq_context_(context){

      CHECK(zmq_context_ != NULL) << "the zmq context is null";
      SetError(VZ_INVALUE_OPERATOR);
    }
    //
    virtual ~VZZmqServerSocket(){
    }

    virtual int Bind(uint16 port){

      if (GetSocketState() != CS_CLOSED){
        LOG(ERROR) << "The server sokcet state is not CS_CLOSED";
        SetError(VZ_SOCKET_BIND_ERROR);
        return VZ_SOCKET_ERROR;
      }

      //1. Create ZMQ_SOCKET Object
      zmq_socket_ = zmq_socket(zmq_context_, ZMQ_PAIR);
      if (zmq_socket_ == NULL){
        LOG(ERROR) << "Create Zmq socket error " << zmq_errno();
        return VZ_SOCKET_ERROR;
      }

      //2. parser the ip address and the port to the 
      std::ostringstream ostr;
#ifdef WIN32
      ostr << "tcp://127.0.0.1:" << port;
#else
      ostr << "ipc://localhost" << port;
#endif
      //LOG(ERROR) << "bind " << ostr.str();
      //3. binding the socket to address
      int rc = zmq_bind(zmq_socket_, ostr.str().c_str());
      if (rc != 0){
        LOG(ERROR) << "zmq bind getting error : " << zmq_errno();
        SetError(VZ_SOCKET_BIND_ERROR);
        return VZ_SOCKET_ERROR;
      }

      //4. setting the socket state and Init the zmq_pollitem, 
      //   it used for zmq_poll function
      SetZmqPollItem(zmq_socket_);
      SetSocketState(CS_CONNECTED);
      SetDispatcherState(PSTATE_ENABLE);
      return VZ_SOCKET_SUCCEED;
    }

    virtual int AsyncWrite(const char *buffer, int len) {

      //... ...
      if (GetSocketState() != CS_CONNECTED){
        SetError(VZ_SOCKET_NOT_SEND);
        //#if LOGGING
        LOG(ERROR) << "The socket is not connected you can't send any data";
        //#endif
        return VZ_SOCKET_ERROR;
      }

      if (VZ_SOCKET_SUCCEED != ZmqSend(zmq_socket_, buffer, len, ZMQ_DONTWAIT)){
        SetError(VZ_SOCKET_SEND);
        return VZ_SOCKET_ERROR;
      }
      return VZ_SOCKET_SUCCEED;
    }

    virtual int CloseSocket(){
      SetSocketState(CS_CLOSED);
      SetDispatcherState(PSTATE_DISABLE);
      return VZ_SOCKET_SUCCEED;
    }

    virtual void OnReadEvent(char *buffer){

      //DLOG(INFO) << "Beging Client Socket OnEvent .... ... ";
      if (GetSocketState() != CS_CONNECTED){
        //LOG(WARNING) << "The connect state is not the CS_CONNECTD, Can't read data";
        return;
      }
      zmq_msg_t part;
      int rc = 0;

      rc = zmq_msg_init(&part);
      CHECK(rc != VZ_SOCKET_ERROR) << "zmq_msg_init data error" << zmq_errno();
      int size = zmq_recvmsg(zmq_socket_, &part, 0);
      if (size == VZ_SOCKET_ERROR){
        LOG(WARNING) << "zmq_recvmsg getting error";
        SetError(VZ_SOCKET_RECV);
        return;
      }
      memcpy(buffer, zmq_msg_data(&part), size);
      zmq_msg_close(&part);
      //DLOG(INFO) << "Beging Client Socket Signal .... ... ";
      SignalPacketRecived(this, buffer, size);
      //DLOG(INFO) << "Ending Client Socket Signal .... ... ";
    }

  protected:
    void *zmq_context_;
    void *zmq_socket_;
    bool is_destory_;
  };

  ////////////////////////////////////////////////////////////////////////////////
  //VZClientSocket
  class VZZmqClientSocket : public VZDispatcher{
  public:
    VZZmqClientSocket(void *context)
      :zmq_context_(context),
      is_destory_(false){
      SetError(VZ_INVALUE_OPERATOR);
    }
    //
    virtual ~VZZmqClientSocket(){
      CHECK(GetSocketState() == CS_CLOSED);
    }

    virtual int Connect(uint16 port){

      if (GetSocketState() != CS_CLOSED){
        //#if LOGGING
        LOG(ERROR) << "The socket was connected";
        //#endif
        SetError(VZ_SOCKET_BIND_ERROR);
        return VZ_SOCKET_ERROR;
      }
      //DLOG(INFO) << "Starting .. connect";
      zmq_socket_ = zmq_socket(zmq_context_, ZMQ_PAIR);
      if (zmq_socket_ == NULL){
        LOG(ERROR) << "Create Zmq socket error " << zmq_errno();
        return VZ_SOCKET_ERROR;
      }

      //remote_addr_ = addr;
      std::ostringstream ostr;
#ifdef WIN32
      ostr << "tcp://127.0.0.1:" << port;
#else
      ostr << "ipc://localhost" << port;
#endif
      //LOG(ERROR) << "connect " << ostr.str();
      int rc = zmq_connect(zmq_socket_, ostr.str().c_str());
      if (rc != 0){
        SetError(VZ_SOCKET_CONNECT);
        //#if LOGGING
        LOG(ERROR) << "The zmq socket connect error and the zmq error code is "
          << zmq_errno();
        //  << zmq_errno();
        //#endif
        return VZ_SOCKET_ERROR;
      }
      //Init the zmq_pollitem, it used for zmq_poll function
      SetZmqPollItem(zmq_socket_);
      SetDispatcherState(PSTATE_ENABLE);
      SetSocketState(CS_CONNECTED);
      //add the socket to zmq_poll loop
      return VZ_SOCKET_SUCCEED;
    }

    virtual int AsyncWrite(const char *buffer, int len) {
      if (GetSocketState() != CS_CONNECTED){
        SetError(VZ_SOCKET_NOT_SEND);
        //#if LOGGING
        LOG(WARNING) << "The client socket is not connected you can't send any data";
        //#endif
        return VZ_SOCKET_ERROR;
      }

      if (VZ_SOCKET_SUCCEED != ZmqSend(zmq_socket_, buffer, len, ZMQ_DONTWAIT)){
        SetError(VZ_SOCKET_SEND);
        return VZ_SOCKET_ERROR;
      }
      return VZ_SOCKET_SUCCEED;
    }


    virtual int CloseSocket(){
      SetSocketState(CS_CLOSED);
      SetDispatcherState(PSTATE_DISABLE);
      return VZ_SOCKET_SUCCEED;
    }

    virtual void OnReadEvent(char *buffer){

      //DLOG(INFO) << "Beging Client Socket OnEvent .... ... ";
      if (GetSocketState() != CS_CONNECTED){
        //LOG(WARNING) << "The connect state is not the CS_CONNECTD, Can't read data";
        return;
      }
      zmq_msg_t part;
      int rc = 0;

      rc = zmq_msg_init(&part);
      CHECK(rc != VZ_SOCKET_ERROR) << "zmq_msg_init data error" << zmq_errno();
      int size = zmq_recvmsg(zmq_socket_, &part, 0);
      if (size == VZ_SOCKET_ERROR){
        LOG(WARNING) << "zmq_recvmsg getting error";
        SetError(VZ_SOCKET_RECV);
        return;
      }
      memcpy(buffer, zmq_msg_data(&part), size);
      zmq_msg_close(&part);
      //DLOG(INFO) << "Beging Client Socket Signal .... ... ";
      SignalPacketRecived(this, buffer, size);
      //DLOG(INFO) << "Ending Client Socket Signal .... ... ";
    }

  protected:
    //SocketAddress remote_addr_;
    void *zmq_context_;
    void *zmq_socket_;
    bool is_destory_;
  };

  typedef boost::weak_ptr<VZDispatcher> VZDispatcher_wptr;
  typedef boost::shared_ptr<VZDispatcher> VZDispatcher_ptr;

  class VZInternalServerSocket : public VZZmqServerSocket{
  public:
    VZInternalServerSocket(void *context)
      :VZZmqServerSocket(context){}

    virtual int Bind(uint16 port){

      if (GetSocketState() != CS_CLOSED){
        LOG(ERROR) << "The server sokcet state is not CS_CLOSED";
        SetError(VZ_SOCKET_BIND_ERROR);
        return VZ_SOCKET_ERROR;
      }

      //1. Create ZMQ_SOCKET Object
      zmq_socket_ = zmq_socket(zmq_context_, ZMQ_PAIR);
      if (zmq_socket_ == NULL){
        LOG(ERROR) << "Create Zmq socket error " << zmq_errno();
        return VZ_SOCKET_ERROR;
      }

      //2. parser the ip address and the port to the 
      std::ostringstream ostr;
      ostr << "inproc://internalmessage" << port;
      //LOG(ERROR) << "bind " << ostr.str();
      //3. binding the socket to address
      int rc = zmq_bind(zmq_socket_, ostr.str().c_str());
      if (rc != 0){
        LOG(ERROR) << "zmq bind getting error : " << zmq_errno();
        SetError(VZ_SOCKET_BIND_ERROR);
        return VZ_SOCKET_ERROR;
      }

      //4. setting the socket state and Init the zmq_pollitem, 
      //   it used for zmq_poll function
      SetZmqPollItem(zmq_socket_);
      SetSocketState(CS_CONNECTED);
      SetDispatcherState(PSTATE_ENABLE);
      return VZ_SOCKET_SUCCEED;
    }
  private:
  };

  class VZInternalClientSocket : public VZZmqClientSocket{
  public:
    VZInternalClientSocket(void *context)
      :VZZmqClientSocket(context){}

    virtual int Connect(uint16 port){

      if (GetSocketState() != CS_CLOSED){
        //#if LOGGING
        LOG(ERROR) << "The socket was connected";
        //#endif
        SetError(VZ_SOCKET_BIND_ERROR);
        return VZ_SOCKET_ERROR;
      }
      //DLOG(INFO) << "Starting .. connect";
      zmq_socket_ = zmq_socket(zmq_context_, ZMQ_PAIR);
      if (zmq_socket_ == NULL){
        LOG(ERROR) << "Create Zmq socket error " << zmq_errno();
        return VZ_SOCKET_ERROR;
      }

      //remote_addr_ = addr;
      std::ostringstream ostr;
      ostr << "inproc://internalmessage" << port;
      //LOG(ERROR) << "connect " << ostr.str();
      int rc = zmq_connect(zmq_socket_, ostr.str().c_str());
      if (rc != 0){
        SetError(VZ_SOCKET_CONNECT);
        //#if LOGGING
        LOG(ERROR) << "The zmq socket connect error and the zmq error code is "
          << zmq_errno();
        //  << zmq_errno();
        //#endif
        return VZ_SOCKET_ERROR;
      }
      //Init the zmq_pollitem, it used for zmq_poll function
      SetZmqPollItem(zmq_socket_);
      SetDispatcherState(PSTATE_ENABLE);
      SetSocketState(CS_CONNECTED);
      //add the socket to zmq_poll loop
      return VZ_SOCKET_SUCCEED;
    }

  private:
  };

  ////////////////////////////////////////////////////////////////////////////////////////////
  //VZZmqSocketServer
  class VZZmqSocketServer : public VZSocketServer{

    virtual ~VZZmqSocketServer(){
      if (zmq_context_ != NULL)
        zmq_ctx_destroy(zmq_context_);
      //delete ss_;
    }

    //by VZSocket Factory
    VzIpcSocket *CreateVZServerSocket() {
      VZDispatcher_ptr ipc_dispatcher(new VZZmqServerSocket(zmq_context_));
      Add(ipc_dispatcher);
      return ipc_dispatcher.get();
    }

    VzIpcSocket *CreateVZClientSocket() {
      VZDispatcher_ptr ipc_dispatcher(new VZZmqClientSocket(zmq_context_));
      Add(ipc_dispatcher);
      return ipc_dispatcher.get();
    }

    //by VZSocketServer
    void Wait(int cmsWait) {
      fWait_ = true;
      int wait_time = cmsWait;
      uint32 msStart = Time();
      uint32 cmsTotal = msStart + cmsWait;
      while (true){
        std::vector<zmq_pollitem_t > events;
        // Check new Socket;
        CheckSocketState();
        {
          {
            boost::mutex::scoped_lock lock(crit_);

            for (size_t i = 0; i < vz_dispatcher_.size(); i++){
              zmq_pollitem_t *t = vz_dispatcher_[i]->GetZmqPollItem();
              events.push_back(*t);
            }
          }
          if (events.size() == 0){
#ifdef WIN32
            Sleep(cmsWait);
#else
            sleep(cmsWait);
#endif
          }
          else {
            //DLOG(INFO) << "Starting ... ... zmq_poll  "  << events.size();
            int rc = zmq_poll(&events[0], events.size(), wait_time);
            //DLOG(INFO) << "Ending .... .... zmq_poll";
            if (rc == VZ_SOCKET_ERROR){
              LOG(ERROR) << "The Zmq Socket Poll Error" << zmq_errno();
              continue;
            }
          }
        {
          //boost::mutex::scoped_lock lock(crit_);// cr(&crit_);
          for (size_t i = 0; i < events.size(); i++){
            vz_dispatcher_[i]->OnEvent(data_buffer_.get(), events[i].revents);
          }
        }
          if (!fWait_){
            return;
          }
          if (cmsWait == KForever){
            continue;
          }
          uint32 msEnd = Time();
          if (cmsTotal > msEnd){
            wait_time = cmsTotal - msEnd;
            continue;
          }
          else{
            break;
          }
          //DLOG(INFO) << "Ending Dispacther ... ...";
        }
      }
      return;
    }

    void Add(VZDispatcher_ptr ipc_dispatcher){
      //CritScope cs(&crit_);
      boost::mutex::scoped_lock lock(crit_);
      // Prevent duplicates. This can cause dead dispatchers to stick around.
      IpcDispatchers::iterator pos = std::find(ipc_sockets_.begin(),
        ipc_sockets_.end(), ipc_dispatcher);
      if (pos != ipc_sockets_.end())
        return;
      ipc_sockets_.push_back(ipc_dispatcher);
    }

    void CheckSocketState(){

      vz_dispatcher_.clear();

      for (IpcDispatchers::iterator iter = ipc_sockets_.begin();
        iter != ipc_sockets_.end(); ++iter){
        if ((*iter)->GetDispatcherState() == PSTATE_DISABLE){
          ipc_sockets_.erase(iter);
          // Only remove once
          break;
        }
      }

      for (size_t i = 0; i < ipc_sockets_.size(); i++){
        if (ipc_sockets_[i]->GetDispatcherState() == PSTATE_ENABLE){
          vz_dispatcher_.push_back(ipc_sockets_[i].get());
        }
      }
    }
    virtual void WakeUp(){
      fWait_ = false;
      internal_client_->AsyncWrite("A", 1);
    }

  private:
    void OnPacketRecived(VzIpcSocket *socket,
      const char* data, int len){
      //Do nothing
    }
    bool InitSocketServer(bool with_new_thread){
      //ss_ = new PhysicalSocketServer();
      zmq_context_ = zmq_ctx_new();
      if (zmq_context_ == NULL){
        //#if LOGGING
        LOG(ERROR) << "Create zmq context error";
        //#endif
        return false;
      }
      internal_server_ = new VZInternalServerSocket(zmq_context_);
      VZDispatcher_ptr s_dispatcher(internal_server_);
      Add(s_dispatcher);
      internal_client_ = new VZInternalClientSocket(zmq_context_);
      VZDispatcher_ptr c_dispatcher(internal_client_);
      Add(c_dispatcher);
      
      s_dispatcher->SignalPacketRecived.connect(
        boost::bind(&VZZmqSocketServer::OnPacketRecived, this, _1, _2, _3));
      c_dispatcher->SignalPacketRecived.connect(
        boost::bind(&VZZmqSocketServer::OnPacketRecived, this, _1, _2, _3));

      if (VZ_SOCKET_ERROR == s_dispatcher->Bind(52980)){
        return false;
      }

      if (VZ_SOCKET_ERROR == c_dispatcher->Connect(52980)){
        return false;
      }

      return true;
    }

    const static int kForever = -1;
    typedef std::vector<VZDispatcher_ptr> IpcDispatchers;
    typedef std::vector<VZDispatcher*> VZDispatchers;

    boost::mutex crit_;
    void *zmq_context_;
    VZDispatchers vz_dispatcher_;
    IpcDispatchers ipc_sockets_;
    boost::scoped_array<char> data_buffer_;
    bool fWait_;
    VZInternalServerSocket *internal_server_;
    VZInternalClientSocket *internal_client_;

    //singleton parrten
  public:
    static VZZmqSocketServer *Instance(bool with_new_thread){
      if (NULL == vz_zss_instance_){
        boost::mutex::scoped_lock lock(*singleton_mutex_);
        if (NULL == vz_zss_instance_){
          vz_zss_instance_ = new VZZmqSocketServer();
          if (!vz_zss_instance_->InitSocketServer(with_new_thread)){
            delete vz_zss_instance_;
            vz_zss_instance_ = NULL;
            return NULL;
          }
        }
      }
      return vz_zss_instance_;
    }
  private:
    static boost::mutex *singleton_mutex_;
    static VZZmqSocketServer *vz_zss_instance_;
    VZZmqSocketServer()
      :zmq_context_(NULL),
      data_buffer_(new char[MAX_MESSAGE_SIZE]),
      fWait_(true){
    }
    //SocketServer *ss_;
  };

  VZZmqSocketServer *VZZmqSocketServer::vz_zss_instance_ = NULL;
  boost::mutex *VZZmqSocketServer::singleton_mutex_ = new boost::mutex();

  VZSocketServer *InitVZNetwork(bool with_new_thread){
    VZSocketServer *vz_ss = VZZmqSocketServer::Instance(with_new_thread);
    return vz_ss;
  }
}; // namesapce zipc