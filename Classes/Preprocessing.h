//
// Created by CUONG on 12/3/2018.
//

#ifndef PROJ_ANDROID_STUDIO_PREPROCESSING_H
#define PROJ_ANDROID_STUDIO_PREPROCESSING_H

#include <opencv2/opencv.hpp>
#include <vector>
#include "Ultils.h"

using namespace std;

cv::Rect extract_board(cv::Mat &image);
void extract_blocks(cv::Mat &board, vector<cv::Rect> &_blocks);
void estimate_block_position(vector<cv::Rect> &blocks, vector<vector<cv::Point>> &block_position,
                             vector<int> &_red_block_indx, cv::Point exit_pos = cv::Point(5, 2));


void parseScreenShot(const char *path, vector<vector<cv::Point>> &block_pos);

void processScreenShot(const char *path, vector<vector<cv::Point>> &block_position);

#endif //PROJ_ANDROID_STUDIO_PREPROCESSING_H
