//
// Created by Takashi Michikawa on 2023/01/24.
//

#ifndef XYZ2ZXY_XYZ2ZXY_HPP
#define XYZ2ZXY_XYZ2ZXY_HPP

#include <mutex>
#include <string>
#include <thread>
#include <fmt/core.h>
#include <sstream>
#include <filesystem>
#include <vector>

namespace xyz2zxy {

        template<typename T>
        inline auto progress_bar(std::mutex &mtx, const T v, const T vmax, const std::string header = "progress",
                                 const int ndots = 20) {
                std::lock_guard<std::mutex> lock(mtx);
                std::cerr << fmt::format("\r{0}:[{1:-<{2}}] ({4:{3}d}/{5})", header,
                                         std::string(uint32_t(v * T(ndots) / vmax), '*'), ndots,
                                         std::to_string(vmax).length(), v, vmax);
        }

        void create_directory(const std::filesystem::path &path) {
                std::filesystem::create_directories(path);
                if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) {
                        throw std::runtime_error(path.string() + " cannot be created.");
                }
        };

        bool write_image(const std::string filename, const cv::Mat &image, std::vector<int> &params) {
                if (image.depth() <= 2) {
                        cv::imwrite(filename, image, params);
                        return true;
                } else {
                        std::cerr << "Unsupported depth:" << image.depth() << std::endl;
                        return false;
                }
        };

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
                                std::cerr << "dpi: " << params[3] << " x " << params[5] << std::endl;
                        }
                }
        }
}
#endif //XYZ2ZXY_XYZ2ZXY_HPP
