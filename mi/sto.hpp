/**
 * @file sto.hpp
 * @brief
 * @author Takashi Michikawa <tmichi@me.com>
 * @copyright (c) 2017  Takashi Michikawa
 * Released under the MIT license
 * https://opensource.org/licenses/mit-license.php
 */
#ifndef MI_STO_HPP
#define MI_STO_HPP 1

#include <string>

namespace mi {
        template<typename T>
        inline auto sto(const std::string &str) -> decltype(T()) {
                if constexpr (std::is_same_v < T, long double >) return std::stold(str);
                else if constexpr (std::is_same_v < T, double >) return std::stod(str);
                else if constexpr (std::is_same_v < T, float >) return std::stof(str);
                else if constexpr (std::is_floating_point_v < T >) return T(std::stod(str));
                else if constexpr (std::is_same_v < T, uint64_t >) return std::stoull(str);
                else if constexpr (std::is_same_v < T, int64_t >) return std::stoll(str);
                else if constexpr (std::is_unsigned_v < T >) return T(std::stoul(str));
                else if constexpr (std::is_signed_v < T >) return T(std::stol(str));
                else return str;
        }
}
#endif //MI_STO_HPP
