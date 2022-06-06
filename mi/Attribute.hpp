/**
 * @file Attribute.hpp
 * @author Takashi Michikawa <tmichi@me.com>
 * @copyright (c) 2007 -  Takashi Michikawa
 * Released under the MIT license
 * https://opensource.org/licenses/mit-license.php
 */
#ifndef MI_ATTRIBUTE_HPP
#define MI_ATTRIBUTE_HPP 1
#include <filesystem>
#include "Argument.hpp"
#include <deque>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <list>
#include <string>
#include <type_traits>
#include <algorithm>

namespace mi {
        template<typename T> using attribute_getter_t = std::function<bool(const Argument &arg, const std::string &, T &)>;
        template<typename T> using validator_t = std::function<bool(const T &)>;

/**
 * @brief Attribute getter
 * @tparam T
 * @return  labmda function for get attributes.
 */
        template<typename T>
        inline auto attribute_getter() -> decltype(std::enable_if_t<std::is_arithmetic_v<T> || std::is_same_v<T, std::string > || std::is_same_v<T, std::filesystem::path> >(), attribute_getter_t<T>()) {
                if constexpr (std::is_same_v<T, bool>) {
                        return [](const Argument &arg, const std::string &key, T &value) {
                                value = arg.exist(key, 0);
                                return value;
                        };
                } else {
                        return [](const Argument &arg, const std::string &key, T &value) {
                                const bool exists = arg.exist(key, 1);
                                if( exists ) {
                                        value = arg.get<T>(key, 1);
                                }
                                return exists;
                        };
                }
        }
#ifdef EIGEN_MATRIX_H
        template<typename T>
        using is_eigen_t = typename std::enable_if<std::is_same_v<Eigen::Matrix<typename T::Scalar, T::RowsAtCompileTime, T::ColsAtCompileTime>, T>>::type;

        ///< @note If you use Eigen classes e.g. Vector3d, Please include <Eigen/Dense>
        ///< prior to this file.
        template<typename T>
        inline auto attribute_getter() -> decltype(is_eigen_t<T>(), attribute_getter_t<T>()) {
                return [](const Argument &arg, const std::string &key, T &value) {
                        for (Eigen::Index i = 0; i < value.cols(); ++i) {
                                for (Eigen::Index j = 0; j < value.rows(); ++j) {
                                        value(j, i) = arg.get<typename T::Scalar>(key, size_t(j + i * T::RowsAtCompileTime) + 1);
                                }
                        }
                        return arg.exist(key, size_t(value.rows() * value.cols()));
                };
        }
        namespace attr {
                template<typename T>
                inline auto greater(const T &lower) -> decltype(is_eigen_t<T>(), validator_t<T>()) {
                        return [lower](const T &value) { return (value - lower).minCoeff() > 0; };
                }

                template<typename T>
                inline auto less(const T &upper) -> decltype(is_eigen_t<T>(), validator_t<T>()) {
                        return [upper](const T &value) { return (upper - value).minCoeff() > 0; };
                }
        }
#endif // EIGEN_MATRIX_H

        namespace attr {
                template<typename T, typename Func>
                inline auto bind(const T &value) -> decltype(typename std::enable_if<std::is_arithmetic_v<T>, T>::type(), validator_t<T>()) {
                        return std::bind(Func(), std::placeholders::_1, value);
                }

                template<typename T>
                inline auto greater(const T &lower) -> decltype(typename std::enable_if<std::is_arithmetic_v<T>, T>::type(), validator_t<T>()) {
                        return mi::attr::bind<T, std::greater<T>>(lower);
                }

                template<typename T>
                inline auto less(const T &upper) -> decltype(typename std::enable_if<std::is_arithmetic_v<T>, T>::type(), validator_t<T>()) {
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


                template <typename T>
                inline auto func_not ( validator_t<T> f ) {
                        return [f](const T& v) { return !f(v);};
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
 * @brief Attrbiute template class
 * @tparam T Type of the attribute
 */
        template<typename T>
        class Attribute : public AttributeBase {
        private:
                std::string key_;      ///< Attribute key
                T &value_;             ///< A reference to stored value
                const T defaultValue_; ///< Default value
                bool isMandatory_;
                bool isInvalidRejected_;
                bool isQuiet_;
                std::string message_;
                validator_t<T> validator_;
                attribute_getter_t<T> getter_;

        public:
                /**
                 * @brief Constructor
                 * @param key Key string
                 * @param value The variable to be stored and the value is used for default
                 * value.
                 */
                explicit Attribute(const std::string &key, T &value) : AttributeBase(), key_(key), value_(value), defaultValue_(value), isMandatory_(false), isInvalidRejected_(false), isQuiet_(false), message_(std::string()),validator_([]([[maybe_unused]] const T &v) { return true; }), getter_(attribute_getter<T>()) {}

                ~Attribute() override = default;

                Attribute<T> &setMandatory() {
                        this->isMandatory_ = true;
                        return *this;
                }

                Attribute<T> &setMessage(const std::string &message) {
                        this->message_ = message;
                        return *this;
                }

                /**
                 * @param[in] validator Validator.  e.g. bool validator(T& v)
                 * @param[in] isInvalidRejected set true if parsing is failed when the value
                 * is not validated.
                 * @return The reference to myself.
                 * @note
                 */
                Attribute<T> &setValidator(validator_t<T> validator, bool isInvalidRejected = false) {
                        this->validator_ = validator;
                        this->isInvalidRejected_ = isInvalidRejected;
                        return *this;
                }

                Attribute<T> &setQuiet() {
                        this->isQuiet_ = true;
                        return *this;
                }

                [[nodiscard]] bool parse(const Argument &arg) const override {
                        try {
                                if (this->getter_(arg, this->key_, this->value_)) {
                                        if (!this->validator_(this->value_) && this->isInvalidRejected_) {
                                                throw std::runtime_error("invalid value");
                                        }
                                } else {
                                        if (this->isMandatory_) {
                                                throw std::runtime_error("key not found");
                                        }
                                        this->value_ = this->defaultValue_;
                                }
                        } catch (std::runtime_error &e) {
                                if (!isQuiet_) {
                                        std::cerr << e.what() << std::endl;
                                }
                                return false;
                        }
                        return true;
                }

                void printUsage() override {
                        std::cerr << "\t" << this->key_ << " : " << this->message_ << std::endl;
                }

                void printValues(std::ostream &out) override {
                        out << "\t[" << this->key_ << "] = " << this->value_ << std::endl;
                }
        };

/**
 * @brief Attribute set
 */
        class AttributeSet : public AttributeBase {
        private:
                bool isAnd_;
                std::list<std::unique_ptr<AttributeBase>> attr_;
        public:
                explicit AttributeSet(const bool isAnd = true) : AttributeBase(), isAnd_(isAnd) {}

                ~AttributeSet() override = default;

                template<typename T>
                Attribute<T> &createAttribute(const std::string &key, T &value) {
                        this->attr_.emplace_back(std::unique_ptr<AttributeBase>(new Attribute<T>(key, value)));
                        return dynamic_cast<Attribute<T> &>(*(this->attr_.back()));
                }

                AttributeSet &createAttributeSet() {
                        this->attr_.emplace_back(std::unique_ptr<AttributeBase>(new AttributeSet()));
                        return dynamic_cast<AttributeSet &>(*(this->attr_.back()));
                }

                [[nodiscard]] bool parse(const Argument &arg) const override {
                        auto fn = [&arg](auto& iter) {return iter->parse(arg);};
                        auto& attr = this->attr_;
                        return this->isAnd_ ? std::all_of(attr.begin(), attr.end(), fn) : std::any_of(attr.begin(), attr.end(), fn);
                }

                void printUsage() override {
                        std::for_each(this->attr_.begin(), this->attr_.end(), [](auto &iter) { iter->printUsage(); });
                }

                void printValues(std::ostream &out) override {
                        std::for_each(this->attr_.begin(), this->attr_.end(), [&out](auto &iter) { iter->printValues(out); });
                }
        };
} // namespace mi
#endif // MI_ATTRIBUTE_HPP