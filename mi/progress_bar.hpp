//
// Created by Takashi Michikawa on 2021/08/13.
//

#ifndef MI_PROGRESS_BAR_HPP
#define MI_PROGRESS_BAR_HPP 1

#include <cmath>
#include <mutex>
#include <string>
#include <type_traits>
#include <iostream>
#include <iomanip>
#include <concepts>
namespace mi{
        template <std::integral T>
        inline void progress_bar(const T v, const T vmax, const std::string header = "progress", const std::string::size_type ndots = 20) {
                std::cerr << "\r" << header << ":[" << std::setw(ndots) << std::setfill('-') << std::left << std::string(v * ndots / vmax, '*') << "] "<<"(" << std::setw(std::to_string(vmax).length())<<std::setfill(' ') <<std::right<< v << "/" << vmax << ")";
        }
        template <std::integral T>
        inline void progress_bar(std::mutex& mtx, const T v, const T max_value, const std::string header = "progress", const int ndots = 20) {
                std::lock_guard<std::mutex> lock(mtx);
                mi::progress_bar(v, max_value, header, ndots);
        }
        
        template <std::integral T>
        class ProgressBar {
        private:
                T max_value_;
                std::string header_;
                int ndots_;
                std::mutex mtx_;
        public:
                explicit ProgressBar (const T max_value, const std::string header = "progress", const int ndots = 20) : max_value_(max_value), header_(header), ndots_(ndots) {
                        this->print(T());
                }
                ~ProgressBar() {
                        std::cerr<<std::endl;
                };
                void print (const T value) {
                        mi::progress_bar(this->mtx_, value, this->max_value_, this->header_, this->ndots_);
                }
        };
};
#endif //MI_PROGRESS_BAR_HPP
