#include "Label.h"

#include <glog/logging.h>
#include <glog/raw_logging.h>

namespace romp {

Label::Label() { 
  RAW_LOG(INFO, "%s\n", "Label() constructor");
}

Label::~Label() {
  RAW_LOG(INFO, "%s\n", "Label() destructor");
}


}
