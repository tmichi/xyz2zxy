/**
 * @file thread_safe_counter.hpp
 * @brief
 * @author Takashi Michikawa <tmichi@me.com>
 * @copyright (c) 2020  Takashi Michikawa
 * Released under the MIT license
 * https://opensource.org/licenses/mit-license.php
 */

#ifndef MI_THREAD_SAFE_COUNTER_HPP
#define MI_THREAD_SAFE_COUNTER_HPP 1

#include <mutex>
#include <stdexcept>
#include <cstdint>
namespace mi {
        template<typename T = int32_t>
        class thread_safe_counter {
        private:
                T n_; ///< counter
                std::mutex mtx_;
        public:
                /**
                 * @brief Constructor.
                 * @param s Init value.
                 */
                explicit thread_safe_counter(const T s = T()) : n_(s) {}

                thread_safe_counter(const thread_safe_counter &d) = delete;

                thread_safe_counter(thread_safe_counter &&d) = delete;

                thread_safe_counter &operator=(const thread_safe_counter &d) = delete;

                thread_safe_counter &&operator=(thread_safe_counter &&d) = delete;

                ~thread_safe_counter() = default;
  
                T get() {
                        std::lock_guard <std::mutex> lock(this->mtx_);
                        if ( this->n_ == std::numeric_limits<T>::max() ) {
                                throw std::runtime_error("mi::thread_safe_counter<T>::get() : max value");
                        }
                        return this->n_++;
                }
                
                void reset(const T s = T()) {
                        this->n_ = s;
                }
        };
}
#endif //MI_THREAD_SAFE_COUNTER_HPP
