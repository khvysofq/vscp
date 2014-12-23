#ifndef EVENT_BASE_WAIT_SERVER_H_
#define EVENT_BASE_WAIT_SERVER_H_

#include <Windows.h>
#include "tmq\base\socketserver.h"

namespace tmq{
#ifdef WIN32
  class EventWaitServer : public SocketServer{
  public:
    EventWaitServer();
    ~EventWaitServer();
    // Sleeps until:
    //  1) cms milliseconds have elapsed (unless cms == kForever)
    //  2) WakeUp() is called
    // While sleeping, I/O is performed if process_io is true.
    virtual bool Wait(int cms, bool process_io);

    // Causes the current wait (if one is in progress) to wake up.
    virtual void WakeUp();
  private:

    HANDLE signal_event_[1];
  };
#else
  class EventWaitServer : public SocketServer{
  public:
    EventWaitServer();
    ~EventWaitServer();
    // Sleeps until:
    //  1) cms milliseconds have elapsed (unless cms == kForever)
    //  2) WakeUp() is called
    // While sleeping, I/O is performed if process_io is true.
    virtual bool Wait(int cms, bool process_io);

    // Causes the current wait (if one is in progress) to wake up.
    virtual void WakeUp();
  private:

    HANDLE signal_event_[1];
};
#endif
} // namespace tmq

#endif // EVENT_BASE_WAIT_SERVER_H_