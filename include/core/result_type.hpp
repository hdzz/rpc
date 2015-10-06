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
        PARSE_BOT     = 2,
        PARSE_TOP     = 3,
        PARSE_FAILURE = 4
    };

    template <typename T>
    struct botr
    {
        template <typename U>
        using rebind = botr<U>;

        template <typename U>
        operator U() const noexcept
        {
            return rebind<U>{};
        }
    };
 
    template <typename T>
    struct topr
    {
        template <typename U>
        using rebind = topr<U>;

        template <typename U>
        operator U() const noexcept
        {
            return rebind<U>{};
        }
    };

    template <typename T>
    using bot_result = botr<T>;
    
    template <typename T>
    using top_result = topr<T>;
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
    using failure_result = failure_message;
 
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

    template <typename T>
    using value_result = T;

    template <typename V>
    using parse_result =
        fnk::adt <value_result<V>,
                  empty_result<V>,
                  detail::bot_result<V>,
                  detail::top_result<V>,
                  failure_result>;

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
    inline decltype(auto) is_bot (parse_result<V> const& r)
    {
        return r.type_index() == detail::result_tags::PARSE_BOT;
    }
    
    template <typename V>
    inline decltype(auto) result_bot (parse_result<V> const& r)
    {
        return r.template value <detail::bot_result<V>>();
    }   
 
    template <typename V>
    inline decltype(auto) is_top (parse_result<V> const& r)
    {
        return r.type_index() == detail::result_tags::PARSE_TOP;
    }
    
    template <typename V>
    inline decltype(auto) result_top (parse_result<V> const& r)
    {
        return r.template value <detail::top_result<V>>();
    }   
 
    template <typename V>
    inline decltype(auto) is_failure (parse_result<V> const& r)
    {
        return r.type_index() == detail::result_tags::PARSE_FAILURE;
    }

    template <typename V>
    inline decltype(auto) result_failure (parse_result<V> const& r)
    {
        return r.template value <failure_result>();
    }   
} // namespace core
} // namespace rpc

#endif // ifndef RESULT_TYPE_HPP

