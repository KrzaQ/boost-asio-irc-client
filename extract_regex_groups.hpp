#ifndef REGEX_EXPLODE_HPP
#define REGEX_EXPLODE_HPP

#include <regex>
#include <string>
#include <tuple>
#include <type_traits>

#include <boost/lexical_cast.hpp>

namespace kq
{

namespace detail
{

template<typename Tuple, size_t... Is>
void assign_regex_matches(
    Tuple&& tup,
    std::cmatch const& match,
    std::index_sequence<Is...>
) {
    using tuple_t = std::decay_t<Tuple>;
    (
    (void) (
        std::get<Is>(tup) =
            boost::lexical_cast<
                std::decay_t<
                    std::tuple_element_t<Is, tuple_t>
                >
            >(match[1+Is])
    ), ...
    );
}

}

template<typename... Ts, typename Tuple>
void extrace_regex_groups(
    char const* string,
    std::regex const& regex,
    Tuple&& tuple
) {
    constexpr auto size = std::tuple_size<Tuple>{}();

    std::cmatch match;
    std::regex_search(string, match, regex);

    if(match.size() != size+1) {
        throw std::runtime_error("Wrong number of captures");
    }

    detail::assign_regex_matches(
        std::forward<Tuple>(tuple),
        match,
        std::make_index_sequence<size>{}
    );
}

}


#endif // REGEX_EXPLODE_HPP
