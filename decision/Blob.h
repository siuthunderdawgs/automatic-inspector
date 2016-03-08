/*
 * Blob.h
 *
 *  Created on: Mar 8, 2016
 *      Author: steven
 */

// Original Name: blob.cpp
// Original Author: Nghia Ho (nghiaho12)
// Source: http://nghiaho.com/?p=1102

#ifndef BLOB_H_
#define BLOB_H_

void FindBlobs(const cv::Mat &binary, std::vector < std::vector<cv::Point2i> > &blobs);

#endif /* BLOB_H_ */
