//
// Parser result type
//
// Author: Dalton Woodard
// Contact: daltonmwoodard@gmail.com
// License: Please see LICENSE.md
//

#ifndef RESULT_TYPE_HPP
#define RESULT_TYPE_HPP

#include <iostream>
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
        failure_message (void) : msg("failure") {}

        failure_message (std::string && s)     : msg(s) {}
        failure_message (std::string const& s) : msg(s) {}

        failure_message (failure_message && f)      : msg(std::move(f.msg)) {}
        failure_message (failure_message const&& f) : msg(std::move(f.msg)) {} 

        failure_message (failure_message & f)      : msg(f.msg) {}
        failure_message (failure_message const& f) : msg(f.msg) {}

        std::string const msg;
    };

    std::ostream& operator<< (std::ostream& os, failure_message const& f)
    {
        return (os << f.msg);
    }

    //
    // parse failure
    //
    using failure = failure_message;

    //
    // empty parse; success but no value
    // is produced.
    //
    template <typename T>
    struct empty
    {
        using type = T;

        template <typename U>
        using rebind = empty<U>;

        template <typename U>
        operator U() const noexcept
        {
            return rebind<U>{};
        }
    };
 
    template <typename T>
    struct is_empty_instance : public std::false_type {};
   
    template <typename T>
    struct is_empty_instance<empty<T>> : public std::true_type {};
    
    template <typename T>
    struct is_empty_instance<empty<T> const> : public std::true_type {};
    
    template <typename T>
    struct is_empty_instance<empty<T> &> : public std::true_type {}; 
    
    template <typename T>
    struct is_empty_instance<empty<T> const&> : public std::true_type {};
 
    template <typename T>
    struct is_empty_instance<empty<T> &&> : public std::true_type {};    

    template <typename E>
    struct empty_traits
    {
        using type = void;
    };

    template <typename T>
    struct empty_traits<empty<T>>
    {
        using type = T;
    };

    template <typename T>
    struct empty_traits<empty<T> &> : public empty_traits<empty<T>> {};
   
    template <typename T>
    struct empty_traits<empty<T> const> : public empty_traits<empty<T>> {};
   
    template <typename T>
    struct empty_traits<empty<T> const&> : public empty_traits<empty<T>> {};

    template <typename T>
    struct empty_traits<empty<T> &&> : public empty_traits<empty<T>> {};

    template <typename T>
    using empty_result = empty<T>;

    //
    // bonafide parsed value
    //
    template <typename V>
    using value_result = V;

    //
    // variant type representing possible parse
    // results: value, empty, or failure.
    //
    template <typename V>
    struct parse_result
        : public fnk::adt <value_result<V>, empty_result<V>, failure>
    {
        //
        // inherit adt constructors
        //
    private:
        using A = fnk::adt <value_result<V>, empty_result<V>, failure>;
    public: 
        using A::A;

        //
        // views and accesses
        //
        inline bool is_value (void) const noexcept
        {
            return this->A::type_index() == detail::result_tags::PARSE_VALUE;
        }

        inline bool is_empty (void) const noexcept
        {
            return this->A::type_index() == detail::result_tags::PARSE_EMPTY;
        }

        inline bool is_failure (void) const noexcept
        {
            return this->A::type_index() == detail::result_tags::PARSE_FAILURE;
        }

        inline bool is_success (void) const noexcept
        {
            return not is_failure ();
        }

        inline value_result<V> to_value (void) const noexcept
        {
            return this->A::template value <value_result<V>>();
        }

        inline empty_result<V> to_empty (void) const noexcept
        {
            return this->A::template value <empty_result<V>>();
        }

        inline failure to_failure (void) const noexcept
        {
            return this->A::template value <failure>();
        }

        inline std::string to_failure_message (void) const noexcept
        {
            return this->A::template value <failure>().msg;
        }
    };
} // namespace core
} // namespace rpc

#endif // ifndef RESULT_TYPE_HPP

