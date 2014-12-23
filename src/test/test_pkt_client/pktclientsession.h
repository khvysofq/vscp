#ifndef TEST_PACKET_CLIENT_SESSION_H_
#define TEST_PACKET_CLIENT_SESSION_H_

#include "vscp/net/basesessionmanager.h"


const int MAX_PACKET_SIZE = 1024 * 512;
const int MIN_PACKET_SIZE = 1;
//------------------------------------------------------------------------------
// This is a base client session
class ClientSession : public vscp::BaseSession{
public:
  ClientSession(boost::asio::io_service &io_service,
    int ping_timeout);
  boost::signals2::signal < void(BaseSession::SessionWptr session) >
    SignalTaskComplete;
  virtual void OnPrepareStart(const boost::system::error_code& err);
  virtual void OnSocketError(BaseSession::SessionWptr session,
    const boost::system::error_code& err);
  virtual void OnPacketRead(BaseSession::SessionWptr session, const char* buffer,
    int buffer_size, const boost::system::error_code& err);
  virtual void OnPacketWriteComplete(BaseSession::SessionWptr session,
    const boost::system::error_code& err);
protected:
  const int MAX_PACKET_SIZE = 1024 * 512;
  const int MIN_PACKET_SIZE = 1;
  boost::scoped_array<char> buffer_;
};

//------------------------------------------------------------------------------
class DownloadSession : public ClientSession{
public:
  DownloadSession(boost::asio::io_service &io_service,
    int ping_timeout, int download_size, int packet_size)
    :ClientSession(io_service, ping_timeout),
    download_data_size_(download_size),
    current_download_(0),
    packet_size_(packet_size){
  }
  virtual void OnPrepareStart(const boost::system::error_code& err){
    if (err){
      LOG(ERROR) << "Connect Server getting an error";
      return;
    }
    AsyncWritePacket(buffer_.get(), 4096);
  }
  virtual void OnPacketRead(BaseSession::SessionWptr session, const char* buffer,
    int buffer_size, const boost::system::error_code& err){
    if (download_data_size_ > current_download_++){
      AsyncWritePacket(buffer, buffer_size);
    }
    else{
      CloseSession();
      SignalTaskComplete(session);
    }
  }
private:
  int download_data_size_;
  int current_download_;
  int packet_size_;
};

class DownloadSessionManager : public vscp::BaseSessionManager{
public:
  DownloadSessionManager() :task_count_(0){
  }
  boost::signals2::signal<void(DownloadSessionManager* sdm)> SignalTaskComplete;

  virtual vscp::BaseSession::SessionPtr CreateClientSession(
    boost::asio::io_service &io_service){
    task_count_++;
    LOG(INFO) << "Create a task " << task_count_;
    DownloadSession* ds = new DownloadSession(io_service, 10, 512, 1);
    ds->SignalTaskComplete.connect(
      boost::bind(&DownloadSessionManager::OnTaskComplete, this, _1));
    // ping timeout with 10 s
    return vscp::BaseSession::SessionPtr(ds);
  }
  // return None, only service with server session
  virtual vscp::BaseSession::SessionPtr CreateServerSession(
    boost::asio::io_service &io_service){
    return vscp::BaseSession::SessionPtr(NULL);
  };
  void OnTaskComplete(vscp::BaseSession::SessionWptr session){
    task_count_--;
    LOG(INFO) << "Remove task " << task_count_;
    if (task_count_ <= 0){
      SignalTaskComplete(this);
    }
  }
private:
  int task_count_;
};

//------------------------------------------------------------------------------
class SessionNormalCloseTest : public ClientSession{
public:
  SessionNormalCloseTest(boost::asio::io_service &io_service,
    int ping_timeout)
    :ClientSession(io_service, ping_timeout){
  }
  virtual void OnPacketRead(BaseSession::SessionWptr session, const char* buffer,
    int buffer_size, const boost::system::error_code& err){
    CloseSession();
    SignalTaskComplete(session);
  }
private:
};

class SessionNormalCloseManager : public vscp::BaseSessionManager{
public:
  SessionNormalCloseManager() :task_count_(0){
  }
  boost::signals2::signal<void(SessionNormalCloseManager* sdm)> SignalTaskComplete;
  virtual vscp::BaseSession::SessionPtr CreateClientSession(
    boost::asio::io_service &io_service){
    // ping timeout with 10 s
    task_count_++;
    LOG(INFO) << "Create a task " << task_count_;
    SessionNormalCloseTest* snct = new SessionNormalCloseTest(io_service, 10);
    snct->SignalTaskComplete.connect(
      boost::bind(&SessionNormalCloseManager::OnTaskComplete, this, _1));
    return vscp::BaseSession::SessionPtr(snct);
  }
  // return None, only service with server session
  virtual vscp::BaseSession::SessionPtr CreateServerSession(
    boost::asio::io_service &io_service){
    return vscp::BaseSession::SessionPtr(NULL);
  };

  void OnTaskComplete(vscp::BaseSession::SessionWptr session){
    task_count_--;
    LOG(INFO) << "Remove task " << task_count_;
    if (task_count_ <= 0){
      SignalTaskComplete(this);
    }
  }
private:
  int task_count_;
};
//------------------------------------------------------------------------------

class DownloadRandomSession : public ClientSession{
public:
  DownloadRandomSession(boost::asio::io_service &io_service,
    int ping_timeout, int download_size, int packet_size)
    :ClientSession(io_service, ping_timeout),
    download_data_size_(download_size),
    current_download_(0),
    packet_size_(packet_size){
  }
  virtual void OnPacketRead(BaseSession::SessionWptr session, const char* buffer,
    int buffer_size, const boost::system::error_code& err){
    if (download_data_size_ > current_download_++){
      AsyncWritePacket(buffer_.get(), packet_size_);
    }
    else{
      CloseSession();
      SignalTaskComplete(session);
    }
  }
private:
  int download_data_size_;
  int current_download_;
  int packet_size_;
};

//------------------------------------------------------------------------------
class DownloadRandomSessionManager : public vscp::BaseSessionManager{
public:
  DownloadRandomSessionManager() :task_count_(0){
  }
  boost::signals2::signal<void(DownloadRandomSessionManager* sdm)> SignalTaskComplete;
  virtual vscp::BaseSession::SessionPtr CreateClientSession(
    boost::asio::io_service &io_service){
    task_count_++;
    LOG(INFO) << "Create a task " << task_count_;
    DownloadRandomSession* ds = new DownloadRandomSession(io_service, 
      3, 128, MAX_PACKET_SIZE/4);
    ds->SignalTaskComplete.connect(
      boost::bind(&DownloadRandomSessionManager::OnTaskComplete, this, _1));
    // ping timeout with 10 s
    return vscp::BaseSession::SessionPtr(ds);
  }
  // return None, only service with server session
  virtual vscp::BaseSession::SessionPtr CreateServerSession(
    boost::asio::io_service &io_service){
    return vscp::BaseSession::SessionPtr(NULL);
  };
  void OnTaskComplete(vscp::BaseSession::SessionWptr session){
    task_count_--;
    LOG(INFO) << "Remove task " << task_count_;
    if (task_count_ <= 0){
      SignalTaskComplete(this);
    }
  }
private:
  int task_count_;
};


class TestSession :public vscp::BaseSession{
public:
  TestSession(boost::asio::io_service &io_service,
    int ping_timeout)
    :BaseSession(io_service, ping_timeout),
    buffer_(new char[MAX_PACKET_SIZE]){

  }

  boost::signals2::signal < void(BaseSession::SessionWptr session) >
    SignalTaskComplete;

  virtual void OnPrepareStart(const boost::system::error_code& err){

  }

  virtual void OnSocketError(BaseSession::SessionWptr session,
    const boost::system::error_code& err){

  }
  virtual void OnPacketRead(BaseSession::SessionWptr session, const char* buffer,
    int buffer_size, const boost::system::error_code& err){

  }
  virtual void OnPacketWriteComplete(BaseSession::SessionWptr session,
    const boost::system::error_code& err){

  }
protected:
  const int MAX_PACKET_SIZE = 1024 * 512;
  const int MIN_PACKET_SIZE = 1;
  boost::scoped_array<char> buffer_;
};

#endif // TEST_PACKET_CLIENT_SESSION_H_
