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
    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    inline constexpr decltype(auto) character (CharT && c)
    {
        return rpc::basic::token<It, CharT> (std::forward<CharT>(c));
    }

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    inline constexpr decltype(auto) characters (CharT && c, std::size_t const n = 0)
    {
        return n == 0 ? rpc::core::some (character<It, CharT>(std::forward<CharT>(c)))
                      : rpc::core::some (character<It, CharT>(std::forward<CharT>(c)), n);
    }

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto space = rpc::basic::satisfy<It> ([](CharT const& c) -> bool { return std::isspace(c); });

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto spaces = rpc::core::some (space<It, CharT>);

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto spacem = rpc::core::many (space<It, CharT>);

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto wspace = rpc::basic::satisfy<It> ([](CharT const& c) -> bool { return std::iswspace(c); });
     
    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto wspaces = rpc::core::some (wspace<It, CharT>);

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto wspacem = rpc::core::many (wspace<It, CharT>);

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto alpha = rpc::basic::satisfy<It> ([](CharT const& c) -> bool { return std::isalpha(c); });
     
    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto alpham = rpc::core::many (alpha<It, CharT>); 

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto alphas = rpc::core::some (alpha<It, CharT>); 
    
    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto walpha = rpc::basic::satisfy<It> ([](CharT const& c) -> bool { return std::iswalpha(c); });

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto walphas = rpc::core::some (walpha<It, CharT>); 

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto walpham = rpc::core::many (walpha<It, CharT>); 

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto lower = rpc::basic::satisfy<It> ([](CharT const& c) -> bool { return std::islower(c); });

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto lowers = rpc::core::some (lower<It, CharT>); 

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto lowerm = rpc::core::many (lower<It, CharT>); 

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto wlower = rpc::basic::satisfy<It> ([](CharT const& c) -> bool { return std::iswlower(c); });
  
    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto wlowers = rpc::core::some (wlower<It, CharT>); 

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto wlowerm = rpc::core::many (wlower<It, CharT>); 
  
    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto upper = rpc::basic::satisfy<It> ([](CharT const& c) -> bool { return std::isupper(c); });   

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto uppers = rpc::core::some (upper<It, CharT>); 

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto upperm = rpc::core::many (upper<It, CharT>); 

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto wupper = rpc::basic::satisfy<It> ([](CharT const& c) -> bool { return std::iswupper(c); });
     
    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto wuppers = rpc::core::some (wupper<It, CharT>); 

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto wupperm = rpc::core::many (wupper<It, CharT>); 
     
    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto digit = rpc::basic::satisfy<It> ([](CharT const& c) -> bool { return std::isdigit(c); });

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto digits = rpc::core::some (digit<It, CharT>); 

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto digitm = rpc::core::many (digit<It, CharT>); 

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto wdigit = rpc::basic::satisfy<It> ([](CharT const& c) -> bool { return std::iswdigit(c); });
     
    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto wdigits = rpc::core::some (wdigit<It, CharT>); 
    
    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto wdigitm = rpc::core::many (wdigit<It, CharT>); 

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto hexdigit = rpc::basic::satisfy<It> ([](CharT const& c) -> bool { return std::isxdigit(c); });

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto hexdigits = rpc::core::some (hexdigit<It, CharT>); 

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto hexdigitm = rpc::core::many (hexdigit<It, CharT>); 

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto whexdigit = rpc::basic::satisfy<It> ([](CharT const& c) -> bool { return std::iswxdigit(c); });
     
    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto whexdigits = rpc::core::some (whexdigit<It, CharT>); 

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto whexdigitm = rpc::core::many (whexdigit<It, CharT>); 

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto cntrl = rpc::basic::satisfy<It> ([](CharT const& c) -> bool { return std::iscntrl(c); });

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto cntrls = rpc::core::some (cntrl<It, CharT>); 

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto cntrlm = rpc::core::many (cntrl<It, CharT>); 

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto wcntrl = rpc::basic::satisfy<It> ([](CharT const& c) -> bool { return std::iswcntrl(c); });
     
    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto wcntrls = rpc::core::some (wcntrl<It, CharT>); 

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto wcntrlm = rpc::core::many (wcntrl<It, CharT>); 

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    rpc::core::parser<It, std::basic_string<CharT>, CharT> word =
    {
        .parse = [](typename rpc::core::parser<It, std::basic_string<CharT>, CharT>::range_type const& r)
        {
            using OT = std::list<typename rpc::core::parser<It, std::basic_string<CharT>, CharT>::result_type>;
         
            auto l = fnk::eval (alphas<It, CharT>.parse, r);
            if (alphas<It, CharT>.is_value_result (l.front())) {
                auto ls = alphas<It, CharT>.values (l);
                return OT
                    { std::make_pair
                        (std::basic_string<CharT>(ls.cbegin(), ls.cend()),
                         alphas<It, CharT>.result_range (l.back()))
                    }; 
            } else
                return OT { rpc::core::failure{"expected word"} };
        }
    };

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto words = rpc::core::some (rpc::core::ignorel(spacem<It, CharT>, word<It, CharT>));

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto wordm = rpc::core::many (rpc::core::ignorel(spacem<It, CharT>, word<It, CharT>));

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    rpc::core::parser<It, std::basic_string<CharT>, CharT> wword =
    {
        .parse = [](typename rpc::core::parser<It, std::basic_string<CharT>, CharT>::range_type const& r)
        {
            using OT = std::list<typename rpc::core::parser<It, std::basic_string<CharT>, CharT>::result_type>;
         
            auto l = fnk::eval (walphas<It, CharT>.parse, r);
            if (walphas<It, CharT>.is_value_result (l.front())) {
                auto ls = walphas<It, CharT>.values (l);
                return OT
                    { std::make_pair
                        (std::basic_string<CharT>(ls.cbegin(), ls.cend()),
                         walphas<It, CharT>.result_range (l.back()))
                    }; 
            } else
                return OT { rpc::core::failure{"expected word"} };
        }
    };

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto wwords = rpc::core::some (rpc::core::ignorel(wspacem<It, CharT>, wword<It, CharT>));

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto wwordm = rpc::core::many (rpc::core::ignorel(wspacem<It, CharT>, wword<It, CharT>));
} // namespace basic
} // namespace rpc

#endif // ifndef TEXT_PARSERS

