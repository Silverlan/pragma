#ifndef __LUA_SHARED_PTR_CONVERTER_HPP__
#define __LUA_SHARED_PTR_CONVERTER_HPP__

// Obsolete
// TODO: Remove this file
#if 0
#include <boost/version.hpp>

#if BOOST_VERSION >= 105300

#include <boost/get_pointer.hpp>

// Source: https://stackoverflow.com/a/15207760
namespace luabind { namespace detail { namespace has_get_pointer_ {
    template<class T>
    T * get_pointer(std::shared_ptr<T> const& p) { return p.get(); }
}}}

#else // if BOOST_VERSION < 105300

#include <memory>

// Not standard conforming: add function to ::std(::tr1)
namespace std {

#if defined(_MSC_VER) && _MSC_VER < 1700
namespace tr1 {
#endif

    template<class T>
    T * get_pointer(shared_ptr<T> const& p) { return p.get(); }

#if defined(_MSC_VER) && _MSC_VER < 1700
} // namespace tr1
#endif

} // namespace std

#endif // if BOOST_VERSION < 105300
#endif

#endif
