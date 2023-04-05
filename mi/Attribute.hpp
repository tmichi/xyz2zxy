/**
 * @file Attribute.hpp
 * @author Takashi Michikawa <tmichi@me.com>
 * @copyright (c) 2007 -  Takashi Michikawa
 * Released under the MIT license
 * https://opensource.org/licenses/mit-license.php
 */
#ifndef MI_ATTRIBUTE_HPP
#define MI_ATTRIBUTE_HPP 1
#include <stack>
#include <deque>
#include <filesystem>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <list>
#include <string>
#include <type_traits>
#include <algorithm>
#include <utility>
#include "Argument.hpp"
namespace mi {
        /**
         * @brief Attribute getter type.
         */
        template<typename T> using attribute_getter_t = std::function<bool(const Argument &arg, const std::string &,T &)>;
        /**
         * @brief Validator.
         */
        template<typename T> using validator_t = std::function<bool(const T &)>;

/**
 * @brief Attribute getter
 * @tparam T
 * @return  lambda function for get attributes.
 */
        template<typename T>
        inline auto attribute_getter() -> decltype(std::enable_if_t<std::is_arithmetic_v<T> || std::is_same_v<T, std::string > || std::is_same_v<T, std::filesystem::path> >(), attribute_getter_t<T>()) {
                return [](const Argument &arg, const std::string &key, T &value) {
                        if constexpr (std::is_same_v<T, bool>) { // bool
                                value = arg.exist(key, 0);
                                return arg.exist(key, 0);
                        } else {// numeric
                                value = arg.get<T>(key, 1);
                                return arg.exist(key, 1);
                        }
                };
        }

#ifdef EIGEN_MATRIX_H
/**
 * @breif Attribute getter for Eigen::Matrix.
 */
        template<typename T>
        using is_eigen_t = typename std::enable_if<std::is_same_v<Eigen::Matrix<typename T::Scalar, T::RowsAtCompileTime, T::ColsAtCompileTime>, T>>::type;
/**
 * @brief Attribute getter for Eigen::Matrix.
 * @tparam T Eigen::Matrix
 * @return lambda function for get attributes.
 */
        template<typename T>
        inline auto attribute_getter() -> decltype(is_eigen_t<T>(), attribute_getter_t<T>()) {
                return [](const Argument &arg, const std::string &key, T &value) {
                        for ( size_t i = 1; auto &v : value.reshaped()) {
                                v = arg.get<typename T::Scalar>(key,i++);
                        }
                        return arg.exist(key, T::RowsAtCompileTime * T::ColsAtCompileTime );
                };
        }

        namespace attr {
                /**
                 * @brief Validator for Eigen::Matrix.
                 * @tparam T
                 * @param lower
                 * @return lambda function for validate attributes.
                 */
                template<typename T>
                inline auto greater(const T &lower) -> decltype(is_eigen_t<T>(), validator_t<T>()) {
                        return [lower](const T &value) { return (value - lower).minCoeff() > 0; };
                }
/**
 *  @brief Validator for Eigen::Matrix.
 */
                template<typename T>
                inline auto less(const T &upper) -> decltype(is_eigen_t<T>(), validator_t<T>()) {
                        return [upper](const T &value) { return (upper - value).minCoeff() > 0; };
                }
        }
#endif // EIGEN_MATRIX_H

#ifndef IS_TUPLE_DEFINED
#define IS_TUPLE_DEFINED 1
        /**
         * @brief Check if T is std::tuple.
         * @tparam T type to check.
         */
        template <typename T> struct is_tuple : std::false_type {};
        template <typename... Ts> struct is_tuple<std::tuple<Ts...>> : std::true_type {};
#endif//IS_TUPLE_DEFINED
        /**
         * @brief Attribute getter for std::tuple.
         * @tparam T std::tuple
         * @tparam Is std::index_sequence
         * @param tuple
         * @param arg
         * @param idx
         */
        template <typename T, std::size_t... Is>
        void set_tuple_from_array(T& tuple, const Argument& arg, const size_t idx,  std::index_sequence<Is...>) {
                std::apply([&arg, &idx](auto&... elems) {
                        ((void)(std::tie(elems) = std::make_tuple(arg.get<std::tuple_element_t<Is, T>>(idx + Is))), ...);
                }, tuple);
        }
        /**
         * @brief Attribute getter for std::tuple.
         * @tparam T tuple.
         * @return lambda function for get attributes.
         */
        template <typename T>
        inline auto attribute_getter() -> decltype(std::enable_if_t<is_tuple<T>::value>(), attribute_getter_t<T>()) {
                return [](const Argument &arg, const std::string& key, T& t) {
                        if ( !arg.exist(key, std::tuple_size_v<T>) ){
                                return false;
                        } else {
                                set_tuple_from_array(t, arg, arg.index(key)+1,std::make_index_sequence<std::tuple_size_v<T>>());
                                return true;
                        }
                };
        }
        
        namespace attr {
                template<typename T, typename Func>
                inline auto
                bind(const T &value) -> decltype(typename std::enable_if<std::is_arithmetic_v<T>, T>::type(), validator_t<T>()) {
                        return std::bind(Func(), std::placeholders::_1, value);
                }

                template<typename T>
                inline auto
                greater(const T &lower) -> decltype(typename std::enable_if<std::is_arithmetic_v<T>, T>::type(), validator_t<T>()) {
                        return mi::attr::bind<T, std::greater<T>>(lower);
                }

                template<typename T>
                inline auto
                less(const T &upper) -> decltype(typename std::enable_if<std::is_arithmetic_v<T>, T>::type(), validator_t<T>()) {
                        return mi::attr::bind<T, std::less<T>>(upper);
                }

                template<typename T>
                inline auto greater_equal(const T &lower) -> decltype(typename std::enable_if<std::is_arithmetic_v<T>, T>::type(), validator_t<T>()) {
                        return mi::attr::bind<T, std::greater_equal<T>>(lower);
                }

                template<typename T>
                inline auto less_equal(const T &upper) -> decltype(typename std::enable_if<std::is_arithmetic_v<T>, T>::type(), validator_t<T>()) {
                        return mi::attr::bind<T, std::less_equal<T>>(upper);
                }

                template<typename T>
                inline auto func_and(validator_t<T> f0, validator_t<T> f1) {
                        return [f0, f1](const T &v) { return f0(v) && f1(v); };
                }

                template<typename T>
                inline auto func_or(validator_t<T> f0, validator_t<T> f1) {
                        return [f0, f1](const T &v) { return f0(v) || f1(v); };
                }


                template<typename T>
                inline auto func_not(validator_t<T> f) {
                        return [f](const T &v) { return !f(v); };
                }

                template<typename T>
                inline auto between_equal(const T &lower, const T &upper) {
                        return mi::attr::func_and<T>(mi::attr::greater_equal<T>(lower), mi::attr::less_equal<T>(upper));
                }

                template<typename T>
                inline auto between(const T &lower, const T &upper) {
                        return mi::attr::func_and<T>(mi::attr::greater<T>(lower), mi::attr::less<T>(upper));
                }
        } // namespace attr

/**
 * * @brief Base class for Attribute<> and AttributeSet
 */
        class AttributeBase {
        public:
                AttributeBase() = default;

                AttributeBase(const AttributeBase &that) = delete;

                AttributeBase(AttributeBase &&that) = delete;

                void operator=(const AttributeBase &that) = delete;

                void operator=(AttributeBase &&that) = delete;

                virtual ~AttributeBase() = default;

                [[nodiscard]] virtual bool parse(const Argument &arg) const = 0;

                virtual void printUsage() = 0;

                virtual void printValues(std::ostream &out) = 0;
        };

/**
 * @brief Attribute template class
 * @tparam T Type of the attribute
 * @note If you create a custom Attribute (e.g. std::tuple<int, int>), you may require
 *  - operator <<(std::ostream& os, T& v )
 *  - auto attribute_getter() -> decltype(std::enable_if_t<std::is_same_v<T, std::tuple<int, int> >(), attribute_getter_t<T>());
 */
        template<typename T>
        class Attribute : public AttributeBase {
        private:
                std::string primaryKey_;      ///< Attribute key
                std::string secondaryKey_; ///< Attribute key
                T &value_;             ///< A reference to stored value
                const T defaultValue_; ///< Default value
                bool isMandatory_; ///< If true, the attribute is mandatory
                bool isInvalidRejected_; ///< If true, the attribute is rejected if the value is invalid.
                bool isQuiet_; ///< If true, the attribute is not printed in printValues().
                std::string message_; ///< Message for the attribute.
                validator_t<T> validator_; ///< Validator function.
                attribute_getter_t<T> getter_; ///< Getter function.

        public:
                /**
                 * @brief Constructor.
                 * @param key Primary key.
                 * @param second_key Secondary key.
                 * @param value Reference to the value.
                 */
                explicit Attribute(std::string key, std::string second_key, T &value) : AttributeBase(), primaryKey_(key), secondaryKey_(second_key), value_(value), defaultValue_(value), isMandatory_(false), isInvalidRejected_(false), isQuiet_(false), message_(std::string()), validator_([]([[maybe_unused]] const T &v) { return true; }), getter_(attribute_getter<T>()) {}
                /**
                 * @brief Constructor.
                 * @param key Primary key.
                 * @param value Reference to the value.
                 */
                explicit Attribute(std::string key, T &value) : AttributeBase(), primaryKey_(key), secondaryKey_(key), value_(value), defaultValue_(value), isMandatory_(false), isInvalidRejected_(false), isQuiet_(false), message_(std::string()), validator_([]([[maybe_unused]] const T &v) { return true; }), getter_(attribute_getter<T>()) {}
                
                ~Attribute() override = default;
                /**
                 * @brief Parse the argument.
                 * @return Reference to myself.
                 */
                Attribute<T> &setMandatory() {
                        this->isMandatory_ = true;
                        return *this;
                }
                /**
                 * @brief Set the default value.
                 * @param message
                 * @return Reference to myself.
                 */
                Attribute<T> &setMessage(const std::string &message) {
                        this->message_ = message;
                        return *this;
                }

                /**
                 * @param[in] validator Validator.  e.g. bool validator(T& v)
                 * @param[in] isInvalidRejected set true if parsing is failed when the value is not validated.
                 * @return Reference to myself.
                 */
                Attribute<T> &setValidator(validator_t<T> validator, bool isInvalidRejected = false) {
                        this->validator_ = validator;
                        this->isInvalidRejected_ = isInvalidRejected;
                        return *this;
                }

                /**
                 * @brief Set the default value.
                 * @param isQuiet If true, the attribute is not printed in printValues().
                 * @return  Reference to myself.
                 */
                Attribute<T> &setQuiet(const bool isQuiet = true) {
                        this->isQuiet_ = isQuiet;
                        return *this;
                }
                /**
                 * @brief Set the default value.
                 * @param arg Argument
                 * @retval true Success.
                 * @retval false Failure.
                 */
                [[nodiscard]] bool parse(const Argument &arg) const override {
                        try {
                                if (this->getter_(arg, this->primaryKey_, this->value_)) {
                                        if (!this->validator_(this->value_) && this->isInvalidRejected_) {
                                                throw std::runtime_error("invalid value");
                                        }
                                } else if (this->getter_(arg, this->secondaryKey_, this->value_) ) {
                                        if (!this->validator_(this->value_) && this->isInvalidRejected_) {
                                                throw std::runtime_error("invalid value");
                                        }
                                } else if (this->isMandatory_) {
                                        throw std::runtime_error("key not found");
                                } else {
                                        this->value_ = this->defaultValue_;
                                }
                        } catch (std::runtime_error &e) {
                                if (!this->isQuiet_) {
                                        std::cerr << e.what() << std::endl;
                                }
                                return false;
                        } catch (...) {
                                std::cerr<<"Unknown error in Attribute<T>::parse()"<<std::endl;
                                return false;
                        }
                        return true;
                }

                /**
                 * @brief Print the usage of the attribute.
                 */
                void printUsage() override {
                        std::cerr << "\t" << this->primaryKey_;
                        if (this->primaryKey_ != this->secondaryKey_){
                                std::cerr<<", "<<this->secondaryKey_;
                        }
                        std::cerr << " : " << this->message_ << std::endl;
                }
                /**
                 * @brief Print the value of the attribute.
                 * @param out Output stream.
                 */
                void printValues(std::ostream &out) override {
                        out << "\t[" << this->primaryKey_;
                        if (this->primaryKey_ != this->secondaryKey_) {
                                out << ", " << this->secondaryKey_;
                        }
                        out<< "] = " << this->value_ << std::endl;
                }
        };
        /**
         * @brief Attribute set class.
         * @note This class is used to group attributes.
         */
        class AttributeSet : public AttributeBase {
        private:
                bool isAnd_; ///< If true, the attribute set is AND.
                std::list<std::unique_ptr<AttributeBase>> attr_; ///< Attribute list.
        public:
                /**
                 * @brief Constructor.
                 * @param isAnd  If true, the attribute set is AND.
                 */
                explicit AttributeSet(const bool isAnd = true) : AttributeBase(), isAnd_(isAnd) {}

                ~AttributeSet() override = default;

                /**
                 * @brief Parse the argument.
                 * @tparam T  Type of the value.
                 * @param key  Primary key.
                 * @param value  Reference to the value.
                 * @return Reference to myself.
                 */
                template<typename T>
                Attribute<T> &createAttribute(const std::string &key, T &value) {
                        return dynamic_cast<Attribute<T> &>(*(*(this->attr_.insert(this->attr_.end(), std::unique_ptr<AttributeBase>(new Attribute<T>(key, value)))))).setQuiet(this->isAnd_);
                }
                /**
                 * @brief Parse the argument.
                 * @tparam T  Type of the value.
                 * @param primaryKey  Primary key.
                 * @param secondaryKey  Secondary key.
                 * @param value  Reference to the value.
                 * @return  Reference to myself.
                 */
                template<typename T>
                Attribute<T> &createAttribute(const std::string &primaryKey, const std::string secondaryKey, T &value) {
                        return dynamic_cast<Attribute<T> &>(*(*(this->attr_.insert(this->attr_.end(), std::unique_ptr<AttributeBase>(new Attribute<T>(primaryKey, secondaryKey, value)))))).setQuiet(this->isAnd_);
                }

                /**
                 * @brief Create a new attribute set.
                 * @param isAnd  If true, the attribute set is AND.
                 * @return  Reference to the new attribute set.
                 */
                [[nodiscard]] AttributeSet &createAttributeSet(bool isAnd = true) {
                        return dynamic_cast<AttributeSet &>(*(*(this->attr_.insert(this->attr_.end(), std::unique_ptr<AttributeBase>(new AttributeSet(isAnd))))));
                }

                /**
                 * @brief Parse the argument.
                 * @param arg Argument.
                 * @retval true Success.
                 * @retval false Failure.
                 */
                [[nodiscard]] bool parse(const Argument &arg) const override {
                        if (const auto count = std::count_if (this->attr_.begin(), this->attr_.end(), [&arg](auto& iter){return iter->parse(arg);}); this->isAnd_) {
                                return count == static_cast<long>(this->attr_.size());
                        } else {
                                return count > 0 ;
                        }
                }
                /**
                 * @brief Print the usage of the attribute set.
                 */
                void printUsage() override {
                        std::for_each(this->attr_.begin(), this->attr_.end(), [](auto &iter) { iter->printUsage(); });
                }

                /**
                 * @brief Print the value of the attribute set.
                 * @param out  Output stream.
                 */
                void printValues(std::ostream &out) override {
                        std::for_each(this->attr_.begin(), this->attr_.end(), [&out](auto &iter) { iter->printValues(out); });
                }
        };
} // namespace mi
#endif // MI_ATTRIBUTE_HPP