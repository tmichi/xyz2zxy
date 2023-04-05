//
// Created by Takashi Michikawa on 2023/02/10.
//

#ifndef MI_STREAM_HPP
#define MI_STREAM_HPP
#include <ostream>
#include <tuple>
#include <iterator>
// for tuple
template <typename T, typename... Ss>
std::ostream& operator<<(std::ostream& os, const std::tuple<T, Ss...>& v) {
        os << "(";
        std::apply([&os](const auto&... args) {((os << args << ", "), ...);}, v);
        os << "\b\b)";// 2文字バック(最後の", "を消すため)
        return os;
}
// for STL containers
template <typename Container, typename = typename Container::allocator_type,
        typename Container::value_type,
        typename Container::size_type,
        typename Container::allocator_type,
        typename Container::iterator,
        decltype(std::declval<Container>().end())
        >
std::ostream& operator << (std::ostream& os, const Container& v){
        os << std::string("{");
        if(std::size(v) >  0){
                std::copy(std::begin(v), std::end(v) - 1, std::ostream_iterator<typename Container::value_type>(os, ", "));
                os << *(std::prev(std::end(v)));
        }
        os << std::string("}");
        return os;
}



#endif //MI_STREAM_HPP
