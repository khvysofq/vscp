// Vision Zenith System Communication Protocol (Project)

#include "vscp/proto/clientprotoanalyzer.h"
#include "vscp/base/helper.h"

namespace vscp {

  VscpClientProtoAnalyzer::VscpClientProtoAnalyzer(const std::string& local_vsid,
    const std::string& remote_vsid)
    :VscpBaseProtoAnalyzer(local_vsid, PROTO_CLIENT),
    login_state_(LOGIN_START){
    SetRemoteVsid(Vsid(remote_vsid));
  }

  bool VscpClientProtoAnalyzer::StartProtoAnalyzer(){
    // At first the client proto should send connect message to server

    JsonStanza json_stanza;
    json_stanza.from_ = GetLocalVsid().Str();
    json_stanza.to_ = GetRemoteVsid().Str();
    json_stanza.mode_ = JSON_MODE_LOGIN;
    json_stanza.id_ = GetRandomVscpid();
    json_stanza.type_ = JSON_MODE_LOGIN_TYPE_START;

    return Step(json_stanza, NULL, 0);
  }

  bool VscpClientProtoAnalyzer::StopProtoAnalyzer(){
    return true;
  }

  void VscpClientProtoAnalyzer::PingMessage(){

    if (DEFAULT_SERVER_PING_CHECK < GetPingElapsed()){
      SignalPingMessage(GetRemoteVsid(), true);
      return;
    }

    JsonStanza json_stanza;
    json_stanza.from_ = GetLocalVsid().Str();
    json_stanza.to_ = GetRemoteVsid().Str();
    json_stanza.mode_ = JSON_MODE_IQ;
    json_stanza.id_ = GetRandomVscpid();
    json_stanza.type_ = JSON_MODE_IQ_TYPE_PING;
    SendStanza(json_stanza);
  }

  bool VscpClientProtoAnalyzer::Step(JsonStanza& json_stanza, 
    const char* data, int len){

    switch (GetProtoSessionState())
    {
    case vscp::STATE_INITED:
      SendStanza(json_stanza);
      SetState(STATE_LOGGING);
      break;
    case vscp::STATE_LOGGING:
    {
      switch (login_state_)
      {
      case vscp::VscpClientProtoAnalyzer::LOGIN_START:
        if (!StartAuthentication(json_stanza)){
          SetState(STATE_DISABLE);
        }
        login_state_ = LOGIN_START_AUTH;
        break;
      case vscp::VscpClientProtoAnalyzer::LOGIN_START_AUTH:
        if (AuthenticationResutl(json_stanza)){
          login_state_ = LOGIN_AUTH_SUCCEED;
          SetState(STATE_LOGINED);
        }
        else{
          login_state_ = LOGIN_AUTH_FAILURE;
          SetState(STATE_DISABLE);
        }
        break;
      default:
        break;
      }
    }
      break;
    case vscp::STATE_LOGINED:
      if (!OnMessage(json_stanza,data,len)){
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

  bool VscpClientProtoAnalyzer::StartAuthentication(JsonStanza& json_stanza){

    if (json_stanza.to_ != GetLocalVsid().Str()
      || json_stanza.from_ != GetRemoteVsid().Str()
      || json_stanza.mode_ != JSON_MODE_LOGIN){
      return false;
    }

    JsonStanza auth_stanza;
    auth_stanza.mode_ = JSON_MODE_LOGIN;
    auth_stanza.from_ = json_stanza.to_;
    auth_stanza.to_ = json_stanza.from_;
    auth_stanza.type_ = JSON_MODE_LOGIN_TYPE_AUTH;
    auth_stanza.id_ = GetRandomVscpid();
    auth_stanza.json_data_[JSON_DATA_LOGIN_AUTH_PASS] = "12345678";
    SendStanza(auth_stanza);
    return true;
  }

  bool VscpClientProtoAnalyzer::AuthenticationResutl(JsonStanza& json_stanza){

    if (json_stanza.to_ != GetLocalVsid().Str()
      || json_stanza.from_ != GetRemoteVsid().Str()
      || json_stanza.mode_ != JSON_MODE_LOGIN){
      return false;
    }

    if (json_stanza.type_ != JSON_MODE_LOGIN_TYPE_ACCEPT_AUTH){
      return false;
    }
    return true;
  }

  bool VscpClientProtoAnalyzer::OnMessage(JsonStanza& json_stanza,
    const char* data, int len){
    if (json_stanza.mode_ == JSON_MODE_LOGOUT){
      return false;
    }
    else if (json_stanza.mode_ == JSON_MODE_IQ
      && json_stanza.type_ == JSON_MODE_IQ_TYPE_PONG){
        UpdatePingTimer();
        SignalPingMessage(GetRemoteVsid(), false);
    }
    if (len != 0){
      SignalOutbandData(GetRemoteVsid(), data, len);
    }
    // Do nothing
    return true;
  }
}; //