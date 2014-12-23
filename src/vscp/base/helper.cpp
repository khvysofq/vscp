// Vision Zenith System Communication Protocol (Project)

#include "vscp/base/helper.h"
#include <sstream>
namespace vscp {

  static const int DEFAULT_RANDOM_LENGTH = 4;

  const std::string GetRandomVscpid(){
    static boost::int64_t start_id = 0;
    start_id++;

    std::stringstream ost;
    boost::random::random_device rng;
    boost::random::uniform_int_distribution<> index_dist(0, CHAR_SET.size() - 1);
    for (int i = 0; i < DEFAULT_RANDOM_LENGTH; ++i) {
      ost << CHAR_SET[index_dist(rng)];
    }
    ost << start_id;

    return ost.str();
  }
}; // namespace vscp