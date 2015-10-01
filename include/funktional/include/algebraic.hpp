//
// Algebraic (variant) data types in C++
//
// Author: Dalton Woodard
// Contact: daltonmwoodard@gmail.com
// License: Please see LICENSE.md
//

#ifndef ALGEBRAIC_HPP
#define ALGEBRAIC_HPP

//
// Algebraic types are discriminated unions of types T1, T2, ..., TN,
// initialized once with a single fixed type Ti, which is invariant through
// the lifetime of the object. Moreover, such a type supports a never-empty guarantee;
// that is, expressions of the form
//      fnk::adt<T1,T2,...,TN> a;
// are not supported. Rather, they must be initialized at declaration with an expression
// of type Ti, implicitly convertible to one of T1, ..., TN.
// 
// Note: this is NOT the same as a boost::variant type.
//

#include <algorithm>
#include <memory>
#include <type_traits>

#include "utility/type_utils.hpp"
#include "utility/bool_utils.hpp"

namespace fnk
{
namespace detail
{
    template <typename T, typename ... Ts>
    struct adt_internal_storage
    {
    public:
        ~adt_internal_storage (void) noexcept = default;
       
        template <typename U> 
        inline constexpr U&       value (void) &      { return *addressof<U>(); }
        template <typename U>
        inline constexpr U&&      value (void) &&     { return std::move (*addressof<U>()); }
        template <typename U>
        inline constexpr U const& value (void) const& { return *addressof<U>(); }
       
        template <typename U>
        inline constexpr U&       operator* (void) &      { return value<U>(); }
        template <typename U>
        inline constexpr U&&      operator* (void) &&     { return value<U>(); }
        template <typename U>
        inline constexpr U const& operator* (void) const& { return value<U>(); }
        
        template <typename U>
        inline constexpr U*       addressof (void) noexcept       { return reinterpret_cast<U*> (data); }
        template <typename U>
        inline constexpr U const* addressof (void) const noexcept { return reinterpret_cast<U const*> (data); }
        
        template <typename U>
        inline constexpr U*       operator& (void) noexcept       { return addressof<U>(); }
        template <typename U>
        inline constexpr U const* operator& (void) const noexcept { return addressof<U>(); }
    
    private: 
        alignas (std::max({alignof(T), alignof(Ts)...})) unsigned char data [std::max({sizeof(T), sizeof(Ts)...})];
    };
} // namespace detail

    template <typename T, typename ... Ts>
    struct adt
    {
    public:
        adt (void) : type_index_(0) { static_assert (sizeof(T) == 0, "cannot default construct algebraic data type"); }

        adt (adt<T, Ts...> const&) = default;

        template <typename U,
            typename = std::enable_if_t<fnk::utility::any_true(std::is_same<U,T>::value, std::is_same<U,Ts>::value...)>>
        adt (U && u) : type_index_ (utility::type_to_index<U, T, Ts...>::value)
        {
            new (reinterpret_cast<void*> (storage.template addressof<U>())) U(std::forward<U>(u));
        }
        
        ~adt (void) noexcept = default;

        template <typename U,
            typename = std::enable_if_t<fnk::utility::any_true(std::is_same<U,T>::value, std::is_same<U,Ts>::value...)>>
        static decltype(auto) emplace (U && u) noexcept (std::is_nothrow_move_constructible<U>::value)
        {
            return adt (U(std::forward<U>(u))); 
        }

        template <typename U,
            typename = std::enable_if_t<fnk::utility::any_true(std::is_same<U,T>::value, std::is_same<U,Ts>::value...)>,
            typename ... Args>
        static decltype(auto) emplace (Args && ... args) noexcept (std::is_nothrow_constructible<U, Args...>::value)
        {
            return adt (U(std::forward<Args>(args)...)); 
        }

        template <typename U,
            typename = std::enable_if_t<fnk::utility::any_true(std::is_same<U,T>::value, std::is_same<U,Ts>::value...)>,
            typename A,
            typename ... Args>
        static decltype(auto) emplace (std::initializer_list<A> const& il, Args && ... args)
            noexcept (std::is_nothrow_constructible<U, decltype(il), Args...>::value) 
        {
            return adt (U(il, std::forward<Args>(args)...));
        }

        template <typename U,
            typename = std::enable_if_t<fnk::utility::any_true(std::is_same<U,T>::value, std::is_same<U,Ts>::value...)>>
        void call_dtor (void) noexcept (std::is_nothrow_destructible<U>::value)
        {
            reinterpret_cast<U*> (storage.template addressof<U>())->U::~U (); 
        }

        template <typename U,
            typename = std::enable_if_t<fnk::utility::any_true(std::is_same<U,T>::value, std::is_same<U,Ts>::value...)>> 
        inline constexpr U&       value (void) &      { return *addressof<U>(); }
        
        template <typename U,
            typename = std::enable_if_t<fnk::utility::any_true(std::is_same<U,T>::value, std::is_same<U,Ts>::value...)>>
        inline constexpr U&&      value (void) &&     { return std::move (*addressof<U>()); }
        
        template <typename U,
            typename = std::enable_if_t<fnk::utility::any_true(std::is_same<U,T>::value, std::is_same<U,Ts>::value...)>>
        inline constexpr U const& value (void) const& { return *addressof<U>(); }
       
        template <typename U,
            typename = std::enable_if_t<fnk::utility::any_true(std::is_same<U,T>::value, std::is_same<U,Ts>::value...)>>
        inline constexpr U&       operator* (void) &      { return value<U>(); }
        
        template <typename U,
            typename = std::enable_if_t<fnk::utility::any_true(std::is_same<U,T>::value, std::is_same<U,Ts>::value...)>>
        inline constexpr U&&      operator* (void) &&     { return value<U>(); }
        
        template <typename U,
            typename = std::enable_if_t<fnk::utility::any_true(std::is_same<U,T>::value, std::is_same<U,Ts>::value...)>>
        inline constexpr U const& operator* (void) const& { return value<U>(); }
        
        template <typename U,
            typename = std::enable_if_t<fnk::utility::any_true(std::is_same<U,T>::value, std::is_same<U,Ts>::value...)>>
        inline constexpr U*       addressof (void) noexcept
        {
            return reinterpret_cast<U*> (storage.template addressof<U>());
        }
        
        template <typename U,
            typename = std::enable_if_t<fnk::utility::any_true(std::is_same<U,T>::value, std::is_same<U,Ts>::value...)>>
        inline constexpr U const* addressof (void) const noexcept
        {
            return reinterpret_cast<U const*> (storage.template addressof<U>());
        }
        
        template <typename U,
            typename = std::enable_if_t<fnk::utility::any_true(std::is_same<U,T>::value, std::is_same<U,Ts>::value...)>>
        inline constexpr U*       operator& (void) noexcept       { return addressof<U>(); }
        
        template <typename U,
            typename = std::enable_if_t<fnk::utility::any_true(std::is_same<U,T>::value, std::is_same<U,Ts>::value...)>>
        inline constexpr U const* operator& (void) const noexcept { return addressof<U>(); }
       
        template <typename U,
            typename = std::enable_if_t<fnk::utility::any_true(std::is_same<U,T>::value, std::is_same<U,Ts>::value...)>>
        constexpr decltype(auto) operator= (U && u) noexcept (std::is_nothrow_move_assignable<U>::value)
        {
            new (reinterpret_cast<void*> (storage.template addressof<U>())) U(std::forward<U>(u));
            return *this;
        }

        inline decltype(auto) type_index (void) const noexcept
        {
            return type_index_;
        }

        template <typename U>
        using is_adt_type =
            std::conditional_t<fnk::utility::any_true(std::is_same<U,T>::value, std::is_same<U,Ts>::value...), 
                             std::true_type, 
                             std::false_type>;

        using ntypes = std::integral_constant<std::size_t, 1 + sizeof...(Ts)>;

        template <std::size_t N,
            typename = std::enable_if_t<N < ntypes::value>> 
        using type = std::tuple_element_t<N, std::tuple<T, Ts...>>;

        template <typename U,
            typename = std::enable_if_t<is_adt_type<U>::value>>
        using index = fnk::utility::type_to_index<U, T, Ts...>;
    private:
        std::size_t const type_index_;
        detail::adt_internal_storage<T, Ts...> storage;
    };

    template <typename T>
    struct is_algebraic : public std::false_type {};

    template <typename T, typename ... Ts>
    struct is_algebraic<adt<T, Ts...>> : public std::true_type {};
    
    template <typename T, typename ... Ts>
    struct is_algebraic<adt<T, Ts...> const> : public std::true_type {};
    
    template <typename T, typename ... Ts>
    struct is_algebraic<adt<T, Ts...> &> : public std::true_type {};
    
    template <typename T, typename ... Ts>
    struct is_algebraic<adt<T, Ts...> const&> : public std::true_type {};
    
    template <typename T, typename ... Ts>
    struct is_algebraic<adt<T, Ts...> &&> : public std::true_type {};
} // namespace fnk

#endif // ifndef ALGEBRAIC_HPP

