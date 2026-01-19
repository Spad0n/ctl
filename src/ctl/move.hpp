#ifndef CTL_MOVE_HPP
#define CTL_MOVE_HPP
#include "traits.hpp"

namespace ctl {

    template<typename T>
    CTL_FORCEINLINE constexpr RemoveReference<T>&& move(T&& arg) {
	return static_cast<RemoveReference<T>&&>(arg);
    }

} // namespace ctl

#endif // CTL_MOVE_HPP
