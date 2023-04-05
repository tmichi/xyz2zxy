/**
 * @file Argument.hpp
 * @author Takashi Michikawa <tmichi@me.com>
 * @copyright (c) 2007 -  Takashi Michikawa
 * Released under the MIT license
 * https://opensource.org/licenses/mit-license.php
 */
#ifndef MI_ARGUMENT_HPP
#define MI_ARGUMENT_HPP

#include <algorithm>
#include <initializer_list>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace mi {
        /**
         * @brief A class for handling command line arguments.
         */
        class Argument {
        public:
                /**
                 * @brief Constructor.
                 * @param argc Number of arguments.
                 * @param argv Array of arguments.
                 */
                explicit Argument(int argc = 0, char **argv = nullptr)
                        : argv_(argv, argv + argc) {}

                /**
                 * @brief Constructor.
                 * @param args A list of arguments.
                 */
                Argument(const std::initializer_list<std::string> &args)
                        : argv_(args.begin(), args.end()) {}

                Argument(const Argument &) = delete;

                void operator=(const Argument &) = delete;

                ~Argument() = default;

                /**
                 * @brief Get the number of arguments.
                 * @return The number of arguments.
                 */
                [[nodiscard]] inline size_t size() const {
                        return this->argv_.size();
                }

                /**
                 * @brief Check if the specified key exists.
                 * @param key The key to check.
                 * @param offset The offset.
                 * @return True if the specified key exists, false otherwise.
                 */
                [[nodiscard]] inline bool contains(const std::string &key, const size_t offset = 0) const {
                        return this->index(key) + offset < this->size();
                }
                /**
                 * @brief Check if the specified key or the second specified key exists.
                 * @param key The key to check.
                 * @param key2nd The second key to check.
                 * @param offset The offset.
                 * @return True if the specified key or the second specified key exists, false otherwise.
                 */
                [[nodiscard]] bool contains(const std::string &key, const std::string &key2nd, const size_t offset = 0) const {
                        return this->contains(key, offset) || this->contains(key2nd, offset);
                }
        
                /**
                 * @brief Check if the specified key exists.
                 * @param key The key to check.
                 * @param offset The offset.
                 * @return True if the specified key exists, false otherwise.
                 */
                [[nodiscard]] bool exist(const std::string &key, const size_t offset = 0) const {
                        return this->index(key) + offset < this->size();
                }
                

                /**
                 * @brief Get the value of the specified key.
                 * @tparam T The type of the value.
                 * @param key The key to get.
                 * @param offset The offset.
                 * @return The value of the specified key.
                 */
                template<typename T>
                [[nodiscard]] T get(const std::string &key,
                                    const size_t offset = 1) const {
                        return this->get<T>(this->index(key) + offset);
                }

                /**
                 * @brief Get the value of the specified index.
                 * @tparam T The type of the value.
                 * @param idx The index.
                 * @return The value of the specified index.
                 */
                template<typename T>
                [[nodiscard]] T get(const size_t idx) const noexcept {
                        try {
                                return this->sto<T>(this->argv_.at(idx));
                        } catch (const std::out_of_range &e) {
#if defined(DEBUG)
                                std::cerr << "Invalid index " << std::endl
#endif
                        } catch (const std::invalid_argument &e) {
                                std::cerr << "Conversion error. " << std::endl;
                        } catch (const std::exception &e) {
                                std::cerr << "Exception at Argument::get<>() :" << e.what()
                                          << std::endl;
                        } catch (...) {
                                std::cerr << "Unexpected error." << std::endl;
                        }
                        return T();
                }

                /**
                 * @brief Get the index of the specified key.
                 * @param key The key to get the index of.
                 * @return The index of the specified key.
                 */
                [[nodiscard]] size_t index(const std::string &key) const {
                        return size_t(std::distance(this->argv_.begin(),std::find(this->argv_.begin(), this->argv_.end(), key)));
                }

        private:
                /**
                 * @brief Convert a string to the specified type.
                 * @tparam T The type to convert the string to.
                 * @param str The string to convert.
                 * @return The string converted to the specified type.
                 */
                template<typename T>
                [[nodiscard]] T sto(const std::string &str) const noexcept {
                        if constexpr (std::is_same_v<T, long double>)
                                return std::stold(str);
                        else if constexpr (std::is_same_v<T, double>)
                                return std::stod(str);
                        else if constexpr (std::is_same_v<T, float>)
                                return std::stof(str);
                        else if constexpr (std::is_floating_point_v<T>)
                                return T(std::stod(str));
                        else if constexpr (std::is_same_v<T, uint64_t>)
                                return std::stoull(str);
                        else if constexpr (std::is_same_v<T, int64_t>)
                                return std::stoll(str);
                        else if constexpr (std::is_unsigned_v<T>)
                                return T(std::stoul(str));
                        else if constexpr (std::is_signed_v<T>)
                                return T(std::stol(str));
                        else
                                return str;
                }
        private:
                std::vector<std::string> argv_;  ///< Arguments
        };
}
#endif //MI_ARGUMENT_HPP
