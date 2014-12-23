
#ifndef TMQ_BASE_SOCKETSERVER_H_
#define TMQ_BASE_SOCKETSERVER_H_

#include "tmq/base/messagequeue.h"

namespace tmq {

class MessageQueue;

// Provides the ability to wait for activity on a set of sockets.  The Thread
// class provides a nice wrapper on a socket server.
//
// The server is also a socket factory.  The sockets it creates will be
// notified of asynchronous I/O from this server's Wait method.
class SocketServer {
 public:
  // When the socket server is installed into a Thread, this function is
  // called to allow the socket server to use the thread's message queue for
  // any messaging that it might need to perform.
  virtual void SetMessageQueue(MessageQueue* queue) {}

  // Sleeps until:
  //  1) cms milliseconds have elapsed (unless cms == kForever)
  //  2) WakeUp() is called
  // While sleeping, I/O is performed if process_io is true.
  virtual bool Wait(int cms, bool process_io) = 0;

  // Causes the current wait (if one is in progress) to wake up.
  virtual void WakeUp() = 0;
};

}  // namespace tmq

#endif  // TMQ_BASE_SOCKETSERVER_H_
