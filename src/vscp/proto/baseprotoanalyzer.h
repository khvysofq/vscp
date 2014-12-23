// Vision Zenith System Communication Protocol (Project)
#ifndef VSCP_PROTO_BASEPROTOANALYZER_H_
#define VSCP_PROTO_BASEPROTOANALYZER_H_

#include "vscp/proto/protobasicincludes.h"
#include "json/json.h"

namespace vscp {

  class VscpBaseProtoAnalyzer : public boost::noncopyable {
  public:
    VscpBaseProtoAnalyzer(const std::string& local_vsid,
      ProtoAnalyzerType proto_analyzer_type);

    // TODO(guangleiHe), if return false, how to do?
    virtual bool AddPacketData(const char *buffer, int len);
    virtual void Disconnect();

    virtual bool StartProtoAnalyzer() = 0;
    virtual bool StopProtoAnalyzer() = 0;
    virtual void PingMessage() = 0;

    const Vsid& GetLocalVsid() const { return local_vsid_; }
    const Vsid& GetRemoteVsid() const { return remote_vsid_; }
    ProtoAnalyzerType GetProtoAnalyzerType() const { return analyzer_type_; }
    ProtoSessionState GetProtoSessionState() const { 
      return proto_session_state_; 
    }

    // signal 
    boost::signals2::signal < void(
      ProtoSessionState proto_session_state,int err) > SigalProtoStateChange;
    boost::signals2::signal < void(
      std::string data) > SignalProtoReplyStanza;
    boost::signals2::signal < void(
      const Vsid& vsid, bool timeout)> SignalPingMessage;
    boost::signals2::signal < void(
      const Vsid& vsid, const char* data, int len) > SignalOutbandData;

  protected:
    struct JsonStanza {
      std::string mode_;
      std::string from_;
      std::string to_;
      std::string type_;
      std::string id_;
      Json::Value json_data_;
    };
    virtual bool Step(JsonStanza& json_stanza, const char* data, int len) = 0;
    void SetState(ProtoSessionState state);
    void SetRemoteVsid(const Vsid& remote_vsid){ remote_vsid_ = remote_vsid; }
    void UpdatePingTimer();
    boost::uint64_t GetPingElapsed();

    // The best way is to separate JsonStanza members as function parameters,
    // will be avoid creates second JsonStanza object
    void SendStanza(JsonStanza& json_stanza);

    void SetError(int err){
      error_code_ = err;
    }

    int GetLastError() const{
      return error_code_;
    }


  private:

    Vsid local_vsid_;
    Vsid remote_vsid_;
    ProtoAnalyzerType analyzer_type_;
    ProtoSessionState proto_session_state_;
    int error_code_;
    boost::timer ping_start_timer_;
  }; // class VscpProtoBaseAnalyzer
}; // namespace vscp

#endif // VSCP_PROTO_BASEPROTOANALYZER_H_