//
// Function trait implementation.
//
// Author: Dalton Woodard
// Contact: daltonmwoodard@gmail.com
// License: Please see LICENSE.md
//

#ifndef FUNCTION_TRAITS_HPP
#define FUNCTION_TRAITS_HPP

#include <functional>

namespace funk
{
namespace type_support
{
    //
    // Function Traits
    //
    template <class F>
    struct function_traits;

    // base function trait struct
    template <typename R, typename ... Args>
    struct function_traits<R(Args...)>
    {
        using return_type = R;
        static constexpr std::size_t arity = sizeof ...(Args);
        
        template <std::size_t N>
        struct argument
        {
            static_assert (N < arity, "invalid parameter index.");
            using type = typename std::tuple_element_t<N, std::tuple<Args...>>;
        };
    };

    // function pointer types
    template <typename R, typename ... Args>
    struct function_traits<R(*)(Args...)> : public function_traits<R(Args...)> {};

    template <typename R, typename ... Args>
    struct function_traits<R(&)(Args...)> : public function_traits<R(Args...)> {};

    // member function types
    template <class C, typename R, typename ... Args>
    struct function_traits<R(C::*)(Args...)> : public function_traits<R(C&,Args...)> {};

    // const member function pointer types
    template <class C, typename R, typename ... Args>
    struct function_traits<R(C::*)(Args...) const> : public function_traits<R(C&, Args...)> {};

    // member object pointer types
    template <class C, typename R>
    struct function_traits<R(C::*)> : public function_traits<R(C&)> {};

    // function object types
    template <class F>
    struct function_traits
    {
    private:
        using call_type = function_traits<decltype(&F::operator())>;
    public:
        using return_type = typename call_type::return_type;
        static constexpr std::size_t arity = call_type::arity - 1;

        template <std::size_t N>
        struct argument
        {
            static_assert (N < arity, "invalid parameter index.");
            using type = typename call_type::template argument<N+1>::type;
        };
    };

    template <class F>
    struct function_traits<F&> : public function_traits<F> {};

    template <class F>
    struct function_traits<F&&> : public function_traits<F> {};
} // namespace type_support
} // namespace funk

#endif // ifndef FUNCTION_TRAITS_HPP

