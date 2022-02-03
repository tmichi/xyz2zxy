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
#include <limits>
#include <string>
#include <cstdint>
namespace mi {
        template<typename T = int32_t>
        class thread_safe_counter {
        private:
                T n_; ///< counter
                std::mutex mtx_;
                bool is_valid_;
        public:
                /**
                 * @brief Constructor.
                 * @param s Init value.
                 */
                explicit thread_safe_counter(const T s = T()) : n_(s), is_valid_(true) {}

                thread_safe_counter(const thread_safe_counter &d) = delete;

                thread_safe_counter(thread_safe_counter &&d) = delete;

                thread_safe_counter &operator=(const thread_safe_counter &d) = delete;

                thread_safe_counter &&operator=(thread_safe_counter &&d) = delete;

                ~thread_safe_counter() = default;

                /**
                 * @brief
                 * @throw runtime_error if value reaches max.
                 * @return Value
                 * @note can get value less than std:numeric_limits<T>::max()
                 */
                const T get() {
                        if (std::lock_guard<std::mutex> lock(this->mtx_); this->is_valid_) {
                                this->is_valid_ = (this->n_ < std::numeric_limits<T>::max());
                                return this->n_++;
                        } else {
                                throw std::overflow_error("thread_safe_counter::get() exceeds " + std::to_string(std::numeric_limits<T>::max()));
                        }
                }

                /**
                 * @brief Reset counter.
                 * @param s reset value.
                 */
                void reset(const T s = T()) {
                        this->n_ = s;
                        this->is_valid_ = true;
                }
        };
}
#endif //MI_THREAD_SAFE_COUNTER_HPP
