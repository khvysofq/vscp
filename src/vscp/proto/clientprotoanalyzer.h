// Vision Zenith System Communication Protocol (Project)
#ifndef VSCP_PROTO_SERVER_PROTOANALYZER_H_
#define VSCP_PROTO_SERVER_PROTOANALYZER_H_

#include "vscp/proto/baseprotoanalyzer.h"

namespace vscp {

  class VscpClientProtoAnalyzer : public VscpBaseProtoAnalyzer{

  public:
    VscpClientProtoAnalyzer(const std::string& local_vsid, 
      const std::string& remote_vsid);

    // TODO(guangleiHe), if return false, how to do?
    // virtual bool AddPacketData(const char *buffer, int len);

    // For client StartProtoAnalyzer method, it should to create a login message
    virtual bool StartProtoAnalyzer();
    virtual bool StopProtoAnalyzer();
    virtual void PingMessage();

    // Test method
    void SendEchoMsg();

  private:
    enum {
      LOGIN_START,
      LOGIN_START_AUTH,
      LOGIN_AUTH_SUCCEED,
      LOGIN_AUTH_FAILURE
    }login_state_;

    bool Step(JsonStanza& json_stanza, const char* data, int len);
    bool StartAuthentication(JsonStanza& json_stanza);
    bool AuthenticationResutl(JsonStanza& json_stanza);
    bool OnMessage(JsonStanza& json_stanza, const char* data, int len);
    // Test method
  private:
  };

}; // namespace vscp

#endif // VSCP_PROTO_SERVER_PROTOANALYZER_H_