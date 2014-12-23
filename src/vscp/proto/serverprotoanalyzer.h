// Vision Zenith System Communication Protocol (Project)
#ifndef VSCP_PROTO_SERVER_PROTO_ANALYZER_H_
#define VSCP_PROTO_SERVER_PROTO_ANALYZER_H_

#include "json/json.h"

#include "vscp/proto/protobasicincludes.h"
#include "vscp/proto/baseprotoanalyzer.h"

namespace vscp {

  class VscpServerProtoAnalyzer : public VscpBaseProtoAnalyzer{
  public:
    VscpServerProtoAnalyzer(const std::string& local_vsid);

    // TODO(guangleiHe), if return false, how to do?
    // virtual bool AddPacketData(const char *buffer, int len);

    // As server protocol, this method do nothing 
    virtual bool StartProtoAnalyzer();
    virtual bool StopProtoAnalyzer();
    virtual void PingMessage();

  private:

    virtual bool Step(JsonStanza& json_stanza, const char* data, int len);
    bool StartConnectMsg(JsonStanza& json_stanza);
    bool ClientAuthentication(JsonStanza& json_stanza);
    // Test method
    bool ServerEchoMessage(JsonStanza& json_stanza);
    bool OnMessage(JsonStanza& json_stanza, const char* data, int len);

  private:
  }; // class VscpProtoAnalyzer

}; // namespace vscp

#endif // VSCP_PROTO_SERVER_PROTO_ANALYZER_H_