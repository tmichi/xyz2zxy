/**
 * @file repeat.hpp
 * @brief
 * @author Takashi Michikawa <tmichi@me.com>
 * @copyright (c) 2017  Takashi Michikawa
 * Released under the MIT license
 * https://opensource.org/licenses/mit-license.php
 */

#ifndef MI_REPEAT_HPP
#define MI_REPEAT_HPP

#include <algorithm>
#include <functional>
#include <type_traits>
#include <thread>
#include <vector>
namespace mi {
        //template<typename Function, std::integral T = int>
        template<typename Function, typename T = int>
        inline auto repeat(Function fn, const T n) -> decltype(typename std::enable_if<std::is_integral_v<T>, T>::type(), void()) {
                for (T i = 0; i < n; ++i) {
                        fn();
                }
        }
        //template<typename Function, std::integral T = int>
        template<typename Function, typename T = int>
        inline auto repeat_mt(Function fn, const T n = T(std::max(1u, std::thread::hardware_concurrency())) ) -> decltype(typename std::enable_if<std::is_integral_v<T>, T>::type(), void()) {
                std::vector<std::thread> threads;
                mi::repeat([&threads, &fn](){threads.emplace_back(std::thread(fn));}, n);
                std::for_each(threads.begin(), threads.end(), [](auto& t) {t.join();});
        }
}
#endif //MI_REPEAT_HPP
