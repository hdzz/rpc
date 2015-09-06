//
// Filterable objects
//
// Author: Dalton Woodard
// Contact: daltonmwoodard@gmail.com
// License: Please see LICENSE.md
//

#ifndef FILTERABLE_HPP
#define FILTERABLE_HPP

#include <deque>
#include <list>
#include <set>
#include <string>
#include <type_traits>
#include <vector>

#include "foldable.hpp"
#include "type_support/container_traits.hpp"
#include "type_support/function_traits.hpp"

namespace funk
{
    template <class T>
    struct filterable
    {
        // T value_type
        // filterable<A> filter (bool(A), filterable<A>)
        struct is_filterable_instance : public std::false_type {};
    };

    template <class F, class T>
    inline constexpr decltype(auto) filter (F && f, T && in)
    {
        return funk::filterable<T>::filter (f, std::forward<T>(in));
    }

    template <class C,
        typename = std::enable_if_t<funk::foldable<C>::is_foldable_instance::value>>
    struct default_filterable : public funk::foldable<C>
    {
        using value_type = typename funk::type_support::container_traits<C>::value_type;

        template <class P, class C_,
            typename = std::enable_if_t
                <std::is_convertible<typename funk::type_support::function_traits<P>::return_type, bool>::value>,
            typename = std::enable_if_t<std::is_convertible<C_,C>::value>>
        static constexpr decltype(auto) filter (P && pred, C_ && c)
        {
            using OT = funk::utility::rebind_argument_t<C_, C>;
            OT out;
            for (auto&& e : std::forward<OT>(c))
               if (pred (e))
                  funk::type_support::container_traits<OT>::insert (out, e);
            return out; 
        }

        struct is_filterable_instance : public std::true_type {};
    };

#define DEFAULT_FILTERABLE_INSTANCE(C)\
    template<typename ... Args>\
    struct filterable<C<Args...>> : public default_filterable<C<Args...>> {};\
    template<typename ... Args>\
    struct filterable<C<Args...> const> : public default_filterable<C<Args...>> {};\
    template<typename ... Args>\
    struct filterable<C<Args...> &> : public default_filterable<C<Args...>> {};\
    template<typename ... Args>\
    struct filterable<C<Args...> const&> : public default_filterable<C<Args...>> {};\
    template<typename ... Args>\
    struct filterable<C<Args...> &&> : public default_filterable<C<Args...>> {};

#define DEFAULT_CONTAINERS()\
    DEFAULT_FILTERABLE_INSTANCE(std::basic_string)\
    DEFAULT_FILTERABLE_INSTANCE(std::deque)\
    DEFAULT_FILTERABLE_INSTANCE(std::list)\
    DEFAULT_FILTERABLE_INSTANCE(std::vector)\
    DEFAULT_FILTERABLE_INSTANCE(std::set)\
    DEFAULT_FILTERABLE_INSTANCE(std::multiset)\
    DEFAULT_FILTERABLE_INSTANCE(std::unordered_set)\
    DEFAULT_FILTERABLE_INSTANCE(std::unordered_multiset)

    DEFAULT_CONTAINERS();

#undef DEFAULT_CONTAINERS
#undef DEFAULT_FILTERABLE_INSTANCE

} // namespace funk

#endif // FILTERABLE_HPP

