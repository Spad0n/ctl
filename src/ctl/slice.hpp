#ifndef CTL_SLICE_HPP
#define CTL_SLICE_HPP
#include "types.hpp"
#include "exchange.hpp"
#include "allocator.hpp"

namespace ctl {

    // Slice is a convenience type around a pointer and a length. Think of it like a
    // span or a view. Specialization for T = const char is implicitly provided so
    // that Slice<const char> is the same as StringView.
    template<typename T>
    struct Slice {
	constexpr Slice() = default;
	constexpr Slice(T* data, Ulen length)
            : data_{data}
            , length_{length}
	{}

	template<Ulen E>
	constexpr Slice(T (&data)[E])
            : data_{data}
            , length_{E}
	{
            if constexpr (is_same<T, const char>) {
                length_--;
            }
	}

        constexpr Slice(const char* cstr) requires (is_same<T, const char>)
            : data_(cstr)
            , length_{0}
        {
            if (data_) {
                while (data_[length_] != '\0') {
                    length_++;
                }
            }
        }

        //constexpr Slice(Allocator& allocator, Ulen length)
        //    : data_{allocator.allocate<T>(length, true)}
        //    , length_{length}
        //{}

        //void deinit(Allocator& allocator) {
        //    allocator.deallocate(data_, length_);
        //    data_ = nullptr;
        //    length_ = 0;
        //}

	constexpr Slice(const Slice& other)
            : data_{other.data_}
            , length_{other.length_}
	{}

	constexpr Slice& operator=(const Slice&) = default;
	constexpr Slice(Slice&& other)
            : data_{exchange(other.data_, nullptr)}
            , length_{exchange(other.length_, 0)}
	{}

	[[nodiscard]] CTL_FORCEINLINE constexpr T& operator[](Ulen index) { return data_[index]; }
	[[nodiscard]] CTL_FORCEINLINE constexpr const T& operator[](Ulen index) const { return data_[index]; }

	constexpr Slice slice(Ulen offset) const {
            return Slice{data_ + offset, length_ - offset};
	}
	constexpr Slice truncate(Ulen length) const {
            return Slice{data_, length};
	}
        constexpr Slice subrange(Ulen start, Ulen end_excluded) const {
            return slice(start).truncate(end_excluded - start);
        }

	// Just enough to make range based for loops work
	[[nodiscard]] CTL_FORCEINLINE constexpr T* begin() { return data_; }
	[[nodiscard]] CTL_FORCEINLINE constexpr const T* begin() const { return data_; }
	[[nodiscard]] CTL_FORCEINLINE constexpr T* end() { return data_ + length_; }
	[[nodiscard]] CTL_FORCEINLINE constexpr const T* end() const { return data_ + length_; }

	[[nodiscard]] CTL_FORCEINLINE constexpr auto length() const { return length_; }
	[[nodiscard]] CTL_FORCEINLINE constexpr auto is_empty() const { return length_ == 0; }

	[[nodiscard]] CTL_FORCEINLINE constexpr T* data() { return data_; }
	[[nodiscard]] CTL_FORCEINLINE constexpr const T* data() const { return data_; }

	template<typename U>
	CTL_FORCEINLINE Slice<U> cast() {
            const auto ptr = reinterpret_cast<U*>(data_);
            return Slice<U> { ptr, (length_ * sizeof(T)) / sizeof(U) };
	}

	template<typename U>
	CTL_FORCEINLINE Slice<const U> cast() const {
            const auto ptr = reinterpret_cast<const U*>(data_);
            return Slice<const U> { ptr, (length_ * sizeof(T)) / sizeof(U) };
	}

	[[nodiscard]] friend constexpr Bool operator==(const Slice& lhs, const Slice& rhs) {
            const auto lhs_len = lhs.length();
            const auto rhs_len = rhs.length();
            if (lhs_len != rhs_len) {
                return false;
            }
            if (lhs.data() == rhs.data()) {
                return true;
            }
            for (Ulen i = 0; i < lhs_len; i++) {
                if (lhs[i] != rhs[i]) {
                    return false;
                }
            }
            return true;
	}

    private:
	T*   data_   = nullptr;
	Ulen length_ = 0;
    };
    
    template<typename T>
    Slice<T> make_slice(Allocator& alloc, Ulen count, Bool zero = false) {
        T* ptr = alloc.allocate<T>(count, zero);

        if constexpr(!TriviallyDestructible<T>) {
            for (Ulen i = 0; i < count; i++) {
                new (ptr + i) T();
            }
        }

        return Slice<T>(ptr, count);
    }

    template<typename T>
    void free_slice(Allocator& alloc, Slice<T> slice) {
        if (slice.data() == nullptr) return;

        if constexpr(!TriviallyDestructible<T>) {
            for (Ulen i = 0; i < slice.length(); i++) {
                slice[i].~T();
            }
        }

        alloc.deallocate(slice.data(), slice.length());
    }

} // namespace ctl

#endif // CTL_SLICE_HPP
