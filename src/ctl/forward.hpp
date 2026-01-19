#ifndef CTL_FORWARD_HPP
#define CTL_FORWARD_HPP
#include "traits.hpp"

namespace ctl {

    template<typename T>
    constexpr T&& forward(RemoveReference<T>&& arg) {
	return static_cast<T&&>(arg);
    }

    template<typename T>
    constexpr T&& forward(RemoveReference<T>& arg) {
	return static_cast<T&&>(arg);
    }

} // namespace ctl

#endif // CTL_FORWARD_HPP
