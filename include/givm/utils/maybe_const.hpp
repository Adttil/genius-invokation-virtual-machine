#ifndef GIVM_UTILS_MAYBE_CONST_HPP
#define GIVM_UTILS_MAYBE_CONST_HPP

#include <type_traits>

namespace givm
{
    template<bool Mutable, class T>
    using maybe_mutable = std::conditional_t<Mutable, T, const T>;
}

#endif
