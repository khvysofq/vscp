// Vision Zenith System Communication Protocol (Project)
#ifndef VSCP_VSCPSESSION_H_
#define VSCP_VSCPSESSION_H_

#include "vscp/net/basesession.h"
#include "vscp/proto/baseprotoanalyzer.h"

namespace vscp {
  // This is a test name
  const char DEFAULT_CLIENT_NAME[] = "12341234asdfadfa@client";
  const char DEFAULT_SERVER_NAME[] = "asdfcvawe@lprc";

  enum {
    CLIENT_WANT_LOGIN,
    CLIENT_LOGING_SUCCEED,
    CLIENT_LOGOUT
  };

  class VscpBaseSession : public BaseSession {
  public:
    VscpBaseSession(boost::asio::io_service &io_service,
      ProtoAnalyzerType proto_analyzer_type);
    virtual ~VscpBaseSession();

    virtual void StartSession();
    bool WriteVscpData(const char* buffer, int len);
    void DisconnectVscp();

    boost::signals2::signal<void(VscpBaseSession* vscp_session,
      const Vsid& remote_vsid, const std::string& pass)> SignalClientWantLogin;

    boost::signals2::signal<void(VscpBaseSession* vscp_session,
      const Vsid& remote_vsid)> SignalClientLogined;

    boost::signals2::signal<void(VscpBaseSession* vscp_session,
      const Vsid& remote_vsid)> SignalClientLogout;

    boost::signals2::signal<void(VscpBaseSession* vscp_session,
      const char* buffer, int len)> SignalSessionVscpData;
  protected:
    void InitProtoAnalyzer();
  private:

    // Inheritanca with BaseSession
    virtual void OnSocketError(BaseSession::SessionWptr session,
      const boost::system::error_code& err);

    virtual void OnPacketRead(BaseSession::SessionWptr session, 
      const char* buffer, int buffer_size, const boost::system::error_code& err);

    virtual void OnPacketWriteComplete(BaseSession::SessionWptr session,
      const boost::system::error_code& err);

    // Callback with VscpProtoAnalyzer
    virtual void OnProtoStateChange(ProtoSessionState proto_state, int err);
    virtual void OnProtoReplyStanza(std::string data);
    virtual void OnPingMessage(const Vsid& vsid, bool timeout);
    virtual void OnOutbandData(const Vsid& vsid, const char* data, int len);

    virtual void StartPingMessage();
    void HandlePingTimeout(const boost::system::error_code& err);
  private:
    boost::scoped_array<char> send_buffer_;
    boost::scoped_ptr<VscpBaseProtoAnalyzer> proto_analyzer_;
    boost::scoped_ptr<boost::asio::deadline_timer> ping_timer_;
    int ping_check_seconds;
  }; // class session

}; // namespace vscp

#endif