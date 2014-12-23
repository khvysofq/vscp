#include "vscp/net/packettransmitter.h"
//#include <boost/archive/text_iarchive.hpp>
//#include <boost/archive/text_oarchive.hpp>

namespace vscp{

  //////////////////////////////////////////////////////////////////////////////
  SocketTask::SocketTask(boost::asio::ip::tcp::socket& socket,
    const std::vector<boost::asio::const_buffer>& buffers,
    WriteCompleteHandler write_complete_handler)
    :socket_(socket), buffers_(buffers),
    write_complete_handler_(write_complete_handler),
    buffer_(NULL), buffer_size_(0){

  }
  SocketTask::SocketTask(boost::asio::ip::tcp::socket& socket,
    const char* buffer, int buffer_size,
    WriteCompleteHandler write_complete_handler)
    : socket_(socket), write_complete_handler_(write_complete_handler),
    buffer_(buffer), buffer_size_(buffer_size){

  }

  void SocketTask::StartTask(){

    if (buffer_ == NULL && buffer_size_ == 0){
      boost::asio::async_write(socket_, buffers_,
        boost::bind(&SocketTask::HandleWriteComplete, shared_from_this(),
        boost::asio::placeholders::bytes_transferred,
        boost::asio::placeholders::error));
    }
    else{
      boost::asio::async_write(socket_,
        boost::asio::buffer(buffer_, buffer_size_),
        boost::bind(&SocketTask::HandleWriteComplete, shared_from_this(),
        boost::asio::placeholders::bytes_transferred,
        boost::asio::placeholders::error));
    }
  }

  void SocketTask::HandleWriteComplete(int send_size,
    const boost::system::error_code& err){
    SignalTaskComplete(this, err);
    write_complete_handler_(send_size, err);
  }

  //////////////////////////////////////////////////////////////////////////////
  void SocketWriteTaskManager::AsyncWriteTask(
    boost::asio::ip::tcp::socket& socket,
    const std::vector<boost::asio::const_buffer>& buffers,
    WriteCompleteHandler write_complete_handler){
    // 1. Create task
    SocketTaskPtr socket_task = SocketTaskPtr(
      new SocketTask(socket, buffers, write_complete_handler));

    socket_task->SignalTaskComplete.connect(
      boost::bind(&SocketWriteTaskManager::OnTaskComplete, this, _1, _2));
    // 2. whehre has task
    if (!is_task_run_){
      is_task_run_ = true;
      socket_task->StartTask();
    }
    else{
      socket_tasks_.push(socket_task);
    }
  }

  void SocketWriteTaskManager::AsyncWriteTask(
    boost::asio::ip::tcp::socket& socket,
    const char* buffer, int buffer_size,
    WriteCompleteHandler write_complete_handler){

    // 1. Create task
    SocketTaskPtr socket_task = SocketTaskPtr(
      new SocketTask(socket, buffer, buffer_size, write_complete_handler));

    socket_task->SignalTaskComplete.connect(
      boost::bind(&SocketWriteTaskManager::OnTaskComplete, this, _1, _2));
    // 2. whehre has task
    if (!is_task_run_){
      is_task_run_ = true;
      socket_task->StartTask();
    }
    else{
      socket_tasks_.push(socket_task);
    }
  }

  void SocketWriteTaskManager::OnTaskComplete(SocketTask* socket_task,
    const boost::system::error_code& err){
    if (socket_tasks_.size() == 0){
      is_task_run_ = false;
      return;
    }
    if (err){
      RemoveAllTask();
    }
    else{
      SocketTaskPtr socket_task = socket_tasks_.front();
      socket_tasks_.pop();
      is_task_run_ = true;
      socket_task->StartTask();
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  const static int PING_MESSAGE = 1;
  const static int NORMAL_MESSAGE = 0;

  PacketTransmitter::PacketTransmitter(
    boost::asio::io_service &io_service,
    boost::asio::ip::tcp::socket& socket,
    int ping_timeout)
    :socket_(socket),
    io_service_(io_service),
    read_time_out_(io_service,
    boost::posix_time::seconds(DEFAULT_READ_DATA_TIMEOUT)),
    state_(STATE_START),
    ping_check_seconds_(ping_timeout),
    session_type_(SESSION_TYPE_SERVER),
    socket_write_task_manager_(new SocketWriteTaskManager){

    outbound_header_[0] = 'V';
    outbound_header_[1] = 'Z';
    outbound_header_[2] = NORMAL_MESSAGE;
    outbound_header_[3] = 0;

    header_beat_[0] = 'V';
    header_beat_[1] = 'Z';
    header_beat_[2] = PING_MESSAGE;
    header_beat_[3] = 0;
    header_beat_[4] = 0;
    header_beat_[5] = 0;
    header_beat_[6] = 0;
    header_beat_[7] = 0;

    time(&ping_start_timer_);
  }
  PacketTransmitter::~PacketTransmitter(){
    if (ping_timer_){
      ping_timer_->cancel();
      ping_timer_.reset(NULL);
    }
  }
  bool PacketTransmitter::AsyncWritePacket(BaseSession::SessionPtr session,
    const char* buffer, int buffer_size, WriteHandler* write_handler){

    // LOG(INFO) << "Want write " << buffer_size << " biter";
    if (state_ != STATE_START){
      LOG(ERROR) << "The state of packet translate error";
      return false;
    }
    if (buffer_size > MAX_PACKET_SIZE){
      LOG(ERROR) << "The packet size larger than 16MB " << MAX_PACKET_SIZE;
      return false;
    }
    uint32 size = buffer_size;
    uint32 network_size = htonl(size);
    //BOOST_ASSERT(sizeof(network_size) == 4);
    memcpy(outbound_header_ + 4, &network_size, sizeof(uint32));

    // Write the serialized data to the socket. We use "gather-write" to send
    // both the header and the data in a single write operation.
    std::vector<boost::asio::const_buffer> buffers;
    buffers.push_back(boost::asio::buffer(
      outbound_header_, PACKET_SIZE_LENGTH));
    buffers.push_back(boost::asio::buffer(buffer, buffer_size));

    socket_write_task_manager_->AsyncWriteTask(socket_, buffers,
      boost::bind(&PacketTransmitter::HandleWriteComplete, this, 
      session, _1, _2, write_handler));

    return true;
  }

  bool PacketTransmitter::SynchWritePacket(BaseSession::SessionPtr session,
    const char* buffer, int buffer_size){

    if (state_ != STATE_START){
      LOG(ERROR) << "The state of packet translate error";
      return false;
    }
    if (buffer_size > MAX_PACKET_SIZE){
      LOG(ERROR) << "The packet size larger than 16MB " << MAX_PACKET_SIZE;
      return false;
    }
    uint32 size = buffer_size;
    uint32 network_size = htonl(size);
    //BOOST_ASSERT(sizeof(network_size) == PACKET_SIZE_LENGTH);
    memcpy(outbound_header_ + 4, &network_size, sizeof(uint32));

    // Write the serialized data to the socket. We use "gather-write" to send
    // both the header and the data in a single write operation.
    std::vector<boost::asio::const_buffer> buffers;
    buffers.push_back(boost::asio::buffer(
      outbound_header_, PACKET_SIZE_LENGTH));
    buffers.push_back(boost::asio::buffer(buffer, buffer_size));

    WritePacketState packet_state = PACKET_UNSEND;

    //boost::asio::async_write(socket_, buffers,
    //  boost::bind(&PacketTransmitter::HandleSynchWritePacket, this, session,
    //  boost::asio::placeholders::error,
    //  boost::ref(packet_state)));

    socket_write_task_manager_->AsyncWriteTask(socket_, buffers,
      boost::bind(&PacketTransmitter::HandleSynchWritePacket, this, session,
      _1, _2, boost::ref(packet_state)));

    while (packet_state == PACKET_UNSEND){
      io_service_.poll_one();
    }

    if (packet_state == PACKET_ERROR){
      return false;
    }
    return true;
  }

  void PacketTransmitter::SocketError(BaseSession::SessionPtr session,
    const boost::system::error_code& err){

    if (ping_timer_){
      ping_timer_->cancel();
      ping_timer_.reset(NULL);
    }
    if (state_ == STATE_START){
      state_ = STATE_ERROR;
      LOG(ERROR) << err.message();
      session->CloseSession();
      SignalSocketError(session, err);
    }
  }

  void PacketTransmitter::HandleWriteComplete(BaseSession::SessionPtr session,
    int len, const boost::system::error_code& err, WriteHandler* write_handler){
    //  LOG(INFO) << __FUNCTION__;
    if (err){
      return SocketError(session, err);
    }
    // LOG(INFO) << "Writed " << len << " biter";
    //UpdatePingTimer();
    SignalPacketWriteComplete(session, err);
  }

  void PacketTransmitter::StartReadPacket(BaseSession::SessionPtr session, 
    SessionType session_type){
    //  LOG(INFO) << __FUNCTION__;
    session_type_ = session_type;
    AsyncReadPacket(session, socket_);
    if (session_type_ == SESSION_TYPE_CLIENT){
      StartClientPingRequest(session);
    }
    else{
      StartServerPingCheck(session);
    }
  }

  void PacketTransmitter::SendPingMessage(BaseSession::SessionPtr session){

    if (state_ != STATE_START){
      LOG(ERROR) << "The state of packet translate error";
      return;
    }

    socket_write_task_manager_->AsyncWriteTask(socket_, 
      header_beat_, HEADER_BEAT_SIZE,
      boost::bind(&PacketTransmitter::HandlePingComplete, this, _1,_2, session));
  }

  void PacketTransmitter::StartClientPingRequest(BaseSession::SessionPtr session){

    UpdatePingTimer();
    ping_timer_.reset(new boost::asio::deadline_timer(io_service_,
      boost::posix_time::seconds(ping_check_seconds_)));
    ping_timer_->async_wait(boost::bind(&PacketTransmitter::HandlePingTimeout,
      this, session, boost::asio::placeholders::error));
  }

  void PacketTransmitter::StartServerPingCheck(BaseSession::SessionPtr session){

    ping_timer_.reset(new boost::asio::deadline_timer(io_service_,
      boost::posix_time::seconds(ping_check_seconds_)));
    ping_timer_->async_wait(boost::bind(&PacketTransmitter::HandlePongTimeout,
      this, session, boost::asio::placeholders::error));
  }

  void PacketTransmitter::HandlePingTimeout(BaseSession::SessionPtr session,
    const boost::system::error_code& err){

    if (err || state_ == STATE_ERROR){
      return;
    }
    if (IsTimeOut()){
      // Error
      return SocketError(session, err);
    }
    else{
      SendPingMessage(session);
      ping_timer_->expires_from_now(boost::posix_time::seconds(ping_check_seconds_));
      ping_timer_->async_wait(boost::bind(&PacketTransmitter::HandlePingTimeout,
        this, session, boost::asio::placeholders::error));
    }
  }

  void PacketTransmitter::HandlePongTimeout(BaseSession::SessionPtr session,
    const boost::system::error_code& err){

    if (err || state_ == STATE_ERROR){
      return;
    }
    LOG(WARNING) << state_;
    if (IsTimeOut()){
      // Error
      return SocketError(session, err);
    }
    else{
      ping_timer_->expires_from_now(boost::posix_time::seconds(ping_check_seconds_));
      ping_timer_->async_wait(boost::bind(&PacketTransmitter::HandlePongTimeout,
        this, session, boost::asio::placeholders::error));
    }
  }

  void PacketTransmitter::HandlePingComplete(int len, 
    const boost::system::error_code& err, BaseSession::SessionPtr session){

    if (err){
      return SocketError(session, err);
    }
    LOG(WARNING) << "Send Ping Message";
  }

  void PacketTransmitter::UpdatePingTimer(){
    time(&ping_start_timer_);
    LOG(WARNING) << "update timer ";
  }

  bool PacketTransmitter::IsTimeOut(){
    time_t current_time;
    time(&current_time);
    time_t res = current_time - ping_start_timer_;
    //boost::uint64_t res = (boost::uint64_t)(ping_start_timer_.elapsed()) * 1000;

    if (res > ping_check_seconds_ * PING_CHECK_TIMES){
      LOG(ERROR) << "Time out";
      return true;
    }
    // LOG(INFO) << "elapsed time " << res;
    return false;
  }

  bool PacketTransmitter::AsyncReadPacket(BaseSession::SessionPtr session,
    boost::asio::ip::tcp::socket& socket){
    //  LOG(INFO) << __FUNCTION__;

    if (state_ != STATE_START){
      LOG(ERROR) << "The state of packet translate error";
      return false;
    }

    boost::asio::async_read(socket, boost::asio::buffer(
      inbound_header_, PACKET_SIZE_LENGTH),
      boost::bind(&PacketTransmitter::HandleReadHeaderComplete, this, session,
      boost::asio::placeholders::bytes_transferred,
      boost::asio::placeholders::error));
    return true;
  }

  void PacketTransmitter::HandleReadHeaderComplete(
    BaseSession::SessionPtr session,
    int len, const boost::system::error_code& err){
    //  LOG(INFO) << __FUNCTION__;

    if (err){
      return SocketError(session, err);
    }
    // 
    // This is an ping message
    if (inbound_header_[2] == PING_MESSAGE){
      LOG(WARNING) << "receive ping message";
      UpdatePingTimer();
      if (session_type_ == SESSION_TYPE_SERVER){
        SendPingMessage(session);
      }
      AsyncReadPacket(session, socket_);
      return;
    }
    else if (inbound_header_[2] != NORMAL_MESSAGE) {
      LOG(ERROR) << "The Command type getting an error";
      return SocketError(session, boost::asio::error::invalid_argument);
    }
    CHECK(len == PACKET_SIZE_LENGTH);
    uint32 inbound_data_size = ntohl(*(reinterpret_cast<uint32 *>(inbound_header_ + 4)));
    //memcpy(&inbound_data_size, inbound_header_, PACKET_SIZE_LENGTH);

    if (!inbound_data_size || inbound_data_size > MAX_PACKET_SIZE
      || inbound_header_[0] != 'V' || inbound_header_[1] != 'Z') {
      LOG(ERROR) << "Receive packet length error " << inbound_data_size;
      // Header doesn't seem to be valid. Inform the caller.
      return SocketError(session, boost::asio::error::invalid_argument);
    }
    // LOG(INFO) << "Want read data size " << inbound_data_size << " bityer";
    inbound_data_.resize(inbound_data_size);
    boost::asio::async_read(socket_, boost::asio::buffer(inbound_data_),
      boost::bind(&PacketTransmitter::HandleReadDataComplete, this, session,
      boost::asio::placeholders::bytes_transferred,
      boost::asio::placeholders::error));

    // Setting read data timeout
    read_time_out_.expires_from_now(
      boost::posix_time::seconds(DEFAULT_READ_DATA_TIMEOUT));
    read_time_out_.async_wait(
      boost::bind(&PacketTransmitter::HandleReadDataTimeout, this, session,
      boost::asio::placeholders::error));
  }

  void PacketTransmitter::HandleReadDataTimeout(BaseSession::SessionPtr session,
    const boost::system::error_code& err){

    if (!err){
      LOG(ERROR) << "Read data time out ... ...";
      if (socket_.is_open())
        socket_.close();
      return SocketError(session, boost::asio::error::timed_out);
    }

  }

  void PacketTransmitter::HandleReadDataComplete(BaseSession::SessionPtr session,
    int len, const boost::system::error_code& err){
    //  LOG(INFO) << __FUNCTION__;

    if (err){
      return SocketError(session, err);
    }
    // LOG(INFO) << "readed data size " << len << " bityer";
    // UpdatePingTimer();
    //read_time_out_.cancel();
    read_time_out_.cancel_one();
    SignalPacketRead(session, &inbound_data_[0], len, err);

    AsyncReadPacket(session, socket_);
  }

  void PacketTransmitter::HandleSynchWritePacket(BaseSession::SessionPtr session,
    int len, const boost::system::error_code& err, 
    WritePacketState& write_state){

    if (!err){
      write_state = PACKET_SUCCEED;
    }
    // UpdatePingTimer();
    write_state = PACKET_ERROR;
  }

}; // namespace vscp
