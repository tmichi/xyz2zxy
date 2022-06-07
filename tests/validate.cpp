// Created by Takashi Michikawa.
/**
 * MIT License
 * Copyright (c) 2021 RIKEN
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/
#include <filesystem>
#include <iostream>
#include <iomanip>
#include <filesystem>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

int main (int argc, char** argv) {
        try {
                if (argc < 2) {
                        throw std::runtime_error("Runtime error. Invalid argument "+std::string(argv[1]));
                }
                std::vector<std::filesystem::path> paths;
                std::copy(std::filesystem::directory_iterator(argv[1]), std::filesystem::directory_iterator(), std::back_inserter(paths));
                std::sort(paths.begin(), paths.end());
                for (int y = 0 ; y < 256 ; ++y) {
                        if ( cv::Mat image = cv::imread(paths[y], cv::IMREAD_UNCHANGED) ; image.empty() ) {
                                throw std::runtime_error(paths[y].string() + " was empty.");
                        } else if (image.size().width != 256 || image.size().height != 256) {
                                throw std::runtime_error(" Size different.");
                        } else {
                                for (int x = 0 ; x < 256; ++x) {
                                        for (int z = 0 ; z < 256 ; ++z) {
                                                int px, py, pz;
                                                if (image.depth() == CV_8U) {
                                                        const auto& p = image.at<cv::Vec3b>(x, z) ;
                                                        px = p[2];
                                                        py = p[1];
                                                        pz = p[0];
                                                } else if (image.depth() == CV_16U) {
                                                        const auto& p = image.at<cv::Vec3w>(x, z) ;
                                                        px = p[2];
                                                        py = p[1];
                                                        pz = p[0];
                                                } else {
                                                        std::cerr<<paths[y]<<std::endl;
                                                        std::cerr<<image.depth()<<std::endl;
                                                        throw std::runtime_error("Unsupport error.");
                                                }
                                                if(!(x == px && y == py && z == pz) ){
                                                        
                                                        std::cerr<<" "<<px<<" "<<py<<" "<<pz<<std::endl;
                                                        std::cerr<<" "<<x<<" "<<y<<" "<<z<<std::endl;
                                                        throw std::runtime_error("pixel color different");
                                                }
                                        }
                                }
                        }
                }
        } catch (std::runtime_error& e) {
                std::cerr<<e.what()<<std::endl;
                return -1;
        }
        std::cerr<<"validation ok"<<std::endl;
        return 0;
}