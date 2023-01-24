/**
 * @file Argument.hpp
 * @author Takashi Michikawa <tmichi@me.com>
 * @copyright (c) 2007 -  Takashi Michikawa
 * Released under the MIT license
 * https://opensource.org/licenses/mit-license.php
 */
#ifndef MI_ARGUMENT_HPP
#define MI_ARGUMENT_HPP 1

#include <deque>
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <iostream>
#include <algorithm>
#include "sto.hpp"

namespace mi {
        class Argument {
        public:
                explicit Argument(const int argc = 0, const char **argv = nullptr) : argv_(argv, argv + argc) {}

                Argument(const std::initializer_list<std::string> &args) : argv_(args.begin(), args.end()) {}

                Argument(const Argument &) = delete;

                void operator=(const Argument &) = delete;

                ~Argument() = default;

                [[nodiscard]]
                inline size_t size() const {
                        return this->argv_.size();
                }
        
                [[nodiscard]]
                inline bool exist(const std::string &key, const size_t offset = 0) const {
                        return this->index(key) + offset < this->size();
                }

                template <typename T>
                inline T get(const std::string &key, const size_t offset = 1) const {
                        return this->get<T>(this->index(key) + offset);
                }
        
                
                template<typename T>
                [[nodiscard]]
                inline T get(const size_t idx) const {
                    try {
                        return mi::sto<T>(this->argv_.at(idx));
                    }
                    catch (const std::invalid_argument& e) {
                        std::cerr << "invalid argument in Argument::get() " << e.what() << std::endl;
                    }
                    catch (const std::out_of_range& e) {
                        std::cerr << "out of range at Argument::get() " << e.what() << std::endl;
                    }
                    catch (...) {
                        std::cerr << "Unknown error at Argument::get()." << std::endl;
                    }
             
                        return T();
                }

                /**
                 * @brief return the index of the key
                 * @param key Key value
                 * @return index where the key exists. return Argument::size() when the key does not exist.
                 */
                [[nodiscard]]
                inline size_t index(const std::string &key) const {
                        for (size_t i = 0 ; i < this->argv_.size() ; ++i ) {
                                if ( key == this->argv_[i]) {
                                        return i;
                                }
                        }
                        return this->argv_.size();
                }

        private:
                std::deque<std::string> argv_; ///< Arguments
        };
}
#endif //MI_ARGUMENT_HPP
