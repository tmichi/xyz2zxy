//
// Created by Takashi Michikawa on 2020/09/23.
//

#include <opencv2/opencv.hpp>
#include <mi/filesystem.hpp>
#include <string>
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>


std::tuple<int, int, int> getSizeType(const std::string &f) {
        cv::Mat img = cv::imread(f, cv::IMREAD_ANYDEPTH | cv::IMREAD_COLOR);
        return std::make_tuple(img.cols, img.rows, img.type());
}
int main(int argc, char **argv) {
        if (argc != 2) {
                std::cerr << "Usage " << argv[0] << " <input_dir>" << std::endl;
                return -1;
        }
        const std::string input_dir = argv[1];
        auto files = mi::list_files(input_dir);
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
        auto divide_mt = [&sx, &sy, &sz, &counter, &get_filename, &files]() {
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
                        cv::Mat img = cv::imread(files[z].string(), cv::IMREAD_ANYDEPTH | cv::IMREAD_COLOR);
                        for (int y = 0; y < sy; ++y) {
                                cv::Rect r(0, y, sx, 1);
                                cv::Mat clip(img, r);
                                cv::imwrite(get_filename(y, z), clip);
                        }
                }
        };
        auto collect_mt = [&sx, &sy, &sz, &counter, type, &get_filename]() {
                int y = -1 ;
                while (1) {
                        std::mutex mtx;
                        {
                                std::lock_guard<std::mutex> lock(mtx); // mtxを使ってロックする
                                y = counter;
                                ++counter;
                                std::cerr<<y<<std::endl;
                        }
                        if ( y >= sy ) break;
                        cv::Mat composite(sz, sx, type);
                        for (int z = 0; z < sz; ++z) {
                                cv::Mat clip = cv::imread(get_filename(y, z), cv::IMREAD_ANYDEPTH | cv::IMREAD_COLOR);
                                clip.copyTo(composite(cv::Rect(0, z, sx, 1)));
                        }
                        std::stringstream ss;
                        std::string header("result");
                        ss << header << std::setw(6) << std::setfill('0') << y << ".tif";
                        cv::imwrite(ss.str(), composite);
                }
        };

        int nt = 4;
        std::vector<std::thread> ths;
        counter = 0;
        for (int i = 0 ; i < nt ; ++i ) {
                ths.push_back( std::thread (divide_mt) );
        }
        for (int i = 0 ; i < nt ; ++i ) {
                ths[i].join();
        }
        ths.clear();
        std::cerr<<"divide done"<<std::endl;
        counter = 0;
        for (int i = 0 ; i < nt ; ++i ) {
                ths.push_back( std::thread (collect_mt) );
        }
        for (int i = 0 ; i < nt ; ++i ) {
                ths[i].join();
        }
        std::cerr<<"collect done"<<std::endl;
        for (int z = 0; z < sz; ++z) {
                for (int y = 0; y < sy; ++y) {
                        fs::remove(get_filename(y, z));
                }
        }
        return 0;
}
