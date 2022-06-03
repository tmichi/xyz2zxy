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
#include <iostream>
#include <iomanip>
#include <tuple>
#include <algorithm>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <mi/thread_safe_counter.hpp>
#include <mi/repeat.hpp>
#include <mi/progress_bar.hpp>
#include <mi/Argument.hpp>
#include "Version.hpp"
std::ostream& operator << (std::ostream& os, std::tuple<double, double>& v) {
        os << "(" << std::get<0>(v) << ", " << std::get<1>(v) << ")" << std::endl;
        return os;
}
namespace mi {
        template<typename T>
        inline auto attribute_getter() -> decltype(std::enable_if_t<std::is_same_v<T, std::tuple<double, double> > >(),
                std::function<bool(const Argument &arg, const std::string &, T &)>()){
                return [](const Argument& arg, const std::string& key, std::tuple<double, double>& value) {
                        const bool exists = arg.exist(key, 2);
                        if (exists) {
                                value = std::tuple<double, double>(arg.get<double>(key, 1), arg.get<double>(key, 2));
                        }
                        return exists;
                };
        }
}

#include "Xyz2ZxyProgram.hpp"
Xyz2ZxyProgram::Xyz2ZxyProgram(const mi::Argument& arg) : mi::ProgramTemplate(arg, "xyz2zxy", std::string("v.")+ XYZ2ZXY_VERSION), output_dir_("output"), num_(100), extension_(".tif"), pitch_(std::tuple<double, double>(1, 1)) {
        std::filesystem::path input_dir;
        this->getAttributeSet().createAttribute("-i", input_dir).setMessage("Input directory").setMandatory();
        this->getAttributeSet().createAttribute("-o", this->output_dir_).setMessage("Output directory (default : output/)");
        this->getAttributeSet().createAttribute("-n", this->num_).setMessage("The number of steps (Default: 100, Larger n is probably fast but it causes large memory consumption.)").setValidator(mi::attr::greater(0));
        this->getAttributeSet().createAttribute("-ext", this->extension_).setMessage("Extension of the images (e.g., .tif, .png. Default : .tif)");
        this->getAttributeSet().createAttribute("-pitch", this->pitch_).setMessage("Custom voxel pitch (z, x)").setValidator([](const std::tuple<double, double>& p) {return std::get<0>(p) > 0 && std::get<1>(p) > 0; }, true);
        if (!this->getAttributeSet().parse(arg)) {
                std::cerr << "Usage :" << std::endl;
                this->getAttributeSet().printUsage();
                throw std::runtime_error("Insufficient arguments");
        }
        std::copy_if(std::filesystem::directory_iterator(input_dir), std::filesystem::directory_iterator(), std::back_inserter(this->image_paths_), [&ext = this->extension_](const auto& f){
                return !std::filesystem::is_directory(f) && f.path().filename().string().find_first_of(".") != 0 && f.path().extension() == ext; }
        );
        if (this->image_paths_.empty()) {
                throw std::runtime_error("Empty images");
        }
        std::sort(this->image_paths_.begin(), this->image_paths_.end());
        this->getAttributeSet().printValues(std::cerr);
}


Xyz2ZxyProgram::~Xyz2ZxyProgram() {

}

bool
Xyz2ZxyProgram::run() {
        auto write_image = [](const std::string filename, const cv::Mat& image, std::vector<int>& params) {
                if (image.depth() <= 2) {
                        cv::imwrite(filename, image, params);
                }
                else {
                        std::cerr << "Unsupported depth:" << image.depth() << std::endl;
                        return false;
                }
                return true;
        };

        auto& paths = this->image_paths_;
        std::filesystem::path tmpDir = this->output_dir_.string() + "_temp";
        std::filesystem::create_directories(tmpDir);
        if (!std::filesystem::exists(tmpDir) || !std::filesystem::is_directory(tmpDir)) {
                return false;
        }

        std::filesystem::create_directories(this->output_dir_);
        if (!std::filesystem::exists(this->output_dir_) || !std::filesystem::is_directory(this->output_dir_)) {
                return false;
        }


        auto get_tmp_filename = [&tmpDir, &ext = this->extension_](const uint32_t y, const uint32_t z) {
                std::stringstream ss;
                ss << tmpDir.string() << "/" << z << "/image-" << std::setw(5) << std::setfill('0') <<y<< ext.string();
                return ss.str();
                //    return fmt::format("{}/{}/image-{:05d}{}", tmpDir.string(), z, y, ext.string());
        };

        auto get_output_filename = [&outDir = this->output_dir_, &ext = this->extension_](const uint32_t y) {
                std::stringstream ss;
                ss << outDir.string() << "/image-" << std::setw(5) << std::setfill('0') <<y<< ext.string();
                return ss.str();
                //   return fmt::format("{}/image-{:05d}.tif", outDir.string(), y);
        };

        const uint32_t step = uint32_t(this->num_);


        std::tuple<double, double> dpi;
        // dpi =  25.4 mm / (pitch mm/pixel) (inch)
        std::get<0>(dpi) = std::round(25400 / std::get<0>(this->pitch_));
        std::get<1>(dpi) = std::round(25400 / std::get<1>(this->pitch_));

        std::cerr << "dpi " << std::get<0>(dpi) << " " << std::get<1>(dpi) << std::endl;
        // get volume size
        cv::Mat image = cv::imread(paths[0].string(), cv::IMREAD_UNCHANGED);
        const uint32_t sx = uint32_t(image.size().width);
        const uint32_t sy = uint32_t(image.size().height);
        std::cerr << "image size: " << image.size() << std::endl;
        image.release();
        const uint32_t sz = uint32_t(paths.size());

        std::string step1Str{ "Step1 divide" };
        mi::progress_bar(0u, sz, step1Str);
        mi::thread_safe_counter<uint32_t> counter;
        for (uint32_t z = 0; z < sz; z += step) {
                std::vector<cv::Mat> images;
                const uint32_t end = (z + step < sz) ? z + step : sz;
                std::transform(paths.begin() + z, paths.begin() + end, std::back_inserter(images), [](auto& f) {return cv::imread(f.string(), cv::IMREAD_UNCHANGED); });
                std::filesystem::create_directory(tmpDir / std::to_string(z));
                mi::repeat_mt([&counter, &images, &sx, &sy, &z, &get_tmp_filename, &write_image]() {
                        for (uint32_t y = counter.get(); y < sy; y = counter.get()) {
                                std::vector<cv::Mat> local_images;
                                std::transform(images.begin(), images.end(), std::back_inserter(local_images), [&y, &sx](auto& image) {return cv::Mat(image, cv::Rect(cv::Point(0, int(y)), cv::Size(int(sx), 1))); }); // cut
                                cv::Mat local;
                                cv::vconcat(local_images, local);
                                std::vector<int> params = { cv::IMWRITE_TIFF_COMPRESSION, 1 };
                                write_image(get_tmp_filename(y, z), local, params);
                        
                        }
                        });
                mi::progress_bar(z + uint32_t(images.size()), sz, step1Str);
                counter.reset(0);
        }
        std::cerr << std::endl;
        mi::thread_safe_counter<uint32_t> num_of_finished;
        mi::progress_bar<uint32_t>(num_of_finished.get(), sy, "Step2 concat");
        std::mutex mtx;

        mi::repeat_mt(
                [&num_of_finished, &mtx, &counter, &get_tmp_filename, &sy, &step, &sz, &write_image, &dpi, &get_output_filename]() {
                for (uint32_t y = counter.get(); y < sy; y = counter.get()) {
                        std::vector<cv::Mat> local_images;
                        for (uint32_t z = 0; z < sz; z += step) {
                                local_images.push_back(cv::imread(get_tmp_filename(y, z), cv::IMREAD_ANYDEPTH | cv::IMREAD_ANYCOLOR));
                        }
                        cv::Mat result;
                        cv::vconcat(local_images, result);
                        cv::flip(result, result, 0); // mirroring
                        std::vector<int> params = {
                            cv::IMWRITE_TIFF_XDPI, int(std::get<0>(dpi)),
                            cv::IMWRITE_TIFF_YDPI, int(std::get<1>(dpi)),
                            cv::IMWRITE_TIFF_COMPRESSION, 1 //NO COMPSSION
                        };
                        write_image(get_output_filename(y), result, params);
                        mi::progress_bar(mtx, num_of_finished.get(), sy, "Step2 concat");
                }
        });
        std::cerr << std::endl;
        std::filesystem::remove_all(tmpDir);
        return true;
}



