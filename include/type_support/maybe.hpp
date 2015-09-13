//
// Maybe implementation based on the proposal for std::optional
//
// Author: Dalton Woodard
// Contact: daltonmwoodard@gmail.com
// License: Please see LICENSE.md
//

#ifndef MAYBE_HPP
#define MAYBE_HPP

#include <string>
#include <type_traits>
#include <new>

#include "utility/type_utils.hpp"

namespace fnk
{
namespace type_support
{
    template <class T>
    struct maybe;

    template <class T>
    struct maybe<T&>;

    template <class T>
    struct nothing : private maybe<T>
    {
        static constexpr bool is_nothing (void) noexcept { return true; }
    };

    template <class T>
    struct is_maybe : std::false_type {};

    template <class T>
    struct is_maybe<maybe<T>> : std::true_type {};

    template <class T>
    struct is_maybe<nothing<T>> : std::true_type {};

    namespace detail
    {
        template <class T>
        struct type_storage_default
        {
            std::aligned_storage_t<sizeof(T), alignof(T)> data;
            
            constexpr type_storage_default (void) noexcept = default;
            template <class ... Args>
            constexpr type_storage_default (Args && ... args)
            { 
                *reinterpret_cast<T*> (&data) = T(utility::forward_constexpr<Args> (args)...);
            }
            
            ~type_storage_default (void) noexcept {}
        };
        
        template <class T>
        struct type_storage_trivial
        {
            std::aligned_storage_t<sizeof(T), alignof(T)> data;
            
            constexpr type_storage_trivial (void) noexcept = default;
            template <class ... Args>
            constexpr type_storage_trivial (Args && ... args)
            { 
                *reinterpret_cast<T*> (&data) = T(utility::forward_constexpr<Args> (args)...);
            }
            
            ~type_storage_trivial (void) noexcept = default;
        };
    } // namespace detail

    template <class T>
    struct maybe_default
    {
    protected:
        bool init;
        detail::type_storage_default<T> storage;
        
        // implementation
        constexpr maybe_default (void) noexcept       : init (false) {}
        explicit constexpr maybe_default (T const& t) : init (true), storage (t) {}
        explicit constexpr maybe_default (T && t)     : init (true), storage (utility::move_constexpr(t)) {}
        template <class T_, class ... Args>
        explicit constexpr maybe_default (std::initializer_list<T_> il, Args&& ... ia) : init (true)
                                                                                       , storage (il, std::forward<Args> (ia)...) {}
    public:
        ~maybe_default (void) noexcept
        {
            if (init) { reinterpret_cast<T> (storage.data).T::~T(); }
        } 

        void clear (void) noexcept
        {
            if (init)
                addressof()->T::~T();
            init = false;
        }

        template <class ... Args>
        void set (Args&& ... args) noexcept (noexcept(T(std::forward<Args> (args)...)))
        {
            new (reinterpret_cast<void*> (addressof())) T(std::forward<Args> (args)...);
            init = true;
        }

        template <class T_, class ... Args>
        void set (std::initializer_list<T_> il, Args&& ... ia) noexcept (noexcept(T(il, std::forward<Args> (ia)...)))
        {
            new (reinterpret_cast<void*> (addressof())) T(il, std::forward<Args> (ia)...);
            init = true;
        }

        inline constexpr T&       value (void) &      { return *addressof(); }
        inline constexpr T&&      value (void) &&     { return std::move (*addressof()); }
        inline constexpr T const& value (void) const& { return *addressof(); }

        inline constexpr T&       operator* (void) &      { return value(); }
        inline constexpr T&&      operator* (void) &&     { return value(); }
        inline constexpr T const& operator* (void) const& { return value(); }
 
        inline constexpr T*       addressof (void) noexcept       { return reinterpret_cast<T*> (std::addressof(storage.data)); }
        inline constexpr T const* addressof (void) const noexcept { return reinterpret_cast<T const*> (std::addressof(storage.data)); }

        inline constexpr T*       operator& (void) noexcept       { return addressof(); }
        inline constexpr T const* operator& (void) const noexcept { return addressof(); }
 
        inline constexpr bool is_nothing (void) const noexcept  { return !init; }
    };

    template <class T>
    struct maybe_trivial
    {
    protected:
        bool init;
        detail::type_storage_trivial<T> storage;
        
        // implementation
        constexpr maybe_trivial (void) noexcept       : init (false) {}
        explicit constexpr maybe_trivial (T const& t) : init (true), storage (t) {}
        explicit constexpr maybe_trivial (T && t)     : init (true), storage (utility::move_constexpr(t)) {}
        template <class T_, class ... Args>
        explicit constexpr maybe_trivial (std::initializer_list<T_> il, Args&& ... ia) : init (true)
                                                                                       , storage (il, std::forward<Args> (ia)...) {}
    public:
        ~maybe_trivial (void) = default;
     
        void clear (void) noexcept
        {
            if (init)
                addressof()->T::~T();
            init = false; 
        }

        template <class ... Args>
        void set (Args&& ... args) noexcept (noexcept(T(std::forward<Args> (args)...)))
        {
            new (reinterpret_cast<void*> (addressof())) T(std::forward<Args> (args)...);
            init = true;
        }

        template <class T_, class ... Args>
        void set (std::initializer_list<T_> il, Args&& ... ia) noexcept (noexcept(T(il, std::forward<Args> (ia)...)))
        {
            new (reinterpret_cast<void*> (addressof())) T(il, std::forward<Args> (ia)...);
            init = true;
        }

        inline constexpr T&       value (void) &      { return *addressof(); }
        inline constexpr T&&      value (void) &&     { return std::move (*addressof()); }
        inline constexpr T const& value (void) const& { return *addressof(); }
        
        inline constexpr T&       operator* (void) &      { return value(); }
        inline constexpr T&&      operator* (void) &&     { return value(); }
        inline constexpr T const& operator* (void) const& { return value(); }

        inline constexpr T*       addressof (void) noexcept       { return reinterpret_cast<T*> (std::addressof(storage.data)); }
        inline constexpr T const* addressof (void) const noexcept { return reinterpret_cast<T const*> (std::addressof(storage.data)); }

        inline constexpr T*       operator& (void) noexcept       { return addressof(); }
        inline constexpr T const* operator& (void) const noexcept { return addressof(); }
        
        inline constexpr bool is_nothing (void) const noexcept  { return !init; }
    };

    namespace detail
    {
        template <class T>
        using maybe_t = std::conditional_t
            <std::is_trivially_destructible<T>::value, maybe_trivial<T>, maybe_default<T>>;
    } // namesapce detail

    template <class T>
    struct maybe : detail::maybe_t<T>
    {
    public:
        // type support
        using value_type = T;
        template <class U>
        using rebind = maybe<U>; 

        // implementation
        constexpr maybe (void) noexcept : detail::maybe_t<T> () {};
        constexpr maybe (maybe<T> const& m) : detail::maybe_t<T> ()
        {
            if (!m.is_nothing()) {
                new (static_cast<void*> (detail::maybe_t<T>::addressof())) T(*m);
                detail::maybe_t<T>::init = true;
            }
        }
        constexpr maybe (maybe<T> && m) noexcept (std::is_nothrow_move_constructible<T>::value) : detail::maybe_t<T> ()
        {
            if (!m.is_nothing()) {
                new (static_cast<void*> (detail::maybe_t<T>::addressof())) T(std::move(*m));
                detail::maybe_t<T>::init = true; 
            }
        }
        constexpr maybe (T const& t) : detail::maybe_t<T> (t) {}
        constexpr maybe (T && t)     : detail::maybe_t<T> (utility::move_constexpr(t)) {}
        template <class ... Args>
        explicit constexpr maybe (Args&& ... args) : detail::maybe_t<T> (utility::forward_constexpr<Args> (args)...) {}
        template <class T_, class ... Args>
        explicit constexpr maybe (std::initializer_list<T_> il, Args&& ... ia)
            : detail::maybe_t<T> (il, utility::forward_constexpr<Args> (ia)...) {}

        ~maybe (void) = default;

        constexpr decltype(auto) operator= (nothing<T>) noexcept
        {
            detail::maybe_t<T>::clear();
            return *this;
        }

        constexpr decltype(auto) operator= (maybe<T> const& m)
        {
            if      (detail::maybe_t<T>::init && !m.init)
                detail::maybe_t<T>::clear ();
            else if (!detail::maybe_t<T>::init && m.init)
                detail::maybe_t<T>::set (*m);
            else if (detail::maybe_t<T>::init && m.init)
                detail::maybe_t<T>::value() = *m;
            return *this;
        }

        constexpr decltype(auto) operator= (maybe<T> && m)
            noexcept (std::is_nothrow_move_assignable<T>::value && std::is_nothrow_move_constructible<T>::value)
        {
            if      (detail::maybe_t<T>::init && !m.init)
                detail::maybe_t<T>::clear ();
            else if (!detail::maybe_t<T>::init && m.init)
                detail::maybe_t<T>::set (*m);
            else if (detail::maybe_t<T>::init && m.init)
                detail::maybe_t<T>::value() = std::move(*m);
            return *this;
        }

        template <class T_,
            typename = typename std::enable_if<std::is_same<std::remove_reference_t<T_>, T>::value>::type>
        constexpr decltype(auto) operator= (T_ && t)
        {
            if (detail::maybe_t<T>::init)
                detail::maybe_t<T>::value() = std::forward<T_> (t);
            else
                detail::maybe_t<T>::set (std::forward<T_> (t));
            return *this;
        }

        template <class ... Args>
        void emplace (Args&& ... args)
        {
            detail::maybe_t<T>::clear ();
            detail::maybe_t<T>::set (std::forward<Args> (args)...);
        }

        template <class T_, class ... Args>
        void emplace (std::initializer_list<T_> il, Args&& ... ia)
        {
            detail::maybe_t<T>::clear ();
            detail::maybe_t<T>::set (il, std::forward<Args> (ia)...);
        }

        void swap (maybe<T> & o)
            noexcept (std::is_nothrow_move_assignable<T>::value && noexcept(swap(std::declval<T&> (), std::declval<T&> ())))
        {
            if      (detail::maybe_t<T>::init && !o.init) {
                o.set (std::move(**this));
                detail::maybe_t<T>::clear (); 
            } else if (!detail::maybe_t<T>::init && o.init) {
                detail::maybe_t<T>::set (std::move(*o));
                o.clear (); 
            } else if (detail::maybe_t<T>::init && o.init) {
                std::swap (**this, *o);
            }
        }

        template <class U>
        constexpr decltype(auto) just_or (U && u) const&
        {
            return *this ? **this : static_cast<T> (utility::forward_constexpr(u));
        }

        template <class U>
        constexpr decltype(auto) just_or (U && u) &&
        {
            return *this ? utility::move_constexpr (const_cast<maybe<T>&> (*detail::maybe_t<T>::value()))
                         : static_cast<T> (utility::forward_constexpr(u));
        }

        explicit constexpr operator bool (void) const noexcept
        {
            return !this->is_nothing();
        }
    };

    template <class T>
    struct maybe<T&>
    {
    private:
        T * tref;
    public:
        constexpr maybe (void) noexcept               : tref (nullptr) {}
        constexpr maybe (nothing<T&> const) noexcept  : tref (nullptr) {}
        constexpr maybe (T & t) noexcept              : tref (std::addressof(t)) {}
        constexpr maybe (maybe<T&> const& m) noexcept : tref (m.tref) {}
        maybe (T&&) = delete;
        
        ~maybe (void) = default;

        constexpr decltype(auto) operator= (nothing<T&> const) noexcept
        {
            tref = nullptr;
            return *this;
        }

        template <class T_,
            typename = typename std::enable_if<std::is_same<typename std::decay<T_>::type, maybe<T&>>::value>::type>
        constexpr decltype(auto) operator= (T_ && t) noexcept
        {
            tref = t.tref;
            return *this;
        }

        void emplace (T & t) noexcept { tref = std::addressof (t); }
        void emplace (T&& t) = delete;

        void swap (maybe<T&> & o) noexcept
        {
            std::swap (tref, o.tref);
        }

        constexpr decltype(auto) operator-> (void) const noexcept
        {
            return tref;
        }

        constexpr decltype(auto) operator* (void) const noexcept
        {
            return *tref;
        }

        explicit constexpr operator bool (void) const noexcept
        {
            return tref != nullptr;
        }

        template <class T_>
        constexpr typename std::decay<T_>::type just_or (T_ && t) const noexcept
        {
            return *this ? **this : static_cast<typename std::decay<T>::type> (utility::forward_constexpr<T_> (t));
        }
    };

    template <class T>
    struct maybe<T&&>
    {
        static_assert (sizeof(T) == 0, "rvalue reference cannot be maybe");
    };

    template <class T>
    constexpr decltype(auto) make_maybe (T && t)
    {
        return maybe<std::decay_t<T>> (utility::forward_constexpr<T>(t));
    }

    template <class T>
    constexpr decltype(auto) make_maybe (std::reference_wrapper<T> t)
    {
        return maybe<T&> (t.get());
    }

    template <class T>
    void swap (maybe<T> & x, maybe<T> & y) noexcept (noexcept(x.swap(y)))
    {
        x.swap (y);
    }

    template <class T>
    constexpr bool operator== (maybe<T> const& x, maybe<T>const & y)
    {
        return bool(x) != bool(y) ? false : bool(x) == false ? true : *x == *y;
    }

    template <class T>
    constexpr bool operator!= (maybe<T> const& x, maybe<T> const& y)
    {
        return !(x == y);
    }

    template <class T>
    constexpr bool operator< (maybe<T> const& x, maybe<T> const& y)
    {
        return (!y) ? false : (!x) ? true : *x < *y;
    }
      
    template <class T>
    constexpr bool operator> (const maybe<T>& x, const maybe<T>& y)
    {
        return (y < x);
    }

    template <class T>
    constexpr bool operator<= (const maybe<T>& x, const maybe<T>& y)
    {
        return !(y < x);
    }

    template <class T>
    constexpr bool operator>= (const maybe<T>& x, const maybe<T>& y)
    {
        return !(x < y);
    }

    template <class T>
    constexpr bool operator== (const maybe<T>& x, nothing<T>) noexcept
    {
        return (!x);
    }

    template <class T>
    constexpr bool operator== (nothing<T>, const maybe<T>& x) noexcept
    {
        return (!x);
    }

    template <class T>
    constexpr bool operator!= (const maybe<T>& x, nothing<T>) noexcept
    {
        return bool(x);
    }

    template <class T>
    constexpr bool operator!= (nothing<T>, const maybe<T>& x) noexcept
    {
        return bool(x);
    }

    template <class T>
    constexpr bool operator< (const maybe<T>&, nothing<T>) noexcept
    {
        return false;
    }

    template <class T>
    constexpr bool operator< (nothing<T>, const maybe<T>& x) noexcept
    {
        return bool(x);
    }

    template <class T>
    constexpr bool operator<= (const maybe<T>& x, nothing<T>) noexcept
    {
        return (!x);
    }

    template <class T>
    constexpr bool operator<= (nothing<T>, const maybe<T>&) noexcept
    {
        return true;
    }

    template <class T>
    constexpr bool operator> (const maybe<T>& x, nothing<T>) noexcept
    {
        return bool(x);
    }

    template <class T>
    constexpr bool operator> (nothing<T>, const maybe<T>&) noexcept
    {
        return false;
    }

    template <class T>
    constexpr bool operator>= (const maybe<T>&, nothing<T>) noexcept
    {
        return true;
    }

    template <class T>
    constexpr bool operator>= (nothing<T>, const maybe<T>& x) noexcept
    {
        return (!x);
    }

    template <class T>
    constexpr bool operator== (const maybe<T>& x, T const& v)
    {
        return bool(x) ? *x == v : false;
    }

    template <class T>
    constexpr bool operator== (T const& v, const maybe<T>& x)
    {
        return bool(x) ? v == *x : false;
    }

    template <class T>
    constexpr bool operator!= (const maybe<T>& x, T const& v)
    {
        return bool(x) ? *x != v : true;
    }

    template <class T>
    constexpr bool operator!= (T const& v, const maybe<T>& x)
    {
        return bool(x) ? v != *x : true;
    }

    template <class T>
    constexpr bool operator< (const maybe<T>& x, T const& v)
    {
        return bool(x) ? *x < v : true;
    }

    template <class T>
    constexpr bool operator> (T const& v, const maybe<T>& x)
    {
        return bool(x) ? v > *x : true;
    }

    template <class T>
    constexpr bool operator> (const maybe<T>& x, T const& v)
    {
        return bool(x) ? *x > v : false;
    }

    template <class T>
    constexpr bool operator< (T const& v, const maybe<T>& x)
    {
        return bool(x) ? v < *x : false;
    }

    template <class T>
    constexpr bool operator>= (const maybe<T>& x, T const& v)
    {
        return bool(x) ? *x >= v : false;
    }

    template <class T>
    constexpr bool operator<= (T const& v, const maybe<T>& x)
    {
        return bool(x) ? v <= *x : false;
    }

    template <class T>
    constexpr bool operator<= (const maybe<T>& x, T const& v)
    {
        return bool(x) ? *x <= v : true;
    }

    template <class T>
    constexpr bool operator>= (T const& v, const maybe<T>& x)
    {
        return bool(x) ? v >= *x : true;
    }

    template <class T>
    constexpr bool operator== (const maybe<T&>& x, T const& v)
    {
        return bool(x) ? *x == v : false;
    }

    template <class T>
    constexpr bool operator== (T const& v, const maybe<T&>& x)
    {
        return bool(x) ? v == *x : false;
    }

    template <class T>
    constexpr bool operator!= (const maybe<T&>& x, T const& v)
    {
        return bool(x) ? *x != v : true;
    }

    template <class T>
    constexpr bool operator!= (T const& v, const maybe<T&>& x)
    {
        return bool(x) ? v != *x : true;
    }

    template <class T>
    constexpr bool operator< (const maybe<T&>& x, T const& v)
    {
        return bool(x) ? *x < v : true;
    }

    template <class T>
    constexpr bool operator> (T const& v, const maybe<T&>& x)
    {
        return bool(x) ? v > *x : true;
    }

    template <class T>
    constexpr bool operator> (const maybe<T&>& x, T const& v)
    {
        return bool(x) ? *x > v : false;
    }

    template <class T>
    constexpr bool operator< (T const& v, const maybe<T&>& x)
    {
        return bool(x) ? v < *x : false;
    }

    template <class T>
    constexpr bool operator>= (const maybe<T&>& x, T const& v)
    {
        return bool(x) ? *x >= v : false;
    }

    template <class T>
    constexpr bool operator<= (T const& v, const maybe<T&>& x)
    {
        return bool(x) ? v <= *x : false;
    }

    template <class T>
    constexpr bool operator<= (const maybe<T&>& x, T const& v)
    {
        return bool(x) ? *x <= v : true;
    }

    template <class T>
    constexpr bool operator>= (T const& v, const maybe<T&>& x)
    {
        return bool(x) ? v >= *x : true;
    }

    template <class T>
    constexpr bool operator== (const maybe<T const&>& x, T const& v)
    {
        return bool(x) ? *x == v : false;
    }

    template <class T>
    constexpr bool operator== (T const& v, const maybe<T const&>& x)
    {
        return bool(x) ? v == *x : false;
    }

    template <class T>
    constexpr bool operator!= (const maybe<T const&>& x, T const& v)
    {
        return bool(x) ? *x != v : true;
    }

    template <class T>
    constexpr bool operator!= (T const& v, const maybe<T const&>& x)
    {
        return bool(x) ? v != *x : true;
    }

    template <class T>
    constexpr bool operator< (const maybe<T const&>& x, T const& v)
    {
        return bool(x) ? *x < v : true;
    }

    template <class T>
    constexpr bool operator> (T const& v, const maybe<T const&>& x)
    {
        return bool(x) ? v > *x : true;
    }

    template <class T>
    constexpr bool operator> (const maybe<T const&>& x, T const& v)
    {
        return bool(x) ? *x > v : false;
    }

    template <class T>
    constexpr bool operator< (T const& v, const maybe<T const&>& x)
    {
        return bool(x) ? v < *x : false;
    }

    template <class T>
    constexpr bool operator>= (const maybe<T const&>& x, T const& v)
    {
        return bool(x) ? *x >= v : false;
    }

    template <class T>
    constexpr bool operator<= (T const& v, const maybe<T const&>& x)
    {
        return bool(x) ? v <= *x : false;
    }

    template <class T>
    constexpr bool operator<= (const maybe<T const&>& x, T const& v)
    {
        return bool(x) ? *x <= v : true;
    }

    template <class T>
    constexpr bool operator>= (T const& v, const maybe<T const&>& x)
    {
        return bool(x) ? v >= *x : true;
    }
} // namespace type_support
} // namespace fnk

#endif // ifdef MAYBE_HPP

