//
// Some core parsers for classes of characters
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
#include "core/token_parsers.hpp"

namespace rpc
{
namespace basic
{
    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    inline core::parser<It, T, R> character (T && c)
    {
        return core::token<It, T, R> (std::forward<T>(c));
    }

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    inline core::parser<It, T, R> characters (T && c, std::size_t const n = 0)
    {
        return core::some (character<It, T, R>(std::forward<T>(c)), n);
    }

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, T, R> space =
        core::satisfy<It, T, R>
            ([](T const& c) -> bool { return std::isspace(c); },
            "whitespace");

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, T, R> spaces = core::some (space<It, T, R>);

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, T, R> spacem = core::many (space<It, T, R>);

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, T, R> wspace =
        core::satisfy<It, T, R>
            ([](T const& c) -> bool { return std::iswspace(c); },
            "wide whitespace");
 
    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>    
    core::parser<It, T, R> wspaces = core::some (wspace<It, T, R>);

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, T, R> wspacem = core::many (wspace<It, T, R>);
 
    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>   
    core::parser<It, T, R> punct =
        core::satisfy<It, T, R>
            ([](T const& c) -> bool { return std::ispunct(c); },
            "punctuation");

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, T, R> puncts = core::some (punct<It, T, R>);

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, T, R> punctm = core::many (punct<It, T, R>);

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, T, R> wpunct =
        core::satisfy<It, T, R>
            ([](T const& c) -> bool { return std::iswpunct(c); },
            "wide punctuation");
 
    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>    
    core::parser<It, T, R> wpuncts = core::some (wpunct<It, T, R>);

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, T, R> wpunctm = core::many (wpunct<It, T, R>);

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, T, R> alpha =
        core::satisfy<It, T, R>
            ([](T const& c) -> bool
                { return std::isalpha(c); },
            "alphabetic");
 
    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>    
    core::parser<It, T, R> alpham = core::many (alpha<It, T, R>); 

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, T, R> alphas = core::some (alpha<It, T, R>); 
 
    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>   
    core::parser<It, T, R> walpha =
        core::satisfy<It, T, R>
            ([](T const& c) -> bool { return std::iswalpha(c); },
            "wide alphabetic");

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, T, R> walphas = core::some (walpha<It, T, R>); 

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, T, R> walpham = core::many (walpha<It, T, R>); 

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, T, R> palpha =
        core::satisfy<It, T, R>
            ([](T const& c) -> bool
                { return std::isalpha(c) || std::ispunct(c); },
        "alphabetic or punctuation");
 
    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>    
    core::parser<It, T, R> palpham = core::many (alpha<It, T, R>); 

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, T, R> palphas = core::some (alpha<It, T, R>); 
 
    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>   
    core::parser<It, T, R> wpalpha =
        core::satisfy<It, T, R>
            ([](T const& c) -> bool
                { return std::iswalpha(c) || std::iswpunct(c); },
            "wide alphabetic or wide punctuation");

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, T, R> wpalphas = core::some (walpha<It, T, R>); 

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, T, R> wpalpham = core::many (walpha<It, T, R>); 

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, T, R> lower =
        core::satisfy<It, T, R>
            ([](T const& c) -> bool { return std::islower(c); },
            "lower-case");

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, T, R> lowers = core::some (lower<It, T, R>); 

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, T, R> lowerm = core::many (lower<It, T, R>); 

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, T, R> wlower =
        core::satisfy<It, T, R>
            ([](T const& c) -> bool { return std::iswlower(c); },
            "wide lower-case");
 
    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>> 
    core::parser<It, T, R> wlowers = core::some (wlower<It, T, R>); 

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, T, R> wlowerm = core::many (wlower<It, T, R>); 
 
    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>> 
    core::parser<It, T, R> upper =
        core::satisfy<It, T, R>
            ([](T const& c) -> bool { return std::isupper(c); },
            "upper-case");

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, T, R> uppers = core::some (upper<It, T, R>); 

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, T, R> upperm = core::many (upper<It, T, R>); 

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, T, R> wupper =
        core::satisfy<It, T, R>
            ([](T const& c) -> bool { return std::iswupper(c); },
            "wide upper-case");
 
    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>    
    core::parser<It, T, R> wuppers = core::some (wupper<It, T, R>); 
 
    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>   
    core::parser<It, T, R> wupperm = core::many (wupper<It, T, R>); 
 
    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>    
    core::parser<It, T, R> digit =
        core::satisfy<It, T, R>
            ([](T const& c) -> bool { return std::isdigit(c); },
            "digit character");

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, T, R> digits = core::some (digit<It, T, R>); 

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, T, R> digitm = core::many (digit<It, T, R>); 

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, T, R> wdigit =
        core::satisfy<It, T, R>
            ([](T const& c) -> bool { return std::iswdigit(c); },
            "wide digit character");
 
    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>    
    core::parser<It, T, R> wdigits = core::some (wdigit<It, T, R>); 
 
    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>   
    core::parser<It, T, R> wdigitm = core::many (wdigit<It, T, R>); 

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, T, R> hexdigit =
        core::satisfy<It, T, R>
            ([](T const& c) -> bool { return std::isxdigit(c); },
            "hex-digit character");

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, T, R> hexdigits = core::some (hexdigit<It, T, R>); 

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, T, R> hexdigitm = core::many (hexdigit<It, T, R>); 

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, T, R> whexdigit =
        core::satisfy<It, T, R>
            ([](T const& c) -> bool { return std::iswxdigit(c); },
            "wide hex-digit character");
 
    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>    
    core::parser<It, T, R> whexdigits = core::some (whexdigit<It, T, R>); 

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, T, R> whexdigitm = core::many (whexdigit<It, T, R>); 

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, T, R> cntrl =
        core::satisfy<It, T, R>
            ([](T const& c) -> bool { return std::iscntrl(c); },
            "control character");

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, T, R> cntrls = core::some (cntrl<It, T, R>); 

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, T, R> cntrlm = core::many (cntrl<It, T, R>); 

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, T, R> wcntrl =
        core::satisfy<It, T, R>
            ([](T const& c) -> bool { return std::iswcntrl(c); },
            "wide control character");
 
    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>    
    core::parser<It, T, R> wcntrls = core::some (wcntrl<It, T, R>); 

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, T, R> wcntrlm = core::many (wcntrl<It, T, R>); 

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, std::basic_string<T>, R> word =
        core::override_description
            (core::reducel (alphas<It, T, R>,
                          [](T c, std::basic_string<T> & s)
                                { s.push_back (c); return s; },
                          std::basic_string<T>()),
             "[word ::" + fnk::utility::type_name<T>::name() + "]");

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, std::basic_string<T>, R> words =
        core::some (rpc::core::ignorel (spacem<It, T, R>, word<It, T, R>));

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, std::basic_string<T>, R> wordm =
        core::many (rpc::core::ignorel (spacem<It, T, R>, word<It, T, R>));

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, std::basic_string<T>, R> pword =
        core::override_description
            (core::reducer (palphas<It, T, R>, 
                           [](T c, std::basic_string<T> & s)
                                { s.push_back (c); return s; },
                           std::basic_string<T>()),
             "[pword ::" + fnk::utility::type_name<T>::name() + "]");

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, std::basic_string<T>, R> pwords =
        core::some
            (core::ignorel (spacem<It, T, R>, pword<It, T, R>));

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, std::basic_string<T>, R> pwordm =
        core::many
            (core::ignorel
                (spacem<It, T, R>, pword<It, T, R>));

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, std::basic_string<T>, R> wword =
        core::override_description
            (core::reducer (walphas<It, T, R>,
                           [](T c, std::basic_string<T> & s)
                                { s.append (c); return s; },
                           std::basic_string<T>()),
             "[wide word ::" + fnk::utility::type_name<T>::name() + "]");

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, std::basic_string<T>, R> wwords =
        core::some
            (core::ignorel (wspacem<It, T, R>, wword<It, T, R>));

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, std::basic_string<T>, R> wwordm =
        core::many
            (core::ignorel (wspacem<It, T, R>, wword<It, T, R>));
 
    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>   
    core::parser<It, std::basic_string<T>, R> pwword =
        core::override_description
            (core::reducer (wpalphas<It, T, R>,
                           [](T c, std::basic_string<T> & s)
                                { s.append (c); return s; },
                           std::basic_string<T>()),
             "[wide pword ::" + fnk::utility::type_name<T>::name() + "]");

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, std::basic_string<T>, R> pwwords =
        core::some
            (core::ignorel (wspacem<It, T, R>, pwword<It, T, R>));

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    core::parser<It, std::basic_string<T>, R> pwwordm =
        core::many
            (core::ignorel (wspacem<It, T, R>, pwword<It, T, R>));
} // namespace basic
} // namespace rpc

#endif // ifndef TEXT_PARSERS
