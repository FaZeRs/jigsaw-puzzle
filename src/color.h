#ifndef COLOR_H
#define COLOR_H

#include <opencv2/opencv.hpp>

#include <uc/dimensions.h>

cv::Scalar makeColor(dimensions::Time time);

#endif // COLOR_H