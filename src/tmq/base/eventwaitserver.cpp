
#include "tmq\base\eventwaitserver.h"
#include "glog\logging.h"
namespace tmq{

  EventWaitServer::EventWaitServer(){

    signal_event_[0] = CreateEvent(
      NULL,   // default security attributes
      FALSE,  // auto-reset event object
      FALSE,  // initial state is nonsignaled
      NULL);  // unnamed object

    if (signal_event_[0] == NULL){
      LOG(ERROR) << "Create signal_event error";
    }
  }

  EventWaitServer::~EventWaitServer(){

    CloseHandle(signal_event_[0]);
  }

  bool EventWaitServer::Wait(int cms, bool process_io){

    DWORD dwEvent = WaitForMultipleObjects(
      1,                 // number of objects in array
      signal_event_,     // array of objects
      FALSE,             // wait for any object
      cms);              // cms wait

    // The return value indicates which event is signaled
    switch (dwEvent)
    {
      // signal_event_ was signaled
    case WAIT_OBJECT_0 + 0:
      // TODO: Perform tasks required by this event
      //DLOG(INFO) << "signal_event_ was signaled";
      break;

    case WAIT_TIMEOUT:
      DLOG(INFO) << "Wait timed out.";
      break;

      // Return value is invalid.
    default:
      LOG(ERROR) << "Wait error: " << GetLastError();
      return false;
    }
    return true;
  }

  void EventWaitServer::WakeUp(){

    // Set one event to the signaled state
    if (!SetEvent(signal_event_[0]))
    {
      LOG(ERROR) << "SetEvent failed " << GetLastError();
    }
  }
} // namespace tmq