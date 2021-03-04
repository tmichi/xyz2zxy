//
// Created by Takashi Michikawa on 2020/09/24.
//

#include "get_size_type.hpp"
#include <opencv2/opencv.hpp>
#include <mi/filesystem.hpp>
#include <string>
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <mi/thread_safe_counter.hpp>

int main(int argc, char **argv) {
        if (argc < 2) {
                std::cerr << "Usage " << argv[0] << " <input_dir>" << std::endl;
                return -1;
        }
        const fs::path input_path(argv[1]);
        const std::string input_dir = input_path.stem().string() + "_temp";
        const std::string output_dir = input_path.stem().string() + "_result";
        auto files = mi::list_files(input_path);
        std::sort(files.begin(), files.end());
        if (files.size() == 0) {
                std::cerr << "Images not found" << std::endl;
                return -1;
        }
        mi::make_directory(output_dir);
        
        auto[sx0, sy0, type0] = getSizeType(files[0].string());
        int sx = sx0;
        int sy = sy0;
        int sz = files.size();
        int type = type0;
        std::cerr << "Size:" << sx << " " << sy << " " << sz << " " << type << std::endl;
        auto get_filename = [](const int y, const int z) {
                std::stringstream ss2;
                ss2 << "temp" << std::setw(6) << std::setfill('0') << z << "-" << std::setw(6) << std::setfill('0') << y << ".tif";
                return ss2.str();
        };
        
        mi::thread_safe_counter counter;
        auto collect_mt = [&sx, &sy, &sz, &counter, type, &get_filename, &output_dir, &input_dir]() {
                int y = -1;
                while (1) {
                        y = counter.get();
                        std::cerr << y << std::endl << std::flush;
                        if (y >= sy) break;
                        std::stringstream ss;
                        std::string header("result");
                        ss << header << std::setw(6) << std::setfill('0') << y << ".tif";
                        if (fs::exists(output_dir + "/" + ss.str()))continue;
                        cv::Mat composite(sz, sx, type);
                        for (int z = 0; z < sz; ++z) {
                                std::stringstream ss2;
                                ss2 << input_dir << "/" << std::setw(5) << std::setfill('0') << z;
                                cv::Mat clip = cv::imread(ss2.str() + std::string("/") + get_filename(y, z), cv::IMREAD_ANYDEPTH | cv::IMREAD_COLOR);
                                clip.copyTo(composite(cv::Rect(0, z, sx, 1)));
                        }
                        cv::imwrite(output_dir + "/" + ss.str(), composite);
                }
        };
        
        int nt = std::thread::hardware_concurrency();
        std::cerr << nt << std::endl;
        std::vector<std::thread> ths;
        counter.reset();
        for (int i = 0; i < nt; ++i) {
                ths.push_back(std::thread(collect_mt));
        }
        for (int i = 0; i < nt; ++i) {
                ths[i].join();
        }
        std::cerr << "collect done" << std::endl;
        for (int z = 0; z < sz; ++z) {
                for (int y = 0; y < sy; ++y) {
                        std::stringstream ss2;
                        ss2 << input_dir << "/" << std::setw(5) << std::setfill('0') << z;
                        fs::remove(ss2.str() + "/" + get_filename(y, z));
                }
        }
        return 0;
}
