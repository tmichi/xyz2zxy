//
// Created by Takashi Michikawa on 2022/10/05.
//

#ifndef XYZ2ZXY_ATTRIBUTETUPLE_HPP
#define XYZ2ZXY_ATTRIBUTETUPLE_HPP
#include <mi/Argument.hpp>
std::ostream& operator << (std::ostream& os, std::tuple<double, double>& v) {
        os << "(" << std::get<0>(v) << ", " << std::get<1>(v) << ")" << std::endl;
        return os;
}
namespace mi {
        class Argument;
        template<typename T>
        inline auto attribute_getter() -> decltype(std::enable_if_t<std::is_same_v<T, std::tuple<double, double> > >(),
                std::function<bool(const Argument &arg, const std::string &, T &)>()){
                return [](const Argument& arg, const std::string& key, std::tuple<double, double>& value) {
                        if (arg.exist(key, 2)) {
                                value = std::tuple<double, double>(arg.get<double>(key, 1), arg.get<double>(key, 2));
                                return true;
                        } else {
                                return false;
                        }
                };
        }
}
#include <mi/Attribute.hpp>
#endif //XYZ2ZXY_ATTRIBUTETUPLE_HPP
