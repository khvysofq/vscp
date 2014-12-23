// Vision Zenith System Communication Protocol (Project)
#include "vscp/proto/serverprotoanalyzer.h"

namespace vscp {

  VscpServerProtoAnalyzer::VscpServerProtoAnalyzer( 
    const std::string& local_vsid)
    :VscpBaseProtoAnalyzer(local_vsid, PROTO_SERVER){

  }

  bool VscpServerProtoAnalyzer::StartProtoAnalyzer(){
    return true;
  }

  bool VscpServerProtoAnalyzer::StopProtoAnalyzer(){
    return true;
  }

  void VscpServerProtoAnalyzer::PingMessage(){
    if (DEFAULT_SERVER_PING_CHECK < GetPingElapsed()){
      SignalPingMessage(GetRemoteVsid(), true);
    }
  }

  bool VscpServerProtoAnalyzer::Step(JsonStanza& json_stanza, 
    const char* data, int len) {

    switch (GetProtoSessionState())
    {
    case vscp::STATE_INITED:
      SetRemoteVsid(Vsid(json_stanza.from_));
      if (StartConnectMsg(json_stanza)){
        SetState(STATE_LOGGING);
        return true;
      }
      else{
        SetState(STATE_DISABLE);
        return false;
      }
      break;
    case vscp::STATE_LOGGING:
      if (ClientAuthentication(json_stanza)){
        SetState(STATE_LOGINED);
        UpdatePingTimer();
      }
      else{
        SetState(STATE_DISABLE);
      }
      break;
    case vscp::STATE_LOGINED:
      if (!OnMessage(json_stanza, data, len)){
        SetState(STATE_DISABLE);
      }
      break;
    case vscp::STATE_LOGOUT:
      break;
    case vscp::STATE_DISABLE:
      break;
    default:
      break;
    }
    return true;
  }

  bool VscpServerProtoAnalyzer::StartConnectMsg(JsonStanza& json_stanza){
    
    // Don't check the remote use id. Whoever to connect is crrect 
    if (json_stanza.mode_ != JSON_MODE_LOGIN
      || json_stanza.type_ != JSON_MODE_LOGIN_TYPE_START){
      return false;
    }

    // Default to accept this session, when refused it ?
    JsonStanza reply_stanza;
    reply_stanza.mode_ = json_stanza.mode_;
    reply_stanza.from_ = GetLocalVsid().Str();
    reply_stanza.to_ = json_stanza.from_;
    reply_stanza.id_ = json_stanza.id_;
    reply_stanza.type_ = JSON_MODE_LOGIN_TYPE_ACCEPT_START;
    SendStanza(reply_stanza);
    return true;
  }

  bool VscpServerProtoAnalyzer::ClientAuthentication(JsonStanza& json_stanza){

    if (json_stanza.to_ != GetLocalVsid().Str()
      || json_stanza.from_ != GetRemoteVsid().Str()
      || json_stanza.mode_ != JSON_MODE_LOGIN){
      return false;
    }

    // Default to accept this session, when refused it ?
    JsonStanza auth_stanza;
    auth_stanza.mode_ = json_stanza.mode_;
    auth_stanza.from_ = json_stanza.to_;
    auth_stanza.to_ = json_stanza.from_;
    auth_stanza.type_ = JSON_MODE_LOGIN_TYPE_ACCEPT_AUTH;
    auth_stanza.id_ = json_stanza.id_;

    SendStanza(auth_stanza);
    return true;
  }


  bool VscpServerProtoAnalyzer::ServerEchoMessage(JsonStanza& json_stanza){

    if (json_stanza.to_ != GetLocalVsid().Str()
      || json_stanza.from_ != GetRemoteVsid().Str()){
      return false;
    }

    JsonStanza reply_stanza;
    reply_stanza.mode_ = json_stanza.mode_;
    reply_stanza.from_ = json_stanza.to_;
    reply_stanza.to_ = json_stanza.from_;
    reply_stanza.type_ = JSON_MODE_LOGIN_TYPE_AUTH;
    reply_stanza.id_ = json_stanza.id_;
    SendStanza(reply_stanza);
    return true;
  }

  bool VscpServerProtoAnalyzer::OnMessage(JsonStanza& json_stanza,
    const char* data, int len){
    if (json_stanza.mode_ == JSON_MODE_LOGOUT){
      return false;
    }
    else if (json_stanza.mode_ == JSON_MODE_IQ
      && json_stanza.type_ == JSON_MODE_IQ_TYPE_PING){

      UpdatePingTimer();
      SignalPingMessage(GetRemoteVsid(), false);

      JsonStanza pong_stanza;
      pong_stanza.from_ = json_stanza.to_;
      pong_stanza.to_ = json_stanza.from_;
      pong_stanza.mode_ = json_stanza.mode_;
      pong_stanza.id_ = json_stanza.id_;
      pong_stanza.type_ = JSON_MODE_IQ_TYPE_PONG;
      SendStanza(pong_stanza);
    }

    if (len != 0){
      SignalOutbandData(GetRemoteVsid(), data, len);
    }
    // Do nothing
    return true;
  }
}; // namespace vscp