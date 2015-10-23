//
// This is a splice of Microsoft's GSL,
// using only a slightly modified version
// of the not_null class.
//

/////////////////////////////////////////////////////////////////////////////// 
// 
// Copyright (c) 2015 Microsoft Corporation. All rights reserved. 
// 
// This code is licensed under the MIT License (MIT). 
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
// THE SOFTWARE. 
// 
///////////////////////////////////////////////////////////////////////////////

#ifndef NOT_NULL_HPP
#define NOT_NULL_HPP

#include <memory>
#include <type_traits>

namespace gsl
{
    template <typename T>
    struct not_null_default;

    template <typename T>
    using not_null_ptr =
        std::conditional_t<std::is_pointer<T>::value,
                           not_null_default<T>,
                           not_null_default<T*>>;

    //
    // not_null
    //
    // Restricts a pointer or smart pointer to only hold non-null values.
    // 
    // Has zero size overhead over T.
    //
    // If T is a pointer (i.e. T == U*) then 
    // - allow construction from U* or U& 
    // - disallow construction from nullptrt
    // - disallow default construction
    // - ensure construction from U* fails with nullptr
    // - allow implicit conversion to U*
    //
    template <typename T>
    struct not_null_default
    {
        static_assert (std::is_assignable <T&, std::nullptr_t>::value,
                      "T cannot be assigned nullptr.");
    public:
        using type = not_null_default<T>;

        not_null_default (T t) : ptr(t) { ensure_invariant(); }
        not_null_default& operator= (T const& t)
        {
            ptr = t;
            ensure_invariant ();
            return *this;
        }

        not_null_default (not_null_default const& other)            = default;
        not_null_default& operator= (not_null_default const& other) = default;

        template <typename U,
                  typename = std::enable_if_t<std::is_convertible<U, T>::value>>
        not_null_default (not_null_default<U> const& other)
        {
            *this = other;
        }

        template <typename U,
                  typename = std::enable_if_t<std::is_convertible<U, T>::value>>
        not_null_default& operator= (not_null_default<U> const& other)
        {
            ptr = other.get ();
            return *this;
        }

        not_null_default (std::nullptr_t)               = delete;
        not_null_default<T>& operator= (std::nullptr_t) = delete;
        
        not_null_default (int)               = delete;
        not_null_default<T>& operator= (int) = delete;
        
        T get (void) const noexcept { return ptr; }

        operator T (void)  const noexcept { return get (); }
        T operator->(void) const noexcept { return get (); }

        bool operator== (T const& rhs) const noexcept { return ptr == rhs; }
        bool operator!= (T const& rhs) const noexcept { return !(*this == rhs); }
    private:
        T ptr;

        void ensure_invariant (void) const
        {
            assert (ptr != nullptr && "not_null_default cannot be nullptr");
        }

        not_null_default<T>& operator++ (void) = delete;
        not_null_default<T>& operator-- (void) = delete;
        not_null_default<T>  operator++ (int)  = delete;
        not_null_default<T>  operator-- (int)  = delete;
        not_null_default<T>& operator+  (std::size_t) = delete;
        not_null_default<T>& operator+= (std::size_t) = delete;
        not_null_default<T>& operator-  (std::size_t) = delete;
        not_null_default<T>& operator-= (std::size_t) = delete;
    };
} // namesapce gsl

#endif // ifndef NOT_NULL_HPP
