
#ifndef TMQ_BASE_MESSAGEHANDLER_H_
#define TMQ_BASE_MESSAGEHANDLER_H_

#include "tmq/base/boostincludes.h"

namespace tmq {

struct Message;

// Messages get dispatched to a MessageHandler

class MessageHandler : public boost::noncopyable {
 public:
  virtual void OnMessage(Message* msg) = 0;

 protected:
  MessageHandler() {}
  virtual ~MessageHandler();
};

} // namespace tmq

#endif // TMQ_BASE_MESSAGEHANDLER_H_
