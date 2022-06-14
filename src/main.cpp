#include <opencv2/opencv.hpp>

#include <uc/logger.h>
#include <uc/command_line.h>
#include <uc/timer.h>

#include "color.h"

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
    output = makeColor(timer.elapsedS());
    cv::imshow("Output", output);

    int key = cv::waitKey(1);
    if (key != -1) {
      break;
    }
  }

  LOG_INFO("Exiting cleanly!");

  return 0;
}