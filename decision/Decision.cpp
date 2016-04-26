/*
 * Decision.cpp
 *
 *  Created on: Mar 8, 2016
 *      Author: steven
 */

#include <cv.h>
#include <highgui.h>

#include <iostream>
#include <unistd.h>

#include "Blob.h"

void Decision(cv::Mat mask_line, cv::Mat mask_hot, cv::Mat image_thermal, cv::Mat& out, double stddev_mult)
{
	cv::Mat temp_gray; cv::cvtColor(image_thermal, temp_gray, CV_BGR2GRAY);
	image_thermal = temp_gray.clone();

	cv::Mat output_local = cv::Mat::zeros(image_thermal.size(), CV_8U);

	cv::Mat gray_line;
	cv::Mat gray_hot;
	cv::cvtColor(mask_line, gray_line, CV_BGR2GRAY);
	cv::cvtColor(mask_hot, gray_hot, CV_BGR2GRAY);

	cv::Mat binary_line;
	cv::Mat binary_hot;
	cv::threshold(gray_line, binary_line, 0.0, 1.0, cv::THRESH_BINARY);
	cv::threshold(gray_hot, binary_hot, 0.0, 1.0, cv::THRESH_BINARY);

	std::vector< std::vector<cv::Point2i> > blobs_mask_line, blobs_mask_hot;

	std::cout << "Finding blobs..." << std::endl;
	FindBlobs(binary_line, blobs_mask_line);
	FindBlobs(binary_hot, blobs_mask_hot);
	std::cout << "Number of line blobs: " << blobs_mask_line.size() << std::endl;
	std::cout << "Number of hot blobs: " << blobs_mask_hot.size() << std::endl;

	std::vector<cv::Mat> map_line, map_hot;

	std::vector< std::vector<cv::Point2i> >::iterator it1;
	for(it1 = blobs_mask_line.begin(); it1 != blobs_mask_line.end(); ++it1)
	{
		cv::Mat temp = cv::Mat::zeros(image_thermal.size(), CV_8U);
		unsigned char k = 255;

		std::vector<cv::Point2i>::iterator it2;
		for(it2 = (*it1).begin(); it2 != (*it1).end(); ++it2)
		{
			int x = (*it2).x;
			int y = (*it2).y;

			temp.at<uchar>(y,x) = k;
		}

		//cv::imshow("temp", temp);
		//cv::waitKey(0);

		map_line.push_back(temp);
	}

	for(it1 = blobs_mask_hot.begin(); it1 != blobs_mask_hot.end(); ++it1)
	{
		cv::Mat temp = cv::Mat::zeros(image_thermal.size(), CV_8U);
		unsigned char k = 255;

		std::vector<cv::Point2i>::iterator it2;
		for(it2 = (*it1).begin(); it2 != (*it1).end(); ++it2)
		{
			int x = (*it2).x;
			int y = (*it2).y;

			temp.at<uchar>(y,x) = k;
		}

		//cv::imshow("temp", temp);
		//cv::waitKey(0);

		map_hot.push_back(temp);
	}

	for(std::vector<cv::Mat>::iterator ita = map_line.begin(); ita != map_line.end(); ++ita)
	{
		cv::Mat line_mean;
		cv::Mat line_stddev;

		cv::meanStdDev(image_thermal, line_mean, line_stddev, *ita);

		int line_meanval = int(line_mean.at<double>(0,0));
		int line_stddevval = int(line_stddev.at<double>(0,0));

		for(std::vector<cv::Mat>::iterator itb = map_hot.begin(); itb != map_hot.end(); ++itb)
		{
			cv::Mat conjunction;
			cv::bitwise_and(*ita, *itb, conjunction);

			int count = cv::countNonZero(conjunction);

			if(count > 0)
			{
				cv::Mat hot_mean;
				cv::Mat hot_stddev;

				//Question: Should this be *itb or conjunction?
				cv::meanStdDev(image_thermal, hot_mean, hot_stddev, *itb);

				int hot_meanval = int(hot_mean.at<double>(0,0));
				int hot_stddevval = int(hot_stddev.at<double>(0,0));

				if(hot_meanval > line_meanval + stddev_mult*line_stddevval)
				{
					std::cout << "Found major hot spot!\n";

					std::cout << "Average line intensity: " << line_meanval << "\n";
					std::cout << "Std.dev. line intensity: " << line_stddevval << "\n";
					std::cout << "Average spot intensity: " << hot_meanval << "\n";
					std::cout << "Std. dev. spot intensity: " << hot_stddevval << std::endl;

					cv::bitwise_or(output_local, *itb, output_local);
				}
				else
				{
					std::cout << "Found minor hot spot!\n";
				}
			}
		}
	}

	out = output_local;
}
