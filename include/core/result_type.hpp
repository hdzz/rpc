//
// Parser result type
//
// Author: Dalton Woodard
// Contact: daltonmwoodard@gmail.com
// License: Please see LICENSE.md
//

#ifndef RESULT_TYPE_HPP
#define RESULT_TYPE_HPP

#include <utility>

#include "funktional/include/algebraic.hpp"
#include "funktional/include/type_support/container_traits.hpp"

namespace rpc
{
namespace core
{
namespace detail
{
    enum result_tags
    {
        PARSE_VALUE   = 0,
        PARSE_EMPTY   = 1,
        PARSE_FAILURE = 2
    };
} // namespace detail
    
    struct failure_message
    {
        failure_message (void)                     : msg("failure") {}
        failure_message (std::string const& s)     : msg(s) {}
        failure_message (std::string && s)         : msg(s) {}
        failure_message (failure_message const& f) : msg(f.msg) {}

        std::string const msg;
    };

    std::ostream& operator<< (std::ostream& os, failure_message const& f)
    {
        return (os << f.msg);
    }

    //
    // Represents parse failure.
    //
    using failure = failure_message;
 
    template <typename T>
    struct empty
    {
        template <typename U>
        using rebind = empty<U>;

        template <typename U>
        operator U() const noexcept
        {
            return rebind<U>{};
        }
    };
   
    template <typename T>
    using empty_result = empty<T>;

    template <typename V>
    using value_result = V;

    template <typename V>
    using parse_result =
        fnk::adt <value_result<V>, empty_result<V>, failure>;

    template <typename P>
    using parse_result_value_type = typename P::template type <0>::type;

    template <typename V>
    inline decltype(auto) is_value (parse_result<V> const&);
    
    template <typename V>
    inline decltype(auto) is_empty (parse_result<V> const&);

    template <typename V>
    inline decltype(auto) is_success (parse_result<V> const& r)
    {
        return is_value (r) || is_empty (r);
    }

    template <typename V>
    inline decltype(auto) is_value (parse_result<V> const& r)
    {
        return r.type_index() == detail::result_tags::PARSE_VALUE;
    }
 
    template <typename V>
    inline decltype(auto) result_value (parse_result<V> const& r)
    {
        return r.template value <value_result<V>>();
    }   
 
    template <typename V>
    inline decltype(auto) is_empty (parse_result<V> const& r)
    {
        return r.type_index() == detail::result_tags::PARSE_EMPTY;
    }
 
    template <typename V>
    inline decltype(auto) result_empty (parse_result<V> const& r)
    {
        return r.template value <empty_result<V>>();
    }   
    
    template <typename V>
    inline decltype(auto) is_failure (parse_result<V> const& r)
    {
        return r.type_index() == detail::result_tags::PARSE_FAILURE;
    }

    template <typename V>
    inline decltype(auto) result_failure (parse_result<V> const& r)
    {
        return r.template value <failure>();
    }
    
    template <typename V>
    inline decltype(auto) result_failure_message (parse_result<V> const& r)
    {
        return r.template value <failure>().msg;
    }   
} // namespace core
} // namespace rpc

#endif // ifndef RESULT_TYPE_HPP

