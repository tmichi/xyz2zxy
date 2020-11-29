//
// Created by Takashi Michikawa on 2020/09/24.
//

#ifndef REPOSITORY_GET_SIZE_TYPE_HPP
#define REPOSITORY_GET_SIZE_TYPE_HPP
#include <opencv2/opencv.hpp>
#include <tuple>
std::tuple<int, int, int> getSizeType(const std::string &f) {
        cv::Mat img = cv::imread(f, cv::IMREAD_ANYDEPTH | cv::IMREAD_COLOR);
        return std::make_tuple(img.cols, img.rows, img.type());
}
#endif //REPOSITORY_GET_SIZE_TYPE_HPP
