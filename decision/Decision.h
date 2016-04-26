/*
 * Decision.h
 *
 *  Created on: Mar 8, 2016
 *      Author: steven
 */

#ifndef DECISION_H_
#define DECISION_H_

#include <cv.h>
#include <highgui.h>

void Decision(cv::Mat mask_line, cv::Mat mask_hot, cv::Mat image_thermal, cv::Mat& out, double stddev_mult);

#endif /* DECISION_H_ */
