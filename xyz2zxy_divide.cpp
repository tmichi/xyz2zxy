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

int main(int argc, char **argv) {
        if (argc != 2) {
                std::cerr << "Usage " << argv[0] << " <input_dir>" << std::endl;
                return -1;
        }
        const fs::path input_path(argv[1]);
        const std::string output_dir = input_path.stem().string() + "_temp";
        mi::make_directory(output_dir);
        std::cerr<<output_dir<<std::endl;
        auto files = mi::list_files(input_path);
        std::sort(files.begin(), files.end());
        if (files.size() == 0) {
                std::cerr << "Images not found" << std::endl;
                return -1;
        }

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

        int counter = 0;
        auto divide_mt = [&sx, &sy, &sz, &counter, &get_filename, &files, &output_dir]() {
                int z = -1;
                while (1) {
                        std::mutex mtx;
                        {
                                std::lock_guard<std::mutex> lock(mtx); // mtxを使ってロックする
                                z = counter;
                                ++counter;
                                std::cerr<<z<<std::endl;
                        }
                        if ( z >= sz ) break;
			std::stringstream ss;
			ss<<output_dir<<"/"<<std::setw(5)<<std::setfill('0')<<z;
			mi::make_directory(ss.str());
                        cv::Mat img = cv::imread(files[z].string(), cv::IMREAD_ANYDEPTH | cv::IMREAD_COLOR);
                        for (int y = 0; y < sy; ++y) {
                                cv::Rect r(0, y, sx, 1);
                                cv::Mat clip(img, r);
                                cv::imwrite(ss.str()+"/"+get_filename(y, z), clip);
                        }
                }
        };

        int nt = std::thread::hardware_concurrency();
	std::cerr<<nt<<std::endl;
        std::vector<std::thread> ths;
        for (int i = 0 ; i < nt ; ++i ) {
                ths.push_back( std::thread (divide_mt) );
        }
        for (int i = 0 ; i < nt ; ++i ) {
                ths[i].join();
        }
        return 0;
}
