//
// A tiny parser library using `funktional`
//
// Author: Dalton Woodard
// Contact: daltonmwoodard@gmail.com
// License: Please see LICENSE.md
//

#include <iostream>
#include <string>
#include <type_traits>
#include <utility>

#include "foldable.hpp"
#include "parser.hpp"
#include "zip.hpp"

#include "utility/bool_utils.hpp"

namespace fnk
{
namespace parse
{
    template <typename T> 
    using iter_type = typename std::list<T>::const_iterator;
    
    template <typename V, typename T = V>
    using listparser = parser<V, iter_type<T>>;
}
}

template <char ... cs>
fnk::parse::listparser<char> one_of =
    fnk::parse::satisfy<fnk::parse::iter_type<char>> ([](char a) { return fnk::utility::any_true((a == cs)...); });

template <char c>
fnk::parse::listparser<char> character = one_of <c>;

auto whitespace = one_of <' ','\t','\n','\r'>;

auto alpha = fnk::parse::satisfy<fnk::parse::iter_type<char>> ([](char a) -> bool { return std::isalpha(a); });

auto alphas = fnk::parse::some (alpha);

auto lower = fnk::parse::satisfy<fnk::parse::iter_type<char>> ([](char a) -> bool { return std::islower(a); });

auto upper = fnk::parse::satisfy<fnk::parse::iter_type<char>> ([](char a) -> bool { return std::islower(a); });

auto is_digit = fnk::parse::satisfy<fnk::parse::iter_type<char>> ([](char a) { return '0' <= a && a <= '9'; });

fnk::parse::listparser<std::string, char> word = 
{
    .parse = [] (typename fnk::parse::listparser<char>::range_type const& r)
    {
        using AP = std::decay_t<decltype(alphas)>;
        using OT = std::list<typename fnk::parse::listparser<std::string, char>::result_type>;
        
        auto l = fnk::eval (alphas.parse, r);
        if (AP::is_value_result (l.front())) {
            auto ls = AP::values (l);
            return OT { std::make_pair (std::string(ls.cbegin(), ls.cend()), AP::result_range(l.back())) }; 
        } else
            return OT { fnk::parse::detail::failure{"expected word"} };
    }
};

auto digit = fnk::functor<fnk::parse::listparser<char>>::fmap ([](char c) -> int { return c - '0'; }, fnk::parse::some (is_digit));

fnk::parse::listparser<int, char> natural =
{
    .parse = [] (typename fnk::parse::listparser<char>::range_type const& r)
    {
        using DP = std::decay_t<decltype(digit)>;
        using OT = std::list<DP::result_type>;
        
        auto l = fnk::eval (digit.parse, r);
        if (DP::is_value_result (l.front())) {
            auto vs = DP::values (l);
            return OT
            { std::make_pair
                    (fnk::foldl ([p = vs.size()-1](int a, int b) mutable { return a + (b * std::pow(10, p--)); },
                                 0, 
                                 vs),
                     DP::result_range(l.back())) };
        } else
            return OT { fnk::parse::detail::failure{"expected natural"} };
    }
};

// these are similar parsers that should both match everyting in
// `parse_text_chars`
auto as_bs1 = fnk::parse::sequence (fnk::parse::some(character<'a'>), fnk::parse::some(character<'b'>));
auto as_bs2 = fnk::parse::some (one_of<'a','b'>);

auto whitespaces = fnk::parse::many (whitespace);

auto digits   = fnk::parse::some (fnk::parse::ignorel(whitespaces, digit));
auto naturals = fnk::parse::some (fnk::parse::ignorel(whitespaces, natural));
auto words    = fnk::parse::some (fnk::parse::ignorel(whitespaces, word));

static auto const parse_text_chars = std::list<char> { 'a', 'a', 'b', 'b', 'b' };
static auto const parse_text_ws    = std::list<char> { ' ', '\n', '\t', ' ', '\r' };
static auto const parse_text_nats  = std::list<char> { '1', '2', '3', '4', '5', ' ', '4', '2', ' ', '1', '0', '0', '1' };
static auto const parse_text_words = std::list<char>
{
    't','h','e',' ',
    'q','u','i','c','k',' ',
    'b','r','o','w','n',' ',
    'f','o','x',' ',
    'j','u','m','p','e','d',' ',
    'o','v','e','r',' ',
    't','h','e',' ',
    'l','a','z','y',' ',
    'd','o','g'
};

int main (void)
{
    std::cout << "characters from as_bs1:" << std::endl; 
    for (auto const& e : fnk::eval (as_bs1.parse, parse_text_chars))
        if (fnk::parse::listparser<char>::is_result (e))
            std::cout << '\t' << fnk::parse::listparser<char>::result_value (e) << std::endl;
    
    std::cout << "characters from as_bs2:" << std::endl; 
    for (auto const& e : fnk::eval (as_bs2.parse, parse_text_chars))
        if (fnk::parse::listparser<char>::is_result (e))
            std::cout << '\t' << fnk::parse::listparser<char>::result_value (e) << std::endl;

    auto wsname = [] (char c)
    {
        if (c == ' ') return std::string("space");
        else if (c == '\n') return std::string("newline");
        else if (c == '\t') return std::string("tab");
        else if (c == '\r') return std::string("carriage return");
        else return std::string("not whitespace"); 
    };

    std::cout << "whitespace:" << std::endl; 
    for (auto const& e : fnk::eval (whitespaces.parse, parse_text_ws))
        if (fnk::parse::listparser<char>::is_result (e))
            std::cout << '\t' << fnk::eval (wsname, fnk::parse::listparser<char>::result_value (e)) << std::endl;

    std::cout << "digits:" << std::endl;
    for (auto& e : fnk::eval (digits.parse, parse_text_nats))
        if (fnk::parse::listparser<int, char>::is_result (e))
            std::cout << '\t' << fnk::parse::listparser<int, char>::result_value(e) << std::endl;
   
    std::cout << "numbers:" << std::endl;
    for (auto const& e : fnk::eval (naturals.parse, parse_text_nats))
        if (fnk::parse::listparser<int, char>::is_result (e))
            std::cout << '\t' << fnk::parse::listparser<int, char>::result_value(e) << std::endl;
 
    std::cout << "words:" << std::endl;
    for (auto const& e : fnk::eval (words.parse, parse_text_words))
        if (fnk::parse::listparser<std::string, char>::is_result (e))
            std::cout << '\t' << fnk::parse::listparser<std::string, char>::result_value(e) << std::endl;
    
    std::exit (0);
}

