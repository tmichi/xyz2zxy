/** @author Takashi Michikawa <michi@riken.jp>
  */
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

#include <xyz2zxy.hpp>
int main(const int argc, const char **argv) {
        try {
                std::mutex mtx;
                mi::Argument arg(argc, argv);
                std::filesystem::path input_dir;
                std::filesystem::path outputDir;
                int step = 100;
                std::filesystem::path extension = ".tif";
                std::vector<int> params;

                xyz2zxy::init_arguments("xyz2zxy", arg, input_dir, outputDir, step, extension, params);

                std::filesystem::path tmpDir = outputDir.string() + "_temp";
                xyz2zxy::create_directory(tmpDir);

                std::vector<std::filesystem::path> image_paths = xyz2zxy::list_files(input_dir, tmpDir, extension);

                xyz2zxy::create_directory(outputDir);
                auto get_tmp_filename = [&tmpDir, &extension](const uint32_t y, const uint32_t z) {
                        return std::format("{}/{}/image-{:05d}{}", tmpDir.string(), z, y, extension.string());
                };
                // get volume size
                auto [sx, sy, sz] = xyz2zxy::get_volume_size(image_paths);
                std::string step1Str{"Step1 divide"};
                xyz2zxy::progress_bar(mtx, 0u, sz, step1Str);
                mi::thread_safe_counter<uint32_t> counter;
                { // Release the input batch before allocating final output planes.
                        std::vector<cv::Mat> images(std::min<uint32_t>(step, sz));
                        std::vector<uchar> encoded;
                        // Each worker keeps its slice buffer across both slices and batches.
                        std::vector<cv::Mat> slices(xyz2zxy::worker_count(sy));
                        for (uint32_t z = 0; z < sz; z += step) {
                                const uint32_t count = std::min<uint32_t>(step, sz - z);
                                for (uint32_t i = 0; i < count; ++i) {
                                        xyz2zxy::read_image(image_paths[z + i].string(), images[i], encoded, cv::IMREAD_UNCHANGED);
                                }
                                create_directory(tmpDir / std::to_string(z));
                                mi::thread_safe_counter<unsigned> worker;
                                mi::repeat_mt([&]() {
                                        cv::Mat &local = slices[worker.get()];
                                        local.create(int(count), int(sx), images[0].type());
                                        for (uint32_t y = counter.get(); y < sy; y = counter.get()) {
                                                for (uint32_t i = 0; i < count; ++i) {
                                                        images[i].row(int(y)).copyTo(local.row(int(i)));
                                                }
                                                xyz2zxy::write_image(get_tmp_filename(y, z), local, params);
                                        }
                                }, static_cast<unsigned>(slices.size()));
                                xyz2zxy::progress_bar(mtx, z + count, sz, step1Str);
                                counter.reset(0);
                        }
                }
                std::cerr << std::endl;
                mi::thread_safe_counter<uint32_t> num_of_finished;
                xyz2zxy::progress_bar<uint32_t>(mtx, num_of_finished.get(), sy, "Step2 concat");
                mi::repeat_mt([&]() {
                                cv::Mat full_chunk, tail_chunk, result;
                                std::vector<uchar> encoded;
                                for (uint32_t y = counter.get(); y < sy; y = counter.get()) {
                                        for (uint32_t z = 0; z < sz; z += step) {
                                                const uint32_t count = std::min<uint32_t>(step, sz - z);
                                                cv::Mat &chunk = count == uint32_t(step) ? full_chunk : tail_chunk;
                                                xyz2zxy::read_image(get_tmp_filename(y, z), chunk, encoded,
                                                                   cv::IMREAD_ANYDEPTH | cv::IMREAD_ANYCOLOR);
                                                result.create(int(sx), int(sz), chunk.type());
                                                // flip vertically + rotate clockwise is a transpose.
                                                // Write each chunk directly into the final plane.
                                                cv::Mat destination = result.colRange(int(z), int(z + count));
                                                cv::transpose(chunk, destination);
                                        }
                                        const std::filesystem::path outputFile = outputDir / std::format("image-{:05d}{}", y, extension.string());
                                        xyz2zxy::write_image(outputFile.string(), result, params);
                                        xyz2zxy::progress_bar(mtx, num_of_finished.get(), sy, "Step2 concat");
                                }
                        }, xyz2zxy::worker_count(sy));
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
