#ifndef CTL_UNICODE_HPP
#define CTL_UNICODE_HPP
#include "types.hpp"
#include "slice.hpp"

namespace ctl {

    struct Rune {
	constexpr Rune(Uint32 v) : v_{v} {}
	[[nodiscard]] Bool is_char() const;
	[[nodiscard]] Bool is_digit() const;
	[[nodiscard]] Bool is_digit(Uint32 base) const;
	[[nodiscard]] Bool is_alpha() const;
	[[nodiscard]] Bool is_white() const;
    
        Ulen encode_utf8(Slice<Uint8> dest) const;

	operator Uint32() const { return v_; }
    private:
	Uint32 v_;
    };

} // namespace ctl

#endif // CTL_UNICODE
