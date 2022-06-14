#include "color.h"

using namespace dimensions;
using namespace dimensions::literals;

cv::Scalar makeColor(dimensions::Time time) {
  return {(cos(2.0_pi_rad * 0.3_Hz * time) + 1) * 127.0, (sin(2.0_pi_rad * 0.1_Hz * time) + 1) * 127.0,
          (sin(2.0_pi_rad * 0.5_Hz * time + 0.5_pi_rad) + 1) * 127.0};
}