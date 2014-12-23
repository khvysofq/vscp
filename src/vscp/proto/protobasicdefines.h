// Vision Zenith System Communication Protocol (Project)
#ifndef VSCP_PROTO_PROTO_BASIC_DEFINES_H_
#define VSCP_PROTO_PROTO_BASIC_DEFINES_H_

namespace vscp {

  const char STR_EMPTY[] = "";

  enum ProtoAnalyzerType{
    PROTO_UNKOWN,
    PROTO_SERVER,
    PROTO_CLIENT
  };

  // The protocol state with a session
  enum ProtoSessionState{
    STATE_INITED,
    STATE_LOGGING,
    STATE_LOGINED,
    STATE_LOGOUT,
    STATE_DISABLE
  }; // ProtoSessionState

  const boost::uint64_t DEFAULT_CLIENT_PING_TIMEOUT = 6;
  const boost::uint64_t DEFAULT_SERVER_PING_CHECK = 36;

  // Basic VSCP defines
  const char JSON_MODE[] = "mode";
  const char JSON_FROM[] = "from";
  const char JSON_TO[] = "to";
  const char JSON_ID[] = "id";
  const char JSON_TYPE[] = "type";
  const char JSON_DATA[] = "data";

  // VSCP json mode type define 
  const char JSON_MODE_LOGIN[] = "login";
  const char JSON_MODE_LOGOUT[] = "logout";
  const char JSON_MODE_IQ[] = "iq";
  const char JSON_MODE_MESSAGE[] = "message";
  const char JSON_MODE_PRESENCE[] = "presence";

  // VSCP JSON_MODE_LOGIN type defines
  const char JSON_MODE_LOGIN_TYPE_START[] = "start";
  const char JSON_MODE_LOGIN_TYPE_ACCEPT_START[] = "accept_start";
  const char JSON_MODE_LOGIN_TYPE_AUTH[] = "auth";
  const char JSON_MODE_LOGIN_TYPE_ACCEPT_AUTH[] = "accept_auth";

  // VSCP JSON_LOGIN_DATA Authentication
  const char JSON_DATA_LOGIN_AUTH_PASS[] = "pass";

  // VSCP JSON_MODE_LOGOUT type defines
  const char JSON_MODE_LOGOUT_TYPE[] = "end_session";

  // VSCP JSON_MODE_IQ type defines
  const char JSON_MODE_IQ_TYPE_PING[] = "ping";
  const char JSON_MODE_IQ_TYPE_PONG[] = "pong";
  const char JSON_MODE_IQ_TYPE_GET[] = "get";
  const char JSON_MODE_IQ_TYPE_SET[] = "set";
  const char JSON_MODE_IQ_TYPE_RES[] = "result";

}; // namespace vscp

#endif // VSCP_PROTO_PROTO_BASIC_DEFINES_H_