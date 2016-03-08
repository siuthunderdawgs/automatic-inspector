/*
 * WindowedHoughTransform.cpp
 *
 *  Created on: Feb 6, 2016
 *      Author: steven
 */

#include <vector>
#include <utility>
#include <cmath>

#include "cv.h"
#include "highgui.h"

#include "LinePainter.h"
#include "Windows.h"
#include "WindowedHoughTransform.h"

extern cv::Mat image_src;

void WindowedHoughLine(cv::Mat input, cv::Mat& output, int horz, int vert, double rho, double theta, int thresh, double m, double b)
{
	cv::Mat mask = cv::Mat::zeros(input.size(), CV_8UC3);

	std::vector< std::vector <cv::Mat> > windows_input, windows_mask, windows_src;
	windows_input = CreateWindows(input, horz, vert);
	windows_mask = CreateWindows(mask, horz, vert);
	windows_src = CreateWindows(image_src, horz, vert);

	for(unsigned int i = 0; i < windows_input.size(); i++)
	{
		for(unsigned int j = 0; j < windows_input[i].size(); j++)
		{
			cv::Mat win_in = windows_input[i][j];
			cv::Mat win_mask = windows_mask[i][j];
			cv::Mat win_src = windows_src[i][j];

			double clutter = ComputeWindowClutter(win_in, 4, 4);

			double cthresh;
			if(thresh == 0)
				cthresh = m*clutter + b;
			else
				cthresh = (float)thresh;

			std::vector<cv::Vec2f> lines, lines_temp;
			cv::HoughLines(win_in, lines_temp, rho, theta, cthresh, 0, 0);

			for(std::vector<cv::Vec2f>::iterator it = lines_temp.begin(); it != lines_temp.end(); ++it)
			{
				float rho = (*it)[0];
				float theta = (*it)[1];
				float drho[] = {-5, 0, 5};

				LineSegment line[3];

				for(int k = 0; k < 3; k++)
				{
					line[k] = LineSegment(rho + drho[k], theta);
				}

				float vmean[3], vstddev[3];

				for(int k = 0; k < 3; k++)
				{
					cv::Mat mmean, mstddev;
					cv::Mat mask = cv::Mat::zeros(win_in.size(), CV_8U);

					LinePainter painter;
					painter.SetImage(&mask);
					painter.SetThickness(2);
					painter.SetColor(cv::Scalar(255,255,255));
					painter.AddLines(line[k]);
					painter.DrawLines();
					painter.RstLines();

					meanStdDev(win_src, mmean, mstddev, mask);

					vmean[k] = mmean.at<double>(0,0);
					vstddev[k] = mstddev.at<double>(0,0);
				}

				float fthresh = vstddev[0] >= vstddev[2] ? vstddev[0] : vstddev[2];
				if(abs(vmean[0] - vmean[2]) <= 1*fthresh)
					lines.push_back(*it);
			}

			LinePainter painter;
			painter.SetImage(&win_mask);
			painter.SetLines(lines);
			painter.DrawLines();
			painter.RstLines();

			//cv::namedWindow("Sample");
			//cv::imshow("Sample", win_in);
			//cv::waitKey();
		}
	}

	output = mask.clone();
}
