/**
 * @file progress_bar.hpp
 * @author Takashi Michikawa <tmichi@me.com>
 * @copyright (c) 2021 -  Takashi Michikawa
 * Released under the MIT license
 * https://opensource.org/licenses/mit-license.php
 */
#ifndef MI_PROGRESS_BAR_HPP
#define MI_PROGRESS_BAR_HPP 1

#include <cmath>
#include <mutex>
#include <string>
#include <type_traits>
#include <iostream>
//#include <fmt/core.h>

#if defined (WIN32) || defined (_WIN64)
#include <windows.h>
#include <io.h>
#endif
namespace mi{
        template <typename T>
        inline auto progress_bar(const T v, const T vmax, const std::string header = "progress", const int ndots = 20) -> decltype(std::enable_if_t<std::is_arithmetic_v<T>, T>(), void()) {
#if defined (WIN32) || defined (_WIN64)
            DWORD mode = 0;
            if (HANDLE handle = (HANDLE)_get_osfhandle(_fileno(stderr)); !GetConsoleMode(handle, &mode)) {
            }
            else if (!SetConsoleMode(handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
            }
            #endif
            //std::cerr << fmt::format("\033[G{0}:[{1:-<{2}}] ({4:{3}d}/{5})", header, std::string(uint32_t(v * T(ndots) / vmax), '*'), ndots, int(std::log10(vmax)) + 1, v, vmax);
            std::cerr << "\033[G" << header << ":[" << std::left<<std::setw(ndots) << std::string(uint32_t(std::round(v * T(ndots) / vmax)), '*') << "] (" << v << "/" << vmax << ")"<<std::flush;
        }

        // thread safe version
        template <typename T>
        inline auto progress_bar(std::mutex& mtx, const T v, const T max_value, const std::string header = "progress", const int ndots = 20) {
                std::lock_guard<std::mutex> lock(mtx);
                mi::progress_bar(v, max_value, header, ndots);
        }
}
#endif //MI_PROGRESS_BAR_HPP
