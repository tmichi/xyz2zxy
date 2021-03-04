//
// Created by Takashi Michikawa on 2020/10/17.
//

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
        public:
                /**
                 * @brief Constructor.
                 * @param s Init value.
                 */
                thread_safe_counter(const T s = T()) : n_(s) {}

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
                        static std::mutex mtx;
                        std::lock_guard <std::mutex> lock(mtx);
                        if ( this->n_ == std::numeric_limits<T>::max() ) {
                                throw std::runtime_error("mi::thread_safe_counter<T>::get() : max value");
                        }
                        return this->n_++;
                }

                /**
                 * @brief Reset counter.
                 * @param s reset value.
                 */
                void reset(const T s = T()) {
                        this->n_ = s;
                }
        };
}
#endif //MI_THREAD_SAFE_COUNTER_HPP
