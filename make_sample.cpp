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
#include <opencv2/opencv.hpp>
#include <filesystem>
#include <sstream>
int main () {
        try {
                std::filesystem::path dir("sample");
                std::filesystem::create_directory(dir);
                if (!std::filesystem::exists(dir)) {
                        throw std::runtime_error(dir.string() + " cannot be created");
                }
                cv::Mat image (cv::Size(256, 256), CV_8UC3);
                for (int z = 0 ; z < 256 ; ++z) {
                        for (int y = 0 ; y < 256 ; ++y) {
                                for (int x = 0 ; x < 256; ++x) {
                                        image.at<cv::Vec3b>(y, x) = cv::Vec3b(uint8_t(z), uint8_t(y), uint8_t(x));
                                }
                        }
                        std::stringstream ss;
                        ss<<dir.string()<<"/image-"<<std::setw(5)<<std::setfill('0')<<z<<".tif";
                        if (!cv::imwrite(ss.str(), image)) {
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