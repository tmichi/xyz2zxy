//
// Created by Takashi Michikawa on 2022/05/16.
//
#include <iostream>
#include <iomanip>
#include <filesystem>
#include <opencv2/highgui.hpp>
#include <opencv2/core.hpp>
int main () {
        try {
                std::filesystem::path dir("sample16");
                std::filesystem::create_directory(dir);
                if (!std::filesystem::exists(dir)) {
                        throw std::runtime_error(dir.string() + " cannot be created");
                }
                cv::Mat image (cv::Size(256, 256), CV_16UC3);
                for (int z = 0 ; z < 256 ; ++z) {
                        for (int y = 0 ; y < 256 ; ++y) {
                                for (int x = 0 ; x < 256; ++x) {
                                        image.at<cv::Vec3w>(y, x) = cv::Vec3w(uint16_t(z * 256), uint16_t(y * 256), uint16_t(x* 256));
                                }
                        }
                        std::vector<int> params = {cv::IMWRITE_TIFF_COMPRESSION, 1};
                        std::stringstream ss;
                        ss<<dir.string()<<"/image-"<<std::setw(5)<<std::setfill('0')<<z<<".tif";
                        //if (!cv::imwrite(fmt::format("{0}/image-{1:05d}.tif", dir.string(), z), image, params)) {
                        if (!cv::imwrite(ss.str(), image, params)) {
                                throw std::runtime_error("The image cannot be created");
                        }
                }
        } catch (std::runtime_error& e) {
                std::cerr<<e.what()<<std::endl;
        } catch (...) {
                std::cerr<<"Unknown error."<<std::endl;
        }
        return 0;
}