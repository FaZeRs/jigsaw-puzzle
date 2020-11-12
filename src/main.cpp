#include <opencv2/opencv.hpp>

#include "logger.h"
#include "math_common.h"
#include "math_geometric.h"
#include "command_line.h"
#include "dimensions.h"
#include "timer.h"

using namespace dimensions;
using namespace dimensions::literals;

int main(int argc, char** argv) {
  CMDLine cmd;
  cmd.addParameter("--window-size", "Size of the output window", "600 800", CMDLineType::Double, 2);

  if (cmd.parse(argc, argv) == false) {
    cmd.help();
    return 0;
  }

  cv::Mat output(cmd.getDoubles("--window-size")[0], cmd.getDoubles("--window-size")[1], CV_8UC3);
  Timer timer;
  while (true) {
    output = cv::Scalar((cos(2.0_pi_rad * 0.3_Hz * timer.elapsedS()) + 1) * 127.0,
                        (sin(2.0_pi_rad * 0.1_Hz * timer.elapsedS()) + 1) * 127.0,
                        (sin(2.0_pi_rad * 0.5_Hz * timer.elapsedS() + 0.5_pi_rad) + 1) * 127.0);
    cv::imshow("Output", output);

    int key = cv::waitKey(1);
    if (key != -1) {
      break;
    }
  }

  LOG_INFO("Exiting cleanly!");

  return 0;
}