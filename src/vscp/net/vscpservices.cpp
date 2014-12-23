#include "vscp/net/vscpservices.h"


namespace vscp {

  VscpServices::VscpServices()
  :state_(STATE_NON),
  is_time_out_(false){
  }

  bool VscpServices::InitVscpServices(){
    
    if (state_ != STATE_NON){
      LOG(ERROR) << "Can't Reinit the vscpservices";
      return false;
    }

    try {
      io_service_work_.reset(new boost::asio::io_service::work(io_service_));
    }
    catch (std::exception& e){
      LOG(ERROR) << e.what();
      return false;
    }

    state_ = STATE_INITED;
    return true;
  }

  bool VscpServices::RunVscpServices(int millisecond){

    if (state_ != STATE_INITED){
      return false;
    }
    else{
      state_ = STATE_STARTED;
    }
    try{
      if (millisecond == FOREVER){
        io_service_.run();
      }
      else{
        // Create a timeout object, that the time setting with millisecond
        boost::shared_ptr<boost::asio::deadline_timer> timeout(
          new boost::asio::deadline_timer(io_service_,
          boost::posix_time::milliseconds(millisecond)));

        // Setting the timeout flage with false
        is_time_out_ = false;

        // async wait
        timeout->async_wait(boost::bind(&VscpServices::HandleTimeout,
          this, timeout, boost::asio::placeholders::error));

        // Run untill the timeout is coming
        while (!is_time_out_){
          io_service_.run_one();
        }
      }
    }
    catch (std::exception& e){
      LOG(ERROR) << e.what();
      return false;
    }
    return true;
  }

  void VscpServices::StopVscpServices(){
    
    if (state_ != STATE_STARTED){
      return;
    }
    state_ = STATE_DISABLE;
    try{
      // allow the io service return in run
      io_service_work_.reset();
      // Stop the io service, if you run with a thread, should call the 
      // thread.join to waitting all the task discard
      if (!io_service_.stopped())
        io_service_.stop();
    }
    catch (std::exception& e){
      LOG(ERROR) << "Stop the vscp server getting an exception " << e.what();
    }
  }

  bool VscpServices::PostHandler(boost::function<void()> callback_handler){

    if (state_ != STATE_STARTED){
      LOG(ERROR) << "VscpServices not init, you can't post a task with it";
      return false;
    }
    io_service_.post(callback_handler);
    return true;
  }

  VscpServerPtr VscpServices::CreateVscpServer(BaseSessionManager* session_manager,
    const std::string server_address, uint16 server_port){
    if (state_ == STATE_NON || state_ == STATE_DISABLE || session_manager == NULL){
      LOG(ERROR) << "Can't create the vscpserver, the state is not inited";
      return VscpServerPtr();
      //return nullptr;
    }
    return VscpServerPtr(new VscpServer(
      io_service_, session_manager, server_address, server_port));
  }

  BaseSession::SessionPtr VscpServices::CreateClientSession(
    BaseSessionManager* session_manager,
    const std::string server_address, uint16 server_port){

    if (state_ == STATE_NON || state_ == STATE_DISABLE || session_manager == NULL){
      LOG(ERROR) << "Can't create the vscp client, the state is not inited";
      return BaseSession::SessionPtr();
      //return nullptr;
    }
    BaseSession::SessionPtr session(
      session_manager->CreateClientSession(io_service_));

    io_service_.post(
      boost::bind(&VscpServices::HandleClientSessionInit, this, 
      session, server_address, server_port));

    return session;
  }

  void VscpServices::HandleTimeout(
    boost::shared_ptr<boost::asio::deadline_timer> timeout,
    const boost::system::error_code& err){
#ifdef _DEBUG
    LOG(WARNING) << "Run out of time";
#endif
    // Setting the timeout flage with true, it stop the io_service_->run_one();
    is_time_out_ = true;
  }

  void VscpServices::HandleClientSessionInit( 
    BaseSession::SessionPtr client_session, 
    std::string server_address, 
    uint16 server_port){
    if (state_ != STATE_STARTED){
      LOG(ERROR) << "State error, the state is not inited";
    }
    if (client_session){
      client_session->StartConnectServer(server_address, server_port);
    }
    else{
      LOG(ERROR) << "The client session is null";
      // should not happend
      BOOST_ASSERT(0);
    }
  }

}; // namespace vscp