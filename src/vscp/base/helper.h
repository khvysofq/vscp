// Vision Zenith System Communication Protocol (Project)
#ifndef VSCP_BASE_HELPER_H_
#define VSCP_BASE_HELPER_H_

#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>

namespace vscp {

  const std::string CHAR_SET = "1234567890abcdefghijklmnopqrstuvwxyz";
  const std::string GetRandomVscpid();

}; // namespace

#endif