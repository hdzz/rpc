//
// Container Traits
//
// Author: Dalton Woodard
// Contact: daltonmwoodard@gmail.com
// License: Please see LICENSE.md
//

#ifndef CONTAINER_TRAITS
#define CONTAINER_TRAITS

#include <deque>
#include <forward_list>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../utility/type_utils.hpp"

namespace fnk
{
namespace type_support
{
    template <class C>
    struct container_traits;

    template <class C>
    struct default_container_traits
    {
        struct is_container   : public std::false_type {};
        struct is_sequential  : public std::false_type {};
        struct is_associative : public std::false_type {};
        struct is_hash        : public std::false_type {};
        template <typename U>
        using rebind = U;
    };

    template <class C>
    struct sequence_container_traits : public default_container_traits<C> {}; 

    template <template <class> class C, class T>
    struct sequence_container_traits<C<T>> : public default_container_traits<C<T>>
    {
    private:
        using CT = C<T>;
    public: 
        static inline constexpr decltype(auto) size (CT const& c) noexcept
        {
            return c.size();
        }

        template <typename T_, typename = std::enable_if_t<std::is_convertible<T_,T>::value>>
        static inline void insert (CT & c, T_ && t) 
        { 
            c.push_back (std::forward<T_>(t));
        }

        static inline void concat (CT & l, CT const& r) 
        {
            l.insert (l.end(), r.cbegin(), r.cend());
        }

        template <class U>
        using rebind = C<U>;
        
        using value_type = T;
        
        struct is_container  : public std::true_type {};
        struct is_sequential : public std::true_type {};
    };

    template <template <class, class> class C, class T, class Alloc>
    struct sequence_container_traits<C<T,Alloc>> : default_container_traits<C<T,Alloc>>
    {
    private:
        using CT = C<T,Alloc>;
    public: 
        static inline constexpr decltype(auto) size (CT const& c) noexcept
        {
            return c.size();
        }

        template <typename T_, typename = std::enable_if_t<std::is_convertible<T_,T>::value>>
        static inline constexpr void insert (CT & c, T_ && t)
        { 
            c.push_back (std::forward<T_>(t));
        }

        static inline constexpr void concat (CT & l, CT const& r)
        {
            l.insert (l.end(), r.cbegin(), r.cend());
        }

        template <class U>
        using rebind = C<U,typename Alloc::template rebind<U>::other>;
        
        using value_type = T;
        
        struct is_container  : public std::true_type {};
        struct is_sequential : public std::true_type {};
    };

#define DEFAULT_SEQUENCE_CONTAINER_TRAITS_IMPLEMENTATION(C)\
    template <typename ... Args>\
    struct container_traits<C<Args...>> : public sequence_container_traits<C<Args...>> {};\
    template <typename ... Args>\
    struct container_traits<C<Args...> const> : public sequence_container_traits<C<Args...>> {};\
    template <typename ... Args>\
    struct container_traits<C<Args...> &> : public sequence_container_traits<C<Args...>> {};\
    template <typename ... Args>\
    struct container_traits<C<Args...> const&> : public sequence_container_traits<C<Args...>> {};\
    template <typename ... Args>\
    struct container_traits<C<Args...> &&> : public sequence_container_traits<C<Args...>> {};

#define DEFAULT_CONTAINERS()\
    DEFAULT_SEQUENCE_CONTAINER_TRAITS_IMPLEMENTATION(std::deque)\
    DEFAULT_SEQUENCE_CONTAINER_TRAITS_IMPLEMENTATION(std::forward_list)\
    DEFAULT_SEQUENCE_CONTAINER_TRAITS_IMPLEMENTATION(std::list)\
    DEFAULT_SEQUENCE_CONTAINER_TRAITS_IMPLEMENTATION(std::vector)

    DEFAULT_CONTAINERS();

#undef DEFAULT_SEQUENCE_CONTAINER_TRAITS_IMPLEMENTATION
#undef DEFAULT_CONTAINERS

    template <class C>
    struct associative_container_traits : public default_container_traits<C> {};

    template <template<typename,typename,typename> class C, typename T, template <typename> class Comp, typename Alloc>
    struct associative_container_traits<C<T,Comp<T>,Alloc>> : public default_container_traits<C<T,Comp<T>,Alloc>>
    {
    private:
        using CT = C<T,Comp<T>,Alloc>;
    public: 
        static inline constexpr decltype(auto) size (CT const& c) noexcept
        {
            return c.size();
        }
        
        template <typename T_, typename = std::enable_if_t<std::is_convertible<T_,T>::value>>
        static inline constexpr void insert (CT & c, T_ && t)
        {
            c.insert (std::forward<T_>(t));
        }
        
        static inline constexpr void concat (CT & l, CT const& r)
        {
            l.insert (r.cbegin(), r.cend());
        }

        template <class U>
        using rebind = C<U,Comp<U>,typename Alloc::template rebind<U>::other>;
        
        using value_type = typename CT::value_type;

        struct is_container   : public std::true_type {};
        struct is_associative : public std::true_type {};
    };

#define DEFAULT_ASSOCIATIVE_CONTAINER_TRAITS_IMPLEMENTATION(C)\
    template <typename ... Args>\
    struct container_traits<C<Args...>> : public associative_container_traits<C<Args...>> {};\
    template <typename ... Args>\
    struct container_traits<C<Args...> const> : public associative_container_traits<C<Args...>> {};\
    template <typename ... Args>\
    struct container_traits<C<Args...> &> : public associative_container_traits<C<Args...>> {};\
    template <typename ... Args>\
    struct container_traits<C<Args...> const&> : public associative_container_traits<C<Args...>> {};\
    template <typename ... Args>\
    struct container_traits<C<Args...> &&> : public associative_container_traits<C<Args...>> {};

#define DEFAULT_CONTAINERS()\
    DEFAULT_ASSOCIATIVE_CONTAINER_TRAITS_IMPLEMENTATION(std::set)\
    DEFAULT_ASSOCIATIVE_CONTAINER_TRAITS_IMPLEMENTATION(std::multiset)\
    DEFAULT_ASSOCIATIVE_CONTAINER_TRAITS_IMPLEMENTATION(std::map)\
    DEFAULT_ASSOCIATIVE_CONTAINER_TRAITS_IMPLEMENTATION(std::multimap)

    DEFAULT_CONTAINERS();

#undef DEFAULT_ASSOCIATIVE_CONTAINER_TRAITS_IMPLEMENTATION
#undef DEFAULT_CONTAINERS

    template<class C>
    struct hash_container_traits : public default_container_traits<C> {};

    template <
        template <typename,typename,typename,typename> class C, 
        typename T,
        template <typename> class Hash,
        template <typename> class Comp,
        typename Alloc
    >
    struct hash_container_traits<C<T,Hash<T>,Comp<T>,Alloc>> : public default_container_traits<C<T,Hash<T>,Comp<T>,Alloc>>
    {
    private:
        using CT = typename std::remove_reference_t<C<T,Hash<T>,Comp<T>,Alloc>>;
    public:
        static inline constexpr decltype(auto) size (CT const& c) noexcept
        {
            return c.size();
        }

        template <typename T_, typename = std::enable_if_t<std::is_convertible<T_,T>::value>>
        static inline constexpr void insert (CT & c, T_ && t)
        {
            c.insert (std::forward<T_>(t));
        }

        static inline constexpr void concat (CT & l, CT const& r)
        {
            l.insert (r.cbegin(), r.cend());
        }

        template<class U>
        using rebind = C<U,Hash<U>,Comp<U>,typename Alloc::template rebind<U>::other>;
        
        using value_type = typename CT::value_type;

        struct is_container : public std::true_type {};
        struct is_hash      : public std::true_type {};
    };

#define DEFAULT_HASH_CONTAINER_TRAITS_IMPLEMENTATION(C)\
    template <typename ... Args>\
    struct container_traits<C<Args...>> : public hash_container_traits<C<Args...>> {};\
    template <typename ... Args>\
    struct container_traits<C<Args...> const> : public hash_container_traits<C<Args...>> {};\
    template <typename ... Args>\
    struct container_traits<C<Args...> &> : public hash_container_traits<C<Args...>> {};\
    template <typename ... Args>\
    struct container_traits<C<Args...> const&> : public hash_container_traits<C<Args...>> {};\
    template <typename ... Args>\
    struct container_traits<C<Args...> &&> : public hash_container_traits<C<Args...>> {};

#define DEFAULT_CONTAINERS()\
    DEFAULT_HASH_CONTAINER_TRAITS_IMPLEMENTATION(std::unordered_set)\
    DEFAULT_HASH_CONTAINER_TRAITS_IMPLEMENTATION(std::unordered_multiset)\
    DEFAULT_HASH_CONTAINER_TRAITS_IMPLEMENTATION(std::unordered_map)\
    DEFAULT_HASH_CONTAINER_TRAITS_IMPLEMENTATION(std::unordered_multimap)

    DEFAULT_CONTAINERS();

#undef DEFAULT_HASH_CONTAINER_TRAITS_IMPLEMENTATION
#undef DEFAULT_CONTAINERS

    template <typename T, template <typename> class K, typename Alloc>
    struct container_traits<std::basic_string<T,K<T>,Alloc>> : public default_container_traits<std::basic_string<T,K<T>,Alloc>>
    {
    private:
        using S = std::basic_string<T,K<T>,Alloc>;
    public: 
        template <typename S_, typename = std::enable_if_t<std::is_convertible<S_, S>::value>>
        static inline constexpr decltype(auto) size (S_ && s) noexcept
        {
            using U = fnk::utility::rebind_argument_t<S_, S>;
            return std::forward<U>(s).size();
        }
        
        template <typename T_, typename = std::enable_if_t<std::is_convertible<T_,T>::value>>
        static inline constexpr void insert (S & s, T_ && t)
        {
            s.push_back (std::forward<T_>(t));
        }

        template <typename S_, typename = std::enable_if_t<std::is_convertible<S_, S>::value>>
        static inline constexpr void concat (S & l, S_ && r)
        {
            l.append (std::forward<S_>(r));
        }
        
        template <class U>
        using rebind = std::basic_string<U,K<U>,typename Alloc::template rebind<U>::other>;
        
        using value_type = typename S::value_type;

        struct is_container  : public std::true_type {};
        struct is_sequential : public std::true_type {};
    };

    template <typename T, template <typename> class K, typename Alloc>
    struct container_traits<std::basic_string<T,K<T>,Alloc> const> : container_traits<std::basic_string<T,K<T>,Alloc>> {};

    template <typename T, template <typename> class K, typename Alloc>
    struct container_traits<std::basic_string<T,K<T>,Alloc> &> : container_traits<std::basic_string<T,K<T>,Alloc>> {};
    
    template <typename T, template <typename> class K, typename Alloc>
    struct container_traits<std::basic_string<T,K<T>,Alloc> const&> : container_traits<std::basic_string<T,K<T>,Alloc>> {};

    template <typename T, template <typename> class K, typename Alloc>
    struct container_traits<std::basic_string<T,K<T>,Alloc> &&> : container_traits<std::basic_string<T,K<T>,Alloc>> {};
} // namespace type_support
} // namespace fnk

#endif // ifndef container_TRAITS

