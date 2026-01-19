#ifndef CTL_EXCHANGE_H
#define CTL_EXCHANGE_H
#include "move.hpp"
#include "forward.hpp"

namespace ctl {

    template<typename T, typename U = T>
    CTL_FORCEINLINE constexpr T exchange(T& obj, U&& new_value) {
	T old_value = move(obj);
	obj = forward<U>(new_value);
	return old_value;
    }

} // namespace Ctl

#endif // CTL_EXCHANGE_H
