/*
 * main.cpp
 *
 *  Created on: Mar 7, 2016
 *      Author: steven
 */

#include <cv.h>
#include <highgui.h>

#include <iostream>

#include "power-line-detection/PowerLineDetection.h"
#include "hot-spot-detection/hotSpotDetectionAlgorithm.h"

cv::Mat image_src;

int main(int argc, char** argv)
{
	const char* filename;

	if(argc >= 2)
		filename = argv[1];
	else
		filename = "image.jpg";

	image_src = cv::imread(filename);

	if(image_src.empty())
	{
	 std::cout << "ERROR: Cannot open " << filename << std::endl;
	 return -1;
	}

	cv::Mat line_in = image_src.clone();
	cv::Mat line_out = image_src.clone();

	PowerLineDetection(line_in, line_out, 0.5, 10, 200);


	cv::Mat hot_inout = image_src.clone();
	{
		cv::Mat temp;
		cv::cvtColor(hot_inout, temp, CV_BGR2GRAY);
		hot_inout = temp.clone();
	}

	vector<vector<Point> > contours;
	double pixel_thresh = 20.0;

	hotSpotDetectionAlgorithm(hot_inout, contours, pixel_thresh);


	cv::Mat conjunction;
	cv::bitwise_and(line_out, hot_inout, conjunction);
	for(int i = 0; i < 3; i++)
			cv::dilate(conjunction, conjunction,cv::Mat());

	cv::imshow("src", image_src);
	cv::imshow("line", line_out);
	cv::imshow("hot", hot_inout);
	cv::imshow("conj", conjunction);

	while(char(cv::waitKey(1)) != 'q'){}
	return 0;
}
