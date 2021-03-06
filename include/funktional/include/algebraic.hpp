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
    template <typename T, class Alloc = std::allocator<T>>
    struct recursive
    {
    public:
        using type            = T;
        using reference       = T&;
        using const_reference = T const&;
        using pointer         = T*;
        using const_pointer   = T const*;

        recursive (void) = default;

        recursive (T && t) : data_ (alloc_.allocate (1))
        {
            new (data_.get()) T (t);
        }

        recursive (T const& t) : data_ (alloc_.allocate (1))
        {
            new (data_.get()) T (t);
        }

        recursive (recursive<T> && r)     : data_ (std::move(r.ptr())) {}
 
        recursive (recursive<T> const& r) : data_ (alloc_.allocate (1))
        {
            new (data_.ptr()) T (r.get());
        }

        ~recursive (void) noexcept = default;
 
        inline constexpr T&       value (void) &      { return *addressof(); }
        inline constexpr T&&      value (void) &&     { return std::move (*addressof()); }
        inline constexpr T const& value (void) const& { return *addressof(); }
 
        inline constexpr T&       operator* (void) &      { return value(); }
        inline constexpr T&&      operator* (void) &&     { return value(); }
        inline constexpr T const& operator* (void) const& { return value(); }
 
        inline constexpr T*       addressof (void) noexcept       { return reinterpret_cast<T*> (data_.get()); }
        inline constexpr T const* addressof (void) const noexcept { return reinterpret_cast<T const*> (data_.get()); }
 
        inline constexpr T*       operator& (void) noexcept       { return addressof(); }
        inline constexpr T const* operator& (void) const noexcept { return addressof(); }

        inline constexpr T* ptr (void) noexcept
        {
            return data_.get();
        }

        inline constexpr T const* ptr (void) const noexcept
        {
            return data_.get();
        }
    private:
        Alloc alloc_;
        std::unique_ptr<T> data_; // recursive_type<T> is owning, as it merely replaces the existence of T in a variant.
    };

    template <typename T, typename ... Ts>
    struct adt
    {
    public:
        adt (void) : type_index_(0) { static_assert (sizeof(T) == 0, "cannot default construct algebraic data type"); }

        template <typename U,
            typename = std::enable_if_t<fnk::utility::any_true(std::is_same<U,T>::value, std::is_same<U,Ts>::value...)>,
            bool _ = bool{} /* dummy template parameter to deal with overloading */>
        adt (U && u) : type_index_ (utility::type_to_index<U, T, Ts...>::value)
        {
            new (reinterpret_cast<void*> (storage.template addressof<U>())) U(std::forward<U>(u));
        }
 
        template <typename U,
            typename =
                std::enable_if_t
                    <fnk::utility::any_true(std::is_same<recursive<std::decay_t<U>>,T>::value,
                                            std::is_same<recursive<std::decay_t<U>>,Ts>::value...)>>
        adt (U && u) : type_index_ (utility::type_to_index<recursive<std::decay_t<U>>, T, Ts...>::value)
        {
            using W = std::decay_t<U>;
            new (reinterpret_cast<void*> (storage.template addressof<W>())) recursive<W> (std::forward<U> (u));
        }
 
        adt (adt<T, Ts...> const&) = default;
        
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
            typename = std::enable_if_t<fnk::utility::any_true(std::is_same<U,T>::value, std::is_same<U,Ts>::value...)>,
            bool _ = bool{} /*dummy template parameter to deal with overloading*/> 
        inline constexpr U&       value (void) &      { return *addressof<U>(); }
 
        template <typename U,
            typename =
                std::enable_if_t<fnk::utility::any_true(std::is_same<recursive<U>,T>::value, std::is_same<recursive<U>,Ts>::value...)>> 
        inline constexpr U&       value (void) &      { return **addressof<recursive<U>>(); }
 
        template <typename U,
            typename = std::enable_if_t<fnk::utility::any_true(std::is_same<U,T>::value, std::is_same<U,Ts>::value...)>,
            bool _ = bool{} /*dummy template parameter to deal with overloading*/>       
        inline constexpr U&&      value (void) &&     { return std::move (*addressof<U>()); }
 
        template <typename U,
            typename =
                std::enable_if_t<fnk::utility::any_true(std::is_same<recursive<U>,T>::value, std::is_same<recursive<U>,Ts>::value...)>>
        inline constexpr U&&      value (void) &&     { return std::move (**addressof<recursive<U>>()); } 
 
        template <typename U,
            typename = std::enable_if_t<fnk::utility::any_true(std::is_same<U,T>::value, std::is_same<U,Ts>::value...)>,
            bool _ = bool{} /*dummy template parameter to deal with overloading*/> 
        inline constexpr U const& value (void) const& { return *addressof<U>(); }
  
        template <typename U,
            typename =
                std::enable_if_t<fnk::utility::any_true(std::is_same<recursive<U>,T>::value, std::is_same<recursive<U>,Ts>::value...)>>
        inline constexpr U const& value (void) const& { return **addressof<recursive<U>>(); }
 
        template <typename U,
            typename = std::enable_if_t<fnk::utility::any_true(std::is_same<U,T>::value, std::is_same<U,Ts>::value...)>,
            bool _ = bool{} /*dummy template parameter to deal with overloading*/>       
        inline constexpr U&       operator* (void) &      { return value<U>(); }
   
        template <typename U,
            typename =
                std::enable_if_t<fnk::utility::any_true(std::is_same<recursive<U>,T>::value, std::is_same<recursive<U>,Ts>::value...)>> 
        inline constexpr U&       operator* (void) &      { return value<recursive<U>>(); }
 
        template <typename U,
            typename = std::enable_if_t<fnk::utility::any_true(std::is_same<U,T>::value, std::is_same<U,Ts>::value...)>,
            bool _ = bool{} /*dummy template parameter to deal with overloading*/>       
        inline constexpr U&&      operator* (void) &&     { return value<U>(); }
    
        template <typename U,
            typename =
                std::enable_if_t<fnk::utility::any_true(std::is_same<recursive<U>,T>::value, std::is_same<recursive<U>,Ts>::value...)>> 
        inline constexpr U&&      operator* (void) &&     { return value<recursive<U>>(); }
 
        template <typename U,
            typename = std::enable_if_t<fnk::utility::any_true(std::is_same<U,T>::value, std::is_same<U,Ts>::value...)>,
            bool _ = bool{} /*dummy template parameter to deal with overloading*/>       
        inline constexpr U const& operator* (void) const& { return value<U>(); }
     
        template <typename U,
            typename =
                std::enable_if_t<fnk::utility::any_true(std::is_same<recursive<U>,T>::value, std::is_same<recursive<U>,Ts>::value...)>> 
        inline constexpr U const& operator* (void) const& { return value<recursive<U>>(); }
 
        template <typename U,
            typename = std::enable_if_t<fnk::utility::any_true(std::is_same<U,T>::value, std::is_same<U,Ts>::value...)>,
            bool _ = bool{} /*dummy template parameter to deal with overloading*/>       
        inline constexpr U*       addressof (void) noexcept
        {
            return reinterpret_cast<U*> (storage.template addressof<U>());
        }
      
        template <typename U,
            typename =
                std::enable_if_t<fnk::utility::any_true(std::is_same<recursive<U>,T>::value, std::is_same<recursive<U>,Ts>::value...)>>   
        inline constexpr U*       addressof (void) noexcept
        {
            return reinterpret_cast<U*> (storage.template addressof<recursive<U>>()->addressof());
        }
 
        template <typename U,
            typename = std::enable_if_t<fnk::utility::any_true(std::is_same<U,T>::value, std::is_same<U,Ts>::value...)>,
            bool _ = bool{} /*dummy template parameter to deal with overloading*/>       
        inline constexpr U const* addressof (void) const noexcept
        {
            return reinterpret_cast<U const*> (storage.template addressof<U>());
        }
       
        template <typename U,
            typename =
                std::enable_if_t<fnk::utility::any_true(std::is_same<recursive<U>,T>::value, std::is_same<recursive<U>,Ts>::value...)>>  
        inline constexpr U const* addressof (void) const noexcept
        {
            return reinterpret_cast<U const*> (storage.template addressof<recursive<U>>()->addressof());
        }
 
        template <typename U,
            typename = std::enable_if_t<fnk::utility::any_true(std::is_same<U,T>::value, std::is_same<U,Ts>::value...)>,
            bool _ = bool{} /*dummy template parameter to deal with overloading*/>       
        inline constexpr U*       operator& (void) noexcept       { return addressof<U>(); }
 
        template <typename U,
            typename =
                std::enable_if_t<fnk::utility::any_true(std::is_same<recursive<U>,T>::value, std::is_same<recursive<U>,Ts>::value...)>>
        inline constexpr U*       operator& (void) noexcept       { return addressof<recursive<U>>(); }

        template <typename U,
            typename = std::enable_if_t<fnk::utility::any_true(std::is_same<U,T>::value, std::is_same<U,Ts>::value...)>,
            bool _ = bool{} /*dummy template parameter to deal with overloading*/>
        inline constexpr U const* operator& (void) const noexcept { return addressof<U>(); }
  
        template <typename U,
            typename =
                std::enable_if_t<fnk::utility::any_true(std::is_same<recursive<U>,T>::value, std::is_same<recursive<U>,Ts>::value...)>> 
        inline constexpr U const* operator& (void) const noexcept { return addressof<recursive<U>>(); }
 
        template <typename U,
            typename = std::enable_if_t<fnk::utility::any_true(std::is_same<U,T>::value, std::is_same<U,Ts>::value...)>,
            bool _ = bool{} /*dummy template parameter to deal with overloading*/>      
        constexpr decltype(auto) operator= (U && u) noexcept (std::is_nothrow_move_assignable<U>::value)
        {
            new (reinterpret_cast<void*> (storage.template addressof<U>())) U(std::forward<U>(u));
            return *this;
        }
   
        template <typename U,
            typename =
                std::enable_if_t<fnk::utility::any_true(std::is_same<recursive<U>,T>::value, std::is_same<recursive<U>,Ts>::value...)>> 
        constexpr decltype(auto) operator= (U && u) noexcept (std::is_nothrow_move_assignable<U>::value)
        {
            new (reinterpret_cast<void*> (storage.template addressof<recursive<U>>())) recursive<U> (std::forward<U>(u));
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
        std::size_t type_index_;
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

