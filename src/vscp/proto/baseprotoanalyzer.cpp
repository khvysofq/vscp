// Vision Zenith System Communication Protocol (Project)
#include "vscp/proto/baseprotoanalyzer.h"
#include "vscp/base/helper.h"
namespace vscp {
  VscpBaseProtoAnalyzer::VscpBaseProtoAnalyzer(const std::string& local_vsid,
    ProtoAnalyzerType proto_analyzer_type)
    :local_vsid_(local_vsid),
    analyzer_type_(proto_analyzer_type),
    proto_session_state_(STATE_INITED){
  }

  bool VscpBaseProtoAnalyzer::AddPacketData(const char *buffer, int len){

    // Parser json data
    Json::Value root;
    Json::Reader reader;
    std::string input_json(buffer, len);

    if (!reader.parse(input_json, root)){
      LOG(ERROR) << "Parse input json string failure";
      return false;
    }
    JsonStanza json_stanza;
    json_stanza.mode_ = root[JSON_MODE].asString();
    json_stanza.from_ = root[JSON_FROM].asString();
    json_stanza.to_ = root[JSON_TO].asString();
    json_stanza.type_ = root[JSON_TYPE].asString();
    json_stanza.id_ = root[JSON_ID].asString();
    return Step(json_stanza, NULL, 0);
  }

  void VscpBaseProtoAnalyzer::Disconnect(){
    JsonStanza disconnect_stanza;
    disconnect_stanza.mode_ = JSON_MODE_LOGOUT;
    disconnect_stanza.from_ = GetLocalVsid().Str();
    disconnect_stanza.to_ = GetRemoteVsid().Str();
    disconnect_stanza.type_ = JSON_MODE_LOGOUT_TYPE;
    disconnect_stanza.id_ = GetRandomVscpid();
    SendStanza(disconnect_stanza);
    SetState(STATE_DISABLE);
  }

  void VscpBaseProtoAnalyzer::SetState(ProtoSessionState state){
    CHECK(proto_session_state_ != state);
    proto_session_state_ = state;
    SigalProtoStateChange(state, GetLastError());
  }

  void VscpBaseProtoAnalyzer::UpdatePingTimer(){
    ping_start_timer_ = boost::timer();
    LOG(INFO) << "update timer " ;
  }

  boost::uint64_t VscpBaseProtoAnalyzer::GetPingElapsed(){
    boost::uint64_t res = (boost::uint64_t)(ping_start_timer_.elapsed());
    LOG(INFO) << "elapsed time " <<  res;
    return res;
  }

  void VscpBaseProtoAnalyzer::SendStanza(JsonStanza& json_stanza){

    Json::FastWriter writer;
    Json::Value reply_value;

    reply_value[JSON_MODE] = json_stanza.mode_;
    reply_value[JSON_FROM] = json_stanza.from_;
    reply_value[JSON_TO] = json_stanza.to_;
    reply_value[JSON_ID] = json_stanza.id_;
    reply_value[JSON_TYPE] = json_stanza.type_;

    if (!json_stanza.json_data_.isNull()){
      reply_value[JSON_DATA] = json_stanza.json_data_;
    }

    SignalProtoReplyStanza(writer.write(reply_value));
  }
}; // namespace vscp