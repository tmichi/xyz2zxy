//
// Created by Takashi Michikawa on 2022/05/16.
//

#include <opencv2/opencv.hpp>
#include <filesystem>
#include <fmt/core.h>
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
                                        image.at<cv::Vec3w>(y, x) = cv::Vec3w(z * 256, y * 256, x* 256);
                                }
                        }
                        std::vector<int> params = {cv::IMWRITE_TIFF_COMPRESSION, 1};
                        if (!cv::imwrite(fmt::format("{0}/image-{1:05d}.tif", dir.string(), z), image, params)) {
                                throw std::runtime_error("The image cannot be created");
                        }
                }
        } catch (std::runtime_error& e) {
                std::cerr<<e.what()<<std::endl;
        }
        return 0;
}