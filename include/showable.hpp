//
// Showable objects
//

#ifndef SHOWABLE_HPP
#define SHOWABLE_HPP

#include <cstdint>
#include <string>
#include <type_traits>

#include "accumulate.hpp"
#include "type_support/container_traits.hpp"
#include "utility/bool_utils.hpp"

namespace funk
{
    template <typename T>
    struct showable
    {
        // show (T const&) -> std::string
        // wshow (T const&) -> std::wstring
        struct is_showable_instance : public std::false_type {};
    };

    template <typename T,
        typename = std::enable_if_t<std::is_fundamental<T>::value>>
    struct default_fundamental_showable
    {
        template <typename T_,
            typename = std::enable_if_t<std::is_convertible<T_,T_>::value>>
        static constexpr decltype(auto) show (T_ && t)
        {
            return std::to_string (std::forward<T>(t));
        }

        template <typename T_,
            typename = std::enable_if_t<std::is_convertible<T_,T_>::value>>
        static constexpr decltype(auto) wshow (T_ && t)
        {
            return std::to_wstring (std::forward<T>(t));
        }

        struct is_showable_instance : public std::true_type {};
    };

    template <template <typename...> class C, typename ... Args>
    struct default_compound_showable
    {
         
    };
} // namespace funk

#endif // ifndef SHOWABLE_HPP

