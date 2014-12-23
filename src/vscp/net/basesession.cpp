// Vision Zenith System Communication Protocol (Project)
#include "vscp/net/basesession.h"
#include "vscp/net/packettransmitter.h"

namespace vscp{

  BaseSession::BaseSession(boost::asio::io_service &io_service,
    int ping_timeout)
    :socket_(io_service),
    io_service_(io_service),
    packet_transmitter_(NULL),
    state_(STATE_DISABLE),
    ping_check_seconds_(ping_timeout){

  }

  BaseSession::~BaseSession(){
    // TODO, this delete command is not comfortable, remove this
    LOG(WARNING) << "Delete base session ... ...";
    if (packet_transmitter_){
      delete packet_transmitter_;
      packet_transmitter_ = NULL;
    }
    if (state_ == STATE_ENABLE){
      CloseSession();
    }
    //std::cout << "delete this session ... ..." << std::endl;
  };

  void BaseSession::CloseSession(){
    LOG(WARNING) << "post close message... ...";
    io_service_.post(
      boost::bind(&BaseSession::HandleClose, shared_from_this()));
  }

  void BaseSession::InitServerSession(){

    InitPacketTransmitter();
    state_ = STATE_ENABLE;
    // with server side never error
    OnPrepareStart(boost::system::error_code());
    RealStartSession(SESSION_TYPE_SERVER);
  }

  void BaseSession::StartConnectServer(
    std::string server_address, uint16 server_port){
    boost::asio::ip::tcp::endpoint server_point(
      boost::asio::ip::address().from_string(server_address), server_port);
    socket_.async_connect(server_point,
      boost::bind(&BaseSession::HandleConnect, shared_from_this(),
      boost::asio::placeholders::error));
  }

  void BaseSession::InitPacketTransmitter(){
    // Start to read and write data
    packet_transmitter_ = new PacketTransmitter(
      io_service_, socket_, ping_check_seconds_);

    packet_transmitter_->SignalPacketRead.connect(
      boost::bind(&BaseSession::OnPacketRead, this, _1, _2, _3, _4));
    packet_transmitter_->SignalPacketWriteComplete.connect(
      boost::bind(&BaseSession::OnPacketWriteComplete, this, _1, _2));
    packet_transmitter_->SignalSocketError.connect(
      boost::bind(&BaseSession::OnSocketError, this, _1, _2));
  }
  void BaseSession::HandleConnect(const boost::system::error_code& err){
    if (err){
      OnPrepareStart(err);
    }
    else{
      InitPacketTransmitter();
      state_ = STATE_ENABLE;
      RealStartSession(SESSION_TYPE_CLIENT);
      OnPrepareStart(err);
    }
  }

  void BaseSession::HandleClose(){
    LOG(WARNING) << "realy to close session... ...";
    if (packet_transmitter_)
      packet_transmitter_->SetClose();
    boost::mutex::scoped_lock close_mutex(close_socket_mutex_);
    if (state_ != STATE_ENABLE){
      return;
    }
    if (socket_.is_open()){
      socket_.close();
    }

    state_ = STATE_DISABLE;
  }

  void BaseSession::RealStartSession(SessionType session_type){
    packet_transmitter_->StartReadPacket(shared_from_this(), session_type);
  }

  bool BaseSession::AsyncWritePacket(const char* buffer, int buffer_size,
    WriteHandler *write_handler){
    io_service_.post(
      boost::bind(&BaseSession::HandleAsyncWritePacket, shared_from_this(),
      buffer, buffer_size, write_handler));
    return true;
  }

  void BaseSession::HandleAsyncWritePacket(const char* buffer, int buffer_size,
    WriteHandler *write_handler){

    if (state_ != STATE_ENABLE){
      LOG(ERROR) << "The session is disable";
      return ;
    }
    packet_transmitter_->AsyncWritePacket(shared_from_this(),
      buffer, buffer_size, write_handler);
  }

  bool BaseSession::SynchWritePacket(const char* buffer, int buffer_size){

    if (state_ != STATE_ENABLE){
      LOG(ERROR) << "The session is disable";
      return false;
    }

    return packet_transmitter_->SynchWritePacket(shared_from_this(),
      buffer, buffer_size);
  }

}; // namespace vscp