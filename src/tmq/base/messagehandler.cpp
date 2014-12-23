
#include "tmq/base/messagehandler.h"
#include "tmq/base/messagequeue.h"

namespace tmq {

MessageHandler::~MessageHandler() {
  MessageQueueManager::Clear(this);
}

} // namespace tmq
