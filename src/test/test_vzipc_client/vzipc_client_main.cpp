#include <iostream>
#include "vzipc/base/vzipcserver.h"
#include "glog/logging.h"

class IpcClient{
public:
  IpcClient(vzipc::VZSocketServer *socket_server)
    :socket_server_(socket_server),
    ipc_socket_(NULL){

  }

  void Start(){
    ipc_socket_ = socket_server_->CreateVZClientSocket();
    if (vzipc::VZ_SOCKET_ERROR == ipc_socket_->Connect(5299)){
      CHECK(0);
    }

    ipc_socket_->SignalPacketRecived.connect(
      boost::bind(&IpcClient::OnPacketRecived, this, _1, _2, _3));
    ipc_socket_->SignalStateChange.connect(
      boost::bind(&IpcClient::OnStateChange, this, _1, _2));
    ipc_socket_->SignalSocketError.connect(
      boost::bind(&IpcClient::OnSocketError, this, _1, _2));

    ipc_socket_->AsyncWrite("Hello", 5);
  }

  void OnPacketRecived(vzipc::VzIpcSocket *socket, const char* data, int len){
    LOG(INFO) << ".";
    CHECK(socket == ipc_socket_);
    ipc_socket_->AsyncWrite(data, len);
  }

  void OnStateChange(vzipc::VzIpcSocket *socket, vzipc::ConnState state){
    LOG(INFO) << __FUNCTION__;

  }

  void OnSocketError(vzipc::VzIpcSocket *socket, int err){
    LOG(INFO) << __FUNCTION__;
  }
private:
  vzipc::VZSocketServer *socket_server_;
  vzipc::VzIpcSocket *ipc_socket_;
};


int main(int argc, char *argv[]){
  google::InitGoogleLogging(argv[0]);
  FLAGS_stderrthreshold = 0;
  FLAGS_colorlogtostderr = true;

  vzipc::VZSocketServer *socket_server = vzipc::InitVZNetwork();
  IpcClient ipc_client(socket_server);
  ipc_client.Start();
  while (1){
    socket_server->Wait(1000);
  }

  return 0;
}