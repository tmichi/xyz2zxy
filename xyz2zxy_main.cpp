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
#include "xyz2zxy.hpp"
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


int main(const int argc, const char **argv) {
        try {
                std::mutex mtx;
                mi::Argument arg(argc, argv);

                std::filesystem::path input_dir;
                std::filesystem::path outputDir;
                int step = 100;
                std::filesystem::path extension = ".tif";
                std::vector<int> params;
                xyz2zxy::init_arguments("xyz2zxy",arg, input_dir, outputDir, step, extension, params);


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

                xyz2zxy::create_directory(outputDir);
                std::filesystem::path tmpDir = outputDir.string() + "_temp";
                xyz2zxy::create_directory(tmpDir);
                auto get_tmp_filename = [&tmpDir, &extension](const uint32_t y, const uint32_t z) {
                        return fmt::format("{}/{}/image-{:05d}{}", tmpDir.string(), z, y, extension.string());
                };
                // get volume size
                cv::Mat image = cv::imread(image_paths[0].string(), cv::IMREAD_UNCHANGED);
                const uint32_t sx = uint32_t(image.size().width);
                const uint32_t sy = uint32_t(image.size().height);
                image.release();
                const uint32_t sz = uint32_t(image_paths.size());
                
                std::string step1Str{"Step1 divide"};
                xyz2zxy::progress_bar(mtx, 0u, sz, step1Str);
                mi::thread_safe_counter<uint32_t> counter;
                for (uint32_t z = 0; z < sz; z += step) {
                        std::vector<cv::Mat> images;
                        const uint32_t end = (z + step < sz) ? z + step : sz;
                        std::transform(image_paths.begin() + z, image_paths.begin() + end, std::back_inserter(images), [](auto &f) { return cv::imread(f.string(), cv::IMREAD_UNCHANGED); });
                        create_directory(tmpDir / std::to_string(z));
                        mi::repeat_mt([&counter, &images, &sx, &sy, &z, &get_tmp_filename, &params]() {
                                for (uint32_t y = counter.get(); y < sy; y = counter.get()) {
                                        std::vector<cv::Mat> local_images;
                                        std::transform(images.begin(), images.end(), std::back_inserter(local_images),[&y, &sx](auto &image) { return cv::Mat(image, cv::Rect(cv::Point(0, int(y)), cv::Size(int(sx), 1))); }); // cut
                                        cv::Mat local;
                                        cv::vconcat(local_images, local);
                                        xyz2zxy::write_image(get_tmp_filename(y, z), local, params);
                                }
                        });
                        xyz2zxy::progress_bar(mtx, z + uint32_t(images.size()), sz, step1Str);
                        counter.reset(0);
                }
                std::cerr << std::endl;
                mi::thread_safe_counter<uint32_t> num_of_finished;
                xyz2zxy::progress_bar<uint32_t>(mtx, num_of_finished.get(), sy, "Step2 concat");
                mi::repeat_mt([&]() {
                                for (uint32_t y = counter.get(); y < sy; y = counter.get()) {
                                        std::vector<cv::Mat> local_images;
                                        for (uint32_t z = 0; z < sz; z += step) {
                                                local_images.push_back(cv::imread(get_tmp_filename(y, z), cv::IMREAD_ANYDEPTH | cv::IMREAD_ANYCOLOR));
                                        }
                                        cv::Mat result;
                                        cv::vconcat(local_images, result);
                                        cv::flip(result, result, 0); // mirroring
                                        cv::rotate(result, result, cv::ROTATE_90_CLOCKWISE);
                                        xyz2zxy::write_image(outputDir.string() + "/" + fmt::format("image-{:05d}.tif", y), result, params);
                                        xyz2zxy::progress_bar(mtx, num_of_finished.get(), sy, "Step2 concat");
                                }
                        });
                std::cerr << std::endl;
                std::filesystem::remove_all(tmpDir);
        } catch (std::runtime_error &e) {
                std::cerr << e.what() << std::endl;
        } catch (...) {
                std::cerr << "Unknown error" << std::endl;
        }
        return EXIT_SUCCESS;
}
