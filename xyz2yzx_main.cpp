/** @author Takashi Michikawa <michi@riken.jp>
  */
//#include "Xyz2ZxyProgram.hpp"
#include <vector>
#include <filesystem>
#include <iostream>
#include <algorithm>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <fmt/core.h>
#include <mi/thread_safe_counter.hpp>
#include <mi/repeat.hpp>
#include <mi/Attribute.hpp>
#include <xyz2zxy_version.hpp>

/**
 * MIT License
 * Copyright (c) 2022 RIKEN
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
template <typename T>
inline auto progress_bar(const T v, const T vmax, const std::string header = "progress", const int ndots = 20) {
        std::cerr<<fmt::format("\r{0}:[{1:-<{2}}] ({4:{3}d}/{5})", header, std::string(uint32_t(v * T(ndots) / vmax), '*'), ndots, int(std::log10(vmax))+1, v, vmax);
}
template <typename T>
inline auto progress_bar(std::mutex& mtx, const T v, const T vmax, const std::string header = "progress", const int ndots = 20) {
        std::lock_guard<std::mutex> lock(mtx);
        progress_bar(v, vmax, header, ndots);
}

int main(const int argc, const char **argv) {
        auto create_directory = [](const std::filesystem::path &path) {
                std::filesystem::create_directories(path);
                if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) {
                        throw std::runtime_error(path.string() + " cannot be created.");
                }
        };
        auto write_image = [](const std::string filename, const cv::Mat &image) {
                if (image.depth() <= 2) {
                        std::vector<int> params = {cv::IMWRITE_TIFF_COMPRESSION, 1};
                        cv::imwrite(filename, image, params);
                } else {
                        std::cerr << "Unsupported depth:" << image.depth() << std::endl;
                        return false;
                }
                return true;
        };
        try {
                std::filesystem::path outputDir;
                int step = 100;
                std::filesystem::path extension = ".tif";
                std::filesystem::path input_dir;
                mi::Argument arg(argc, argv);
                mi::AttributeSet attrSet;
                attrSet.createAttribute("-i", input_dir).setMessage("Input directory").setMandatory();
                attrSet.createAttribute("-o", outputDir).setMessage("Output directory (default : output/)");
                attrSet.createAttribute("-n", step).setMessage("The number of steps (Default: 100, Larger n is probably fast but it causes large memory consumption.)").setValidator(mi::attr::greater(0));
                attrSet.createAttribute("-ext", extension).setMessage("Extension of the images (e.g., .tif, .png. Default : .tif)");
                if (!attrSet.parse(arg)) {
                        std::cerr<<"xyz2yzx version. "<<XYZ2ZXY_VERSION<<std::endl;
                        std::cerr << "Usage :" << std::endl;
                        attrSet.printUsage();
                        throw std::runtime_error("Insufficient arguments");
                }
                
                std::vector<std::filesystem::path> image_paths;
                std::copy_if(std::filesystem::directory_iterator(input_dir), std::filesystem::directory_iterator(), std::back_inserter(image_paths), [&extension](const auto &f) {
                                     return !std::filesystem::is_directory(f) &&
                                            f.path().filename().string().find_first_of(".") != 0 &&
                                            f.path().extension() == extension;
                             }
                );
                if (image_paths.empty()) {
                        throw std::runtime_error("Empty images");
                }
                std::sort(image_paths.begin(), image_paths.end());
        
                create_directory(outputDir);
                std::filesystem::path tmpDir = outputDir.string() + "_temp";
                create_directory(tmpDir);
                auto get_tmp_filename = [&tmpDir, &extension](const uint32_t y, const uint32_t z) {
                        return fmt::format("{}/{}/image-{:05d}{}", tmpDir.string(), z, y, extension.string());
                };
                // get volume size
                cv::Mat image = cv::imread(image_paths[0].string(), cv::IMREAD_UNCHANGED);
                const uint32_t sx = uint32_t(image.size().width);
                const uint32_t sy = uint32_t(image.size().height);
                const uint32_t sz = uint32_t(image_paths.size());
                image.release();
                std::string step1Str{"Step1 divide"};
                progress_bar(0u, sz, step1Str);
                mi::thread_safe_counter<uint32_t> counter;
                for (uint32_t z = 0; z < sz; z += step) {
                        std::vector<cv::Mat> images;
                        const uint32_t end = (z + step < sz) ? z + step : sz;
                        std::transform(image_paths.begin() + z, image_paths.begin() + end, std::back_inserter(images), [](auto &f) {
                                return cv::imread(f.string(), cv::IMREAD_UNCHANGED);
                        });
                        create_directory(tmpDir / std::to_string(z));
                        mi::repeat_mt([&counter, &images, &sx, &sy, &z, &get_tmp_filename, &write_image]() {
                                for (uint32_t x = counter.get(); x < sx; x = counter.get()) {
                                        std::vector<cv::Mat> local_images;
                                        std::transform(images.begin(), images.end(), std::back_inserter(local_images),[&x, &sy](auto &image) { return cv::Mat(image, cv::Rect(cv::Point( int(x), 0), cv::Size(1, int(sy)))); }); // cut
                                        cv::Mat local;
                                        cv::hconcat(local_images, local);
                                        write_image(get_tmp_filename(x, z), local);
                                }
                        });
                        progress_bar(z + uint32_t(images.size()), sz, step1Str);
                        counter.reset(0);
                }
                std::cerr << std::endl;
                mi::thread_safe_counter<uint32_t> num_of_finished;
                progress_bar<uint32_t>(num_of_finished.get(), sx, "Step2 concat");
                std::mutex mtx;
                mi::repeat_mt([&]() {
                                for (uint32_t x = counter.get(); x < sx; x = counter.get()) {
                                        std::vector<cv::Mat> local_images;
                                        for (uint32_t z = 0; z < sz; z += step) {
                                                local_images.push_back(cv::imread(get_tmp_filename(x, z), cv::IMREAD_ANYDEPTH | cv::IMREAD_ANYCOLOR));
                                        }
                                        cv::Mat result;
                                        cv::hconcat(local_images, result);
                                        cv::flip(result, result, 0); // mirroring
                                        cv::rotate(result, result, cv::ROTATE_90_CLOCKWISE);
                                        write_image(outputDir.string() + "/" + fmt::format("image-{:05d}.tif", x), result);
                                        progress_bar(mtx, num_of_finished.get(), sx, "Step2 concat");
                                }
                        });
                std::cerr << std::endl;
                std::filesystem::remove_all(tmpDir);
        } catch (std::runtime_error &e) {
                std::cerr << e.what() << std::endl;
        } catch (cv::Exception& e) {
                std::cerr<<"OpenCV exception "<<e.what()<<std::endl;
        }
        catch (...) {
                std::cerr << "Unknown error" << std::endl;
        }
        return EXIT_SUCCESS;
}
