/*
 * main.cpp
 *
 *  Created on: Mar 7, 2016
 *      Author: steven
 */

#include <cv.h>
#include <highgui.h>
#include <libconfig.h++>

#include <iostream>
#include <math.h>

#include "power-line-detection/PowerLineDetection.h"
#include "hot-spot-detection/hotSpotDetectionAlgorithm.h"
#include "decision/Decision.h"

#include "power-line-detection/LineSegment.h"

struct MyPoint
{
	int x;
	int y;
} pt1, pt2;

double angle = NAN;
double angle_thresh = M_PI/8;
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

			cv::Point pt1_cv(pt1.x, pt1.y);
			cv::Point pt2_cv(pt2.x, pt2.y);
			LineSegment line(pt1_cv, pt2_cv);

			double rho, theta;
			line.GetHesseNormalForm(rho, theta);
			angle = theta;

			std::cout << "Angle: " << angle << std::endl;
		}
	}
}

void MyEqualizeHist(cv::Mat& src, cv::Mat& dst)
{
	cv::Mat temp;
	cv::cvtColor(src, temp, CV_BGR2GRAY);
	cv::equalizeHist(temp, temp);
	cv::cvtColor(temp, dst, CV_GRAY2BGR);
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
	
	libconfig::Config cfg;
	try
	{
		cfg.readFile("main.cfg");
	}
	catch(const libconfig::FileIOException &fioex)
	{
		std::cerr << "I/O error while reading file." << std::endl;
		return(EXIT_FAILURE);
	}
	catch(const libconfig::ParseException &pex)
	{
		std::cerr << "Parse error at " << pex.getFile() << 
		":" << pex.getLine() << " - " << pex.getError() << std::endl;
		return(EXIT_FAILURE);
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

	MyEqualizeHist(line_in, line_in);
	cv::imshow("eql", line_in);

	double om = cfg.lookup("PowerLineDetection.om");
	double p1_b = cfg.lookup("PowerLineDetection.p1_b");
	double p1_m = cfg.lookup("PowerLineDetection.p1_m");
	double p2 = cfg.lookup("PowerLineDetection.p2");
	double tm = cfg.lookup("PowerLineDetection.tm");

	PowerLineDetection(line_in, line_out, p1_m, p1_b, p2, om, tm, angle, angle_thresh);

	std::cout << "Finding hot spots..." << std::endl;

	cv::Mat hot_in = image_src.clone();
	{
		cv::Mat temp;
		cv::cvtColor(hot_in, temp, CV_BGR2GRAY);
		hot_in = temp.clone();
	}
	cv::Mat hot_out = cv::Mat::zeros(image_src.size(), CV_8U);
	vector<vector<cv::Point> > contours;

	int win_horz = cfg.lookup("hotSpotDetection.win_horz");
	int win_vert = cfg.lookup("hotSpotDetection.win_vert");
	double pix_thrsh_lowr = cfg.lookup("hotSpotDetection.pix_thrsh_lowr");
	double pix_thrsh_uppr = cfg.lookup("hotSpotDetection.pix_thrsh_uppr");
	double thresh_percent = cfg.lookup("hotSpotDetection.thresh_percent");
	int blur_ksize = cfg.lookup("hotSpotDetection.blur_ksize");

	hotSpotDetectionAlgorithm(hot_in, hot_out, win_horz, win_vert, contours, thresh_percent, pix_thrsh_lowr, pix_thrsh_uppr, blur_ksize); //blur filter - getContourImg()

	{
		cv::Mat temp;
		cv::cvtColor(hot_out, temp, CV_GRAY2BGR);
		hot_out = temp.clone();
	}

	std::cout << "Finding hot spots on power lines..." << std::endl;

	cv::Mat conjunction;
	cv::bitwise_and(line_out, hot_out, conjunction);
	for(int i = 0; i < 3; i++)
			cv::dilate(conjunction, conjunction,cv::Mat());

	std::cout << "Displaying power lines and hot spots..." << std::endl;
	std::cout << "Please press 'q' to continue." << std::endl;

	cv::imshow("src", image_src);
	cv::imshow("line", line_out);
	cv::imshow("hot", hot_out);
	cv::imshow("conj", conjunction);
	while(char(cv::waitKey(1)) != 'q'){}
	cv::destroyAllWindows();

	std::cout << "Analyzing hot spots and power lines..." << std::endl;

	cv::Mat decision_out;

	double stddev_mult = cfg.lookup("Decision.stddev_mult");

	Decision(line_out, hot_out, image_src, decision_out, stddev_mult);

	std::cout << "Displaying analysis..." << std::endl;
	std::cout << "Please press 'q' to continue." << std::endl;

	cv::imshow("src", image_src);
	cv::imshow("decision", decision_out);
	while(char(cv::waitKey(1)) != 'q'){}
	cv::destroyAllWindows();

	std::cout << "Quitting..." << std::endl;

	return 0;
}
