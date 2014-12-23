// Vision Zenith System Communication Protocol (Project)
#ifndef VSCP_BASE_BASIC_DEFINES_H_
#define VSCP_BASE_BASIC_DEFINES_H_

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <boost/signals2.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>

#include <iostream>
#include <string>
#include <iomanip>

#include "glog/logging.h"

//TODO (Guangleihe) use boost basic type instead of this
#include "vscp/base/basictypes.h"

namespace vscp {

  typedef boost::shared_ptr<boost::asio::ip::tcp::socket> socket_ptr;
  typedef boost::shared_ptr<boost::asio::io_service> io_service_ptr;

  static const std::string DEFAULT_SERVER_ADDRESS = "";
  static const int DEFAULT_SERVER_PORT = 5298;
  //static const int DEFAULT_THREAD_COUNT = 2;
  static const int MAX_PACKET_SIZE = 1024 * 512; // 512 KB
  static const int FOREVER = -1;
}; // namespace vscp

#endif // VSCP_BASE_BASIC_DEFINES_H_