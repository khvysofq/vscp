// Vision Zenith System Communication Protocol (Project)

#include "vscp/vscp/vscpbasesession.h"
#include "vscp/proto/serverprotoanalyzer.h"
#include "vscp/proto/clientprotoanalyzer.h"

namespace vscp {

  VscpBaseSession::VscpBaseSession(boost::asio::io_service &io_service,
    ProtoAnalyzerType proto_analyzer_type)
    :BaseSession(io_service),
    send_buffer_(new char[8192]){

    if (proto_analyzer_type == PROTO_SERVER){
      proto_analyzer_.reset(
        new VscpServerProtoAnalyzer(DEFAULT_SERVER_NAME));
      ping_check_seconds = DEFAULT_SERVER_PING_CHECK;
    }
    else if (proto_analyzer_type == PROTO_CLIENT){
      proto_analyzer_.reset(
        new VscpClientProtoAnalyzer(DEFAULT_CLIENT_NAME, DEFAULT_SERVER_NAME));
      ping_check_seconds = DEFAULT_CLIENT_PING_TIMEOUT;
    }
    else{
      // Never reach here
      CHECK(0);
    }
  }

  VscpBaseSession::~VscpBaseSession(){

    SignalClientWantLogin.disconnect_all_slots();
    SignalClientLogined.disconnect_all_slots();
    SignalClientLogout.disconnect_all_slots();
    SignalSessionVscpData.disconnect_all_slots();
    //#ifdef _DEBUG
    //    std::cout << "Delete vscp base session ... ..." << std::endl;
    //#endif
  }

  void VscpBaseSession::StartSession(){
    // Start to read packets loop
    StartReadPacket();
    InitProtoAnalyzer();
  }

  bool VscpBaseSession::WriteVscpData(const char* buffer, int len){
    proto_analyzer_->AddPacketData(buffer, len);
    return true;
  }

  void VscpBaseSession::DisconnectVscp(){
    proto_analyzer_->Disconnect();
  }

  void VscpBaseSession::InitProtoAnalyzer(){

    // Set proto_analyzer_ signal callback function
    proto_analyzer_->SigalProtoStateChange.connect(
      boost::bind(&VscpBaseSession::OnProtoStateChange, this, _1, _2));
    proto_analyzer_->SignalProtoReplyStanza.connect(
      boost::bind(&VscpBaseSession::OnProtoReplyStanza, this, _1));
    proto_analyzer_->SignalPingMessage.connect(
      boost::bind(&VscpBaseSession::OnPingMessage, this, _1, _2));
    proto_analyzer_->SignalOutbandData.connect(
      boost::bind(&VscpBaseSession::OnOutbandData, this, _1, _2, _3));
    // Where to set the VscpServerProtoAnalyzer::StopProtoAnalyzer
    proto_analyzer_->StartProtoAnalyzer();
  }

  void VscpBaseSession::OnSocketError(BaseSession::SessionWptr session,
    const boost::system::error_code& err){
    LOG(ERROR) << __FUNCTION__ << "Socket Error";
    CloseSocket();
    if (ping_timer_){
      ping_timer_->cancel();
      ping_timer_.reset(NULL);
    }
  }

  void VscpBaseSession::OnPacketRead(BaseSession::SessionWptr session,
    const char* buffer, int buffer_size, const boost::system::error_code& err){
    //  LOG(INFO) << __FUNCTION__ << "data size " << buffer_size;

#ifdef _DEBUG
    std::string outstr(buffer, buffer_size);
    LOG(INFO) << "<<<< " << outstr;
#endif
    proto_analyzer_->AddPacketData(buffer, buffer_size);
  }

  void VscpBaseSession::OnPacketWriteComplete(BaseSession::SessionWptr session,
    const boost::system::error_code& err){
    //  LOG(INFO) << __FUNCTION__ << "packet write complete";
  }

  // Callback with VscpProtoAnalyzer
  void VscpBaseSession::OnProtoStateChange(ProtoSessionState proto_state, int err){

    switch (proto_state)
    {
    case vscp::STATE_INITED:
      // Never reach here
      CHECK(0);
      break;
    case vscp::STATE_LOGGING:
      LOG(INFO) << "The protocol state changed with STATE_LOGGING";
      SignalClientWantLogin(this, proto_analyzer_->GetRemoteVsid(), "123456");
      break;
    case vscp::STATE_LOGINED:
      LOG(INFO) << "The protocol state changed with STATE_LOGINED";
      StartPingMessage();
      SignalClientLogined(this, proto_analyzer_->GetRemoteVsid());
      break;
    case vscp::STATE_LOGOUT:
      if (ping_timer_){
        ping_timer_->cancel();
        ping_timer_.reset(NULL);
      }
      LOG(INFO) << "The protocol state changed with STATE_LOGOUT";
      SignalClientLogout(this, proto_analyzer_->GetRemoteVsid());
      break;
    case vscp::STATE_DISABLE:
      if (ping_timer_){
        ping_timer_->cancel();
        ping_timer_.reset(NULL);
      }
      LOG(INFO) << "The protocol state changed with STATE_DISABLE";
      CloseSocket();
      break;
    default:
      break;
    }
  }

  void VscpBaseSession::OnProtoReplyStanza(std::string data){
    // Needs to copy each data and then send it.
    // send_buffer_.get();
    // How to manager the send buffer?
#ifdef _DEBUG
    LOG(INFO) << ">>>> " << data;
#endif
    //memcpy(send_buffer_.get(), data.c_str(), data.length());
    AsyncWritePacket(data.data(), data.length());
  }

  void VscpBaseSession::OnPingMessage(const Vsid& vsid, bool timeout){
    if (timeout){
      LOG(ERROR) << "remote peer timeout";
      CloseSocket();
    }
    else{
      LOG(INFO) << "ping message";
    }
  }

  void VscpBaseSession::OnOutbandData(const Vsid& vsid,
    const char* data, int len) {
    SignalSessionVscpData(this, data, len);
  }

  void VscpBaseSession::StartPingMessage(){

    ping_timer_.reset(new boost::asio::deadline_timer(GetIoService(),
      boost::posix_time::seconds(ping_check_seconds)));

    ping_timer_->async_wait(boost::bind(&VscpBaseSession::HandlePingTimeout,
      this, boost::asio::placeholders::error));
  }

  void VscpBaseSession::HandlePingTimeout(const boost::system::error_code& err){

    if (err){
      return;
    }
    proto_analyzer_->PingMessage();
    ping_timer_->expires_from_now(boost::posix_time::seconds(ping_check_seconds));
    ping_timer_->async_wait(boost::bind(&VscpBaseSession::HandlePingTimeout,
      this, boost::asio::placeholders::error));
  }

}; // namespace vscp