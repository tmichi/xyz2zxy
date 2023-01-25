//
// Created by Takashi Michikawa on 2023/01/24.
//

#ifndef XYZ2ZXY_XYZ2ZXY_HPP
#define XYZ2ZXY_XYZ2ZXY_HPP

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <string>
#include <sstream>
#include <thread>
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

//#include <fmt/core.h>

#include <mi/thread_safe_counter.hpp>
#include <mi/repeat.hpp>
#include <mi/Attribute.hpp>
#include <mi/peak_memory_size.hpp>

#include <xyz2zxy_version.hpp>

namespace xyz2zxy {

        template<typename T>
        inline auto progress_bar(std::mutex &mtx, const T v, const T max_value, const std::string header = "progress",
                                 const int num_dots = 20) {
                std::lock_guard<std::mutex> lock(mtx);
                std::cerr << "\033[G" << header << ":[" << std::left << std::setw(num_dots) << std::string(uint32_t(v * T(num_dots) / max_value), '*') << "] (" << v << "/" << max_value << ")" << std::flush;
                //std::cerr << fmt::format("\r{0}:[{1:-<{2}}] ({4:{3}d}/{5})", header, std::string(uint32_t(v * T(num_dots) / max_value), '*'), num_dots,std::to_string(max_value).length(), v, max_value);
        }

        void create_directory(const std::filesystem::path &path) {
                std::filesystem::create_directories(path);
                if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) {
                        throw std::runtime_error(path.string() + " cannot be created.");
                }
        }

        bool write_image(const std::string &filename, const cv::Mat &image, std::vector<int> &params) {
                if (image.depth() <= 2) {
                        cv::imwrite(filename, image, params);
                        return true;
                } else {
                        std::cerr << "Unsupported depth:" << image.depth() << std::endl;
                        return false;
                }
        }

        void init_arguments(
                const std::string &cmd,
                mi::Argument &arg,
                std::filesystem::path &input_dir,
                std::filesystem::path &outputDir,
                int &step,
                std::filesystem::path &extension,
                std::vector<int> &params) {
                mi::AttributeSet attrSet;
                std::tuple<double, double> pitch(25.4, 25.4);
                attrSet.createAttribute("-i", input_dir).setMessage("Input directory").setMandatory();
                attrSet.createAttribute("-o", outputDir).setMessage("Output directory (default : output/)");
                attrSet.createAttribute("-n", step).setMessage(
                        "The number of steps (Default: 100, Larger n is probably fast but it causes large memory consumption.)").setValidator(
                        mi::attr::greater(0));
                attrSet.createAttribute("-ext", extension).setMessage(
                        "Extension of the images (e.g., .tif, .png. Default : .tif)");
                attrSet.createAttribute("-p", pitch).setMessage("Pixel resolution").setValidator([](const std::tuple<double, double>& v){ return std::get<0>(v)>0 && std::get<1>(v)>0;});

                if (!attrSet.parse(arg)) {
                        std::cerr << cmd << " version. " << XYZ2ZXY_VERSION << std::endl;
                        std::cerr << "Usage :" << std::endl;
                        attrSet.printUsage();
                        throw std::runtime_error("Insufficient arguments");
                }
                if (extension == ".tif") { //only tif
                        params.emplace_back(cv::IMWRITE_TIFF_COMPRESSION);
                        params.emplace_back(1); // no compression
                        if (arg.exist("-p")) {
                                // dpi =  25.4 mm / (pitch mm/pixel) (inch)
                                params.emplace_back(cv::IMWRITE_TIFF_XDPI);
                                params.emplace_back(std::round(25400.0 / std::get<0>(pitch)));
                                params.emplace_back(cv::IMWRITE_TIFF_YDPI);
                                params.emplace_back(std::round(25400.0 / std::get<1>(pitch)));
                        }
                }
        }


        std::vector<std::filesystem::path> list_files(const std::filesystem::path &p, const std::filesystem::path &tmp) {
                std::vector<std::filesystem::path> image_paths;
                if (std::filesystem::is_directory(p)) {
                        std::copy_if(std::filesystem::directory_iterator(p), std::filesystem::directory_iterator(), std::back_inserter(image_paths), [](const auto &f) {
                                             return !std::filesystem::is_directory(f) && f.path().filename().string().find_first_of(".") != 0;
                                     }
                        );
                } else if (p.extension() == ".tif" || p.extension() == ".tiff") {
                        auto input_dir = tmp / "input";
                        xyz2zxy::create_directory(input_dir);

                        std::vector<int> params = {cv::IMWRITE_TIFF_COMPRESSION, 1};
                        std::vector<cv::Mat> images;
                        const int n = int(cv::imcount(p.string()));
                        for (int i = 0; i < n; ++i) {
                                cv::imreadmulti(p.string(), images, i, 1, cv::IMREAD_UNCHANGED);
                                std::stringstream ss;
                                ss << "image-" << std::setw(5) << std::setfill('0') << i << ".tif";
                                image_paths.push_back(input_dir / ss.str());
                                //image_paths.push_back(input_dir / fmt::format("image-{:05}.tif", i));
                                cv::imwrite(image_paths[i].string(), images[0], params);
                        }
                } else {
                        throw std::runtime_error("Unsupported format");
                }
                if (image_paths.empty()) {
                        throw std::runtime_error("Empty images");
                }
                std::sort(image_paths.begin(), image_paths.end());
                return image_paths;
        }

        void get_volume_size(std::vector<std::filesystem::path> &image_paths, uint32_t &sx, uint32_t &sy, uint32_t &sz) {
                cv::Mat image = cv::imread(image_paths[0].string(), cv::IMREAD_UNCHANGED);
                sx = uint32_t(image.size().width);
                sy = uint32_t(image.size().height);
                sz = uint32_t(image_paths.size());
        }

        void print_peak_memory_size() {
                std::cout << "peak_memory_size[KB]: " << mi::peak_memory_size() / 1024.0 << std::endl;
        }

}
#endif //XYZ2ZXY_XYZ2ZXY_HPP
