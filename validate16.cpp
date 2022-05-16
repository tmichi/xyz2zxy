//
// Created by Takashi Michikawa on 2022/05/16.
//
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
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <fmt/core.h>
int main () {
        try {
                for (int y = 0 ; y < 256 ; ++y) {
                        const auto file = fmt::format("{0}/image-{1:05d}.tif", "output16", y);
                        if ( cv::Mat image = cv::imread(file, cv::IMREAD_UNCHANGED) ; image.empty() ) {
                                throw std::runtime_error(file + " was empty.");
                        } else if (image.size().width != 256 || image.size().height != 256) {
                                throw std::runtime_error(" Size different.");
                        } else {
                                for (int z = 0 ; z < 256 ; ++z) {
                                        for (int x = 0 ; x < 256; ++x) {
                                                if (const auto& p = image.at<cv::Vec3w>(z, x) ; 255 - z != p[0] || y != p[1] || x != p[2]) {
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
