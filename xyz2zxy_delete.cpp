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
        if (argc < 2) {
                std::cerr << "Usage " << argv[0] << " <input_dir>" << std::endl;
                return -1;
        }
        const fs::path input_path(argv[1]);
        const std::string input_dir = input_path.stem().string()+"_temp";
        auto get_filename = [](const int y, const int z) {
                std::stringstream ss2;
                ss2 << "temp" << std::setw(6) << std::setfill('0') << z << "-" << std::setw(6) << std::setfill('0') << y << ".tif";
                return ss2.str();
        };
        std::cerr<<"collect done"<<std::endl;
        for (int z = 0; z < sz; ++z) {
                for (int y = 0; y < sy; ++y) {
                        fs::remove(input_dir+"/"+get_filename(y, z));
                }
        }
        return 0;
}
