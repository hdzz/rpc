//
// Some basic parsers for classes of characters
//
// Author: Dalton Woodard
// Contact: daltonmwoodard@gmail.com
// License: Please see LICENSE.md
//

#ifndef TEXT_PARSERS_HPP
#define TEXT_PARSERS_HPP

#include <cctype>
#include <iterator>
#include <list>
#include <string>

#include "core/range.hpp"
#include "core/parser.hpp"
#include "core/combinators.hpp"

#include "atom_parsers.hpp"

namespace rpc
{
namespace basic
{
    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    inline decltype(auto) character (T && c)
    {
        return rpc::basic::token<It, T, R> (std::forward<T>(c));
    }

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    inline decltype(auto) characters (T && c, std::size_t const n = 0)
    {
        return rpc::core::some (character<It, T, R>(std::forward<T>(c)), n);
    }

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto space =
        rpc::basic::satisfy<It, T, R>
            ([](T const& c) -> bool { return std::isspace(c); },
            "whitespace");

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto spaces = rpc::core::some (space<It, T, R>);

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto spacem = rpc::core::many (space<It, T, R>);

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto wspace =
        rpc::basic::satisfy<It, T, R>
            ([](T const& c) -> bool { return std::iswspace(c); },
            "wide whitespace");
 
    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>    
    auto wspaces = rpc::core::some (wspace<It, T, R>);

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto wspacem = rpc::core::many (wspace<It, T, R>);
 
    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>   
    auto punct =
        rpc::basic::satisfy<It, T, R>
            ([](T const& c) -> bool { return std::ispunct(c); },
            "punctuation");

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto puncts = rpc::core::some (punct<It, T, R>);

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto punctm = rpc::core::many (punct<It, T, R>);

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto wpunct =
        rpc::basic::satisfy<It, T, R>
            ([](T const& c) -> bool { return std::iswpunct(c); },
            "wide punctuation");
 
    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>    
    auto wpuncts = rpc::core::some (wpunct<It, T, R>);

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto wpunctm = rpc::core::many (wpunct<It, T, R>);

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto alpha =
        rpc::basic::satisfy<It, T, R>
            ([](T const& c) -> bool
                { return std::isalpha(c); },
            "alphabetic");
 
    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>    
    auto alpham = rpc::core::many (alpha<It, T, R>); 

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto alphas = rpc::core::some (alpha<It, T, R>); 
 
    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>   
    auto walpha =
        rpc::basic::satisfy<It, T, R>
            ([](T const& c) -> bool { return std::iswalpha(c); },
            "wide alphabetic");

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto walphas = rpc::core::some (walpha<It, T, R>); 

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto walpham = rpc::core::many (walpha<It, T, R>); 

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto palpha =
        rpc::basic::satisfy<It, T, R>
            ([](T const& c) -> bool
                { return std::isalpha(c) || std::ispunct(c); },
        "alphabetic or punctuation");
 
    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>    
    auto palpham = rpc::core::many (alpha<It, T, R>); 

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto palphas = rpc::core::some (alpha<It, T, R>); 
 
    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>   
    auto wpalpha =
        rpc::basic::satisfy<It, T, R>
            ([](T const& c) -> bool
                { return std::iswalpha(c) || std::iswpunct(c); },
            "wide alphabetic or wide punctuation");

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto wpalphas = rpc::core::some (walpha<It, T, R>); 

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto wpalpham = rpc::core::many (walpha<It, T, R>); 

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto lower =
        rpc::basic::satisfy<It, T, R>
            ([](T const& c) -> bool { return std::islower(c); },
            "lower-case");

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto lowers = rpc::core::some (lower<It, T, R>); 

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto lowerm = rpc::core::many (lower<It, T, R>); 

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto wlower =
        rpc::basic::satisfy<It, T, R>
            ([](T const& c) -> bool { return std::iswlower(c); },
            "wide lower-case");
 
    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>> 
    auto wlowers = rpc::core::some (wlower<It, T, R>); 

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto wlowerm = rpc::core::many (wlower<It, T, R>); 
 
    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>> 
    auto upper =
        rpc::basic::satisfy<It, T, R>
            ([](T const& c) -> bool { return std::isupper(c); },
            "upper-case");

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto uppers = rpc::core::some (upper<It, T, R>); 

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto upperm = rpc::core::many (upper<It, T, R>); 

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto wupper =
        rpc::basic::satisfy<It, T, R>
            ([](T const& c) -> bool { return std::iswupper(c); },
            "wide upper-case");
 
    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>    
    auto wuppers = rpc::core::some (wupper<It, T, R>); 
 
    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>   
    auto wupperm = rpc::core::many (wupper<It, T, R>); 
 
    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>    
    auto digit =
        rpc::basic::satisfy<It, T, R>
            ([](T const& c) -> bool { return std::isdigit(c); },
            "digit character");

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto digits = rpc::core::some (digit<It, T, R>); 

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto digitm = rpc::core::many (digit<It, T, R>); 

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto wdigit =
        rpc::basic::satisfy<It, T, R>
            ([](T const& c) -> bool { return std::iswdigit(c); },
            "wide digit character");
 
    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>    
    auto wdigits = rpc::core::some (wdigit<It, T, R>); 
 
    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>   
    auto wdigitm = rpc::core::many (wdigit<It, T, R>); 

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto hexdigit =
        rpc::basic::satisfy<It, T, R>
            ([](T const& c) -> bool { return std::isxdigit(c); },
            "hex-digit character");

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto hexdigits = rpc::core::some (hexdigit<It, T, R>); 

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto hexdigitm = rpc::core::many (hexdigit<It, T, R>); 

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto whexdigit =
        rpc::basic::satisfy<It, T, R>
            ([](T const& c) -> bool { return std::iswxdigit(c); },
            "wide hex-digit character");
 
    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>    
    auto whexdigits = rpc::core::some (whexdigit<It, T, R>); 

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto whexdigitm = rpc::core::many (whexdigit<It, T, R>); 

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto cntrl =
        rpc::basic::satisfy<It, T, R>
            ([](T const& c) -> bool { return std::iscntrl(c); },
            "control character");

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto cntrls = rpc::core::some (cntrl<It, T, R>); 

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto cntrlm = rpc::core::many (cntrl<It, T, R>); 

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto wcntrl =
        rpc::basic::satisfy<It, T, R>
            ([](T const& c) -> bool { return std::iswcntrl(c); },
            "wide control character");
 
    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>    
    auto wcntrls = rpc::core::some (wcntrl<It, T, R>); 

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto wcntrlm = rpc::core::many (wcntrl<It, T, R>); 

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto word =
        core::override_description
            (core::reduce (alphas<It, T, R>,
                          [](std::basic_string<T> & s, T c)
                                { s.push_back (c); return s; },
                          std::basic_string<T>()),
             "[word ::" + fnk::utility::type_name<T>::name() + "]");

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto words =
        rpc::core::some (rpc::core::ignorel (spacem<It, T, R>, word<It, T, R>));

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto wordm =
        rpc::core::many (rpc::core::ignorel (spacem<It, T, R>, word<It, T, R>));

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto pword =
        core::override_description
            (core::reduce (palphas<It, T, R>, 
                           [](std::basic_string<T> & s, T c)
                                { s.push_back (c); return s; },
                           std::basic_string<T>()),
             "[pword ::" + fnk::utility::type_name<T>::name() + "]");

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto pwords =
        rpc::core::some
            (rpc::core::ignorel (spacem<It, T, R>, pword<It, T, R>));

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto pwordm =
        rpc::core::many
            (rpc::core::ignorel
                (spacem<It, T, R>, pword<It, T, R>));

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto wword =
        core::override_description
            (core::reduce (walphas<It, T, R>,
                           [](std::basic_string<T> & s, T c)
                                { s.append (c); return s; },
                           std::basic_string<T>()),
             "[wide word ::" + fnk::utility::type_name<T>::name() + "]");

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto wwords =
        rpc::core::some
            (rpc::core::ignorel (wspacem<It, T, R>, wword<It, T, R>));

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto wwordm =
        rpc::core::many
            (rpc::core::ignorel (wspacem<It, T, R>, wword<It, T, R>));
 
    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>   
    auto pwword =
        core::override_description
            (core::reduce (wpalphas<It, T, R>,
                           [](std::basic_string<T> & s, T c)
                                { s.append (c); return s; },
                           std::basic_string<T>()),
             "[wide pword ::" + fnk::utility::type_name<T>::name() + "]");

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto pwwords =
        rpc::core::some
            (rpc::core::ignorel (wspacem<It, T, R>, pwword<It, T, R>));

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    auto pwwordm =
        rpc::core::many
            (rpc::core::ignorel (wspacem<It, T, R>, pwword<It, T, R>));
} // namespace basic
} // namespace rpc

#endif // ifndef TEXT_PARSERS

