/** @author Takashi Michikawa <michi@riken.jp>
  */
#include <xyz2zxy.hpp>
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
int main(const int argc, const char **argv) {
        try {
                std::mutex mtx;
                mi::Argument arg(argc, argv);
                std::filesystem::path input_dir;
                std::filesystem::path outputDir;
                int step = 100;
                std::filesystem::path extension = ".tif";
                std::vector<int> params;
                xyz2zxy::init_arguments("xyz2yzx", arg, input_dir, outputDir, step, extension, params);

                std::filesystem::path tmpDir = outputDir.string() + "_temp";
                xyz2zxy::create_directory(tmpDir);
                std::vector<std::filesystem::path> image_paths = xyz2zxy::list_files(input_dir, tmpDir);

                xyz2zxy::create_directory(outputDir);

                auto get_tmp_filename = [&tmpDir, &extension](const uint32_t y, const uint32_t z) {
                        std::stringstream ss;
                        ss << tmpDir.string() << "/" << z << "/" << "image-" << std::setw(5) << std::setfill('0') << y << extension.string();
                        return ss.str();
                        //        return fmt::format("{}/{}/image-{:05d}{}", tmpDir.string(), z, y, extension.string());
                };

                uint32_t sx, sy, sz;
                xyz2zxy::get_volume_size(image_paths, sx, sy, sz);

                std::string step1Str{"Step1 divide"};
                xyz2zxy::progress_bar(mtx, 0u, sz, step1Str);
                mi::thread_safe_counter<uint32_t> counter;
                for (uint32_t z = 0; z < sz; z += step) {
                        std::vector<cv::Mat> images;
                        const uint32_t end = (z + step < sz) ? z + step : sz;
                        std::transform(image_paths.begin() + z, image_paths.begin() + end, std::back_inserter(images), [](auto &f) {
                                return cv::imread(f.string(), cv::IMREAD_UNCHANGED);
                        });
                        create_directory(tmpDir / std::to_string(z));
                        mi::repeat_mt([&counter, &images, &sx, &sy, &z, &get_tmp_filename, &params]() {
                                for (uint32_t x = counter.get(); x < sx; x = counter.get()) {
                                        std::vector<cv::Mat> local_images;
                                        std::transform(images.begin(), images.end(), std::back_inserter(local_images),[&x, &sy](auto &image) { return cv::Mat(image, cv::Rect(cv::Point( int(x), 0), cv::Size(1, int(sy)))); }); // cut
                                        cv::Mat local;
                                        cv::hconcat(local_images, local);
                                        xyz2zxy::write_image(get_tmp_filename(x, z), local, params);
                                }
                        });
                        xyz2zxy::progress_bar(mtx, z + uint32_t(images.size()), sz, step1Str);
                        counter.reset(0);
                }
                std::cerr << std::endl;
                mi::thread_safe_counter<uint32_t> num_of_finished;
                xyz2zxy::progress_bar<uint32_t>(mtx, num_of_finished.get(), sx, "Step2 concat");
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
                                        // xyz2zxy::write_image(outputDir.string() + "/" + fmt::format("image-{:05d}.tif", x), result,params);
                                        std::stringstream ss;
                                        ss << outputDir.string() << "/" << "image-" << std::setw(5) << std::setfill('0') << x << extension.string();
                                        xyz2zxy::write_image(ss.str(), result, params);
                                        xyz2zxy::progress_bar(mtx, num_of_finished.get(), sx, "Step2 concat");
                                }
                        });
                std::cerr << std::endl;
                std::filesystem::remove_all(tmpDir);
                xyz2zxy::print_peak_memory_size();
        } catch (std::runtime_error &e) {
                std::cerr << e.what() << std::endl;
        } catch (...) {
                std::cerr << "Unknown error" << std::endl;
        }
        return EXIT_SUCCESS;
}
