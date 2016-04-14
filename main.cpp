/*
 * main.cpp
 *
 *  Created on: Mar 7, 2016
 *      Author: steven
 */

#include <cv.h>
#include <highgui.h>

#include <iostream>
#include <math.h>

#include "power-line-detection/PowerLineDetection.h"
#include "hot-spot-detection/hotSpotDetectionAlgorithm.h"
#include "decision/Decision.h"

struct MyPoint
{
	int x;
	int y;
} pt1, pt2;

double angle = NAN;
int counter = 0;

void onClick(int event, int x, int y, int flags, void* userdata)
{
	if(event == EVENT_LBUTTONDOWN)
	{
		cout << "Left button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;

		if(counter == 0)
		{
			pt1.x = x;
			pt1.y = y;
			counter++;
		}
		else if(counter == 1)
		{
			pt2.x = x;
			pt2.y = y;
			counter = 0;

			angle = atan2(double(pt1.y - pt2.y), double(pt1.x - pt2.x)) + M_PI + M_PI/2;
			while(angle < 0 || M_PI < angle)
			{
				if(angle < 0)
					angle += M_PI;
				else
					angle -= M_PI;
			}

			std::cout << "Angle: " << angle << std::endl;
		}
	}
}


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

	std::cout << "Capturing angle of line..." << std::endl;
	std::cout << "Please click twice to set angle then press any button to continue." << std::endl;

	cv::imshow("dir", image_src);
	cv::setMouseCallback("dir", onClick);
	cv::waitKey(0);
	cv::destroyAllWindows();

	std::cout << "Finding power lines..." << std::endl;

	cv::Mat line_in = image_src.clone();
	cv::Mat line_out = image_src.clone();

	double om = 10.0;
	double p1_b = 10;
	double p1_m = 0.5;
	double p2 = 200;
	double tm = 10.0;

	PowerLineDetection(line_in, line_out, p1_m, p1_b, p2, om, tm);

	std::cout << "Finding hot spots..." << std::endl;

	cv::Mat hot_inout = image_src.clone();
	{
		cv::Mat temp;
		cv::cvtColor(hot_inout, temp, CV_BGR2GRAY);
		hot_inout = temp.clone();
	}

	vector<vector<Point> > contours;
	double pixel_thresh = 20.0;

	hotSpotDetectionAlgorithm(hot_inout, contours, pixel_thresh);

	std::cout << "Finding hot spots on power lines..." << std::endl;

	cv::Mat conjunction;
	cv::bitwise_and(line_out, hot_inout, conjunction);
	for(int i = 0; i < 3; i++)
			cv::dilate(conjunction, conjunction,cv::Mat());

	std::cout << "Displaying power lines and hot spots..." << std::endl;
	std::cout << "Please press 'q' to continue." << std::endl;

	cv::imshow("src", image_src);
	cv::imshow("line", line_out);
	cv::imshow("hot", hot_inout);
	cv::imshow("conj", conjunction);
	while(char(cv::waitKey(1)) != 'q'){}
	cv::destroyAllWindows();

	std::cout << "Analyzing hot spots and power lines..." << std::endl;

	cv::Mat decision_out;
	Decision(line_out, hot_inout, image_src, decision_out);

	std::cout << "Displaying analysis..." << std::endl;
	std::cout << "Please press 'q' to continue." << std::endl;

	cv::imshow("src", image_src);
	cv::imshow("decision", decision_out);
	while(char(cv::waitKey(1)) != 'q'){}
	cv::destroyAllWindows();

	std::cout << "Quitting..." << std::endl;

	return 0;
}
