#ifndef VSCP_NET_WRITEHANDLER_H_
#define VSCP_NET_WRITEHANDLER_H_

#include "vscp/base/basicdefines.h"

namespace vscp{

  class WriteHandler : public boost::noncopyable,
    public boost::enable_shared_from_this < WriteHandler > {
  public:
    WriteHandler(){};
    virtual ~WriteHandler(){};
  }; // class WriteHandler

  typedef boost::shared_ptr<WriteHandler> WriteHandlerPtr;

}; // namespace vscp
#endif // VSCP_NET_WRITEHANDLER_H_