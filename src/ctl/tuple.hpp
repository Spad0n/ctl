#ifndef CTL_TUPLE_HPP
#define CTL_TUPLE_HPP
#include "types.hpp"
#include "traits.hpp"
#include "move.hpp"
#include "forward.hpp"

namespace ctl {

    /// @brief A compile-time sequence of integers of type T.
    template<typename T, T... Vs>
    struct IntegerSequence {
	using Type = T;
	[[nodiscard]] static constexpr Ulen size() { return sizeof...(Vs); }
    };

    /// @brief A compile-time sequence of indices (Ulen).
    template<Ulen... Is>
    using IndexSequence = IntegerSequence<Ulen, Is...>;

#if CTL_HAS_BUILTIN(__make_integer_seq)
    // Clang (and MSVC) intrinsic.
    template<Ulen N>
    using MakeIndexSequence = __make_integer_seq<IntegerSequence, Ulen, N>;
#elif defined(CTL_COMPILER_GCC)
    // GCC intrinsic. Must be expanded inside a pack expansion.
    template<Ulen N>
    using MakeIndexSequence = IndexSequence<__integer_pack(N)...>;
#else
    // Portable recursive fallback.
    template<Ulen N, Ulen... Is>
    struct MakeIndexSequence_ : MakeIndexSequence_<N - 1, N - 1, Is...> {};
    template<Ulen... Is>
    struct MakeIndexSequence_<0, Is...> { using Type = IndexSequence<Is...>; };
    template<Ulen N>
    using MakeIndexSequence = typename MakeIndexSequence_<N>::Type;
#endif

    template<Ulen I, typename T>
    struct TupleLeaf {
	T value_;

	constexpr TupleLeaf() = default;

	// Parentheses (not braces) so implicit conversions are allowed and there
	// is no spurious narrowing diagnostic.
	template<typename U>
	constexpr TupleLeaf(U&& value)
            : value_(forward<U>(value))
	{}
    };

    // Recover the leaf at index I by template argument deduction. Because every
    // leaf carries a distinct index I, exactly one base matches, so this is
    // unambiguous and also recovers the element type T.
    template<Ulen I, typename T>
    [[nodiscard]] constexpr T& tuple_leaf(TupleLeaf<I, T>& leaf) { return leaf.value_; }
    template<Ulen I, typename T>
    [[nodiscard]] constexpr const T& tuple_leaf(const TupleLeaf<I, T>& leaf) { return leaf.value_; }

    template<typename Seq, typename... Ts>
    struct TupleStorage;

    template<Ulen... Is, typename... Ts>
    struct TupleStorage<IndexSequence<Is...>, Ts...> : TupleLeaf<Is, Ts>... {
	constexpr TupleStorage() = default;

	template<typename... Us>
	constexpr TupleStorage(Us&&... values)
            : TupleLeaf<Is, Ts>(forward<Us>(values))...
	{}
    };

    template<typename... Ts>
    using TupleStorageFor = TupleStorage<MakeIndexSequence<sizeof...(Ts)>, Ts...>;

    template<typename... Ts>
    struct Tuple;

    /// @brief The type of the element stored at index `I` of `Tuple<Ts...>`.
    template<Ulen I, typename... Ts>
    using TupleElement =
	RemoveReference<decltype(tuple_leaf<I>(declval<TupleStorageFor<Ts...>&>()))>;

    /// @brief A fixed-size, heterogeneous collection of values.
    ///
    /// Primarily useful to return several values from a function. Elements are
    /// accessed by compile-time index, via `get<I>()` (member or free function)
    /// or via structured bindings:
    ///
    /// @code
    /// Tuple<Sint32, Float64> minmax(Slice<const Float64> xs);
    /// auto [lo, hi] = minmax(values);   // structured bindings
    /// auto t = make_tuple(1, 2.0);      // deduces Tuple<Sint32, Float64>
    /// @endcode
    ///
    /// Copy/move semantics follow the contained types: the Tuple is move-only
    /// when any element is move-only (e.g. `Tuple<Array<Uint8>>`).
    template<typename... Ts>
    struct Tuple {
        /// @brief Number of elements in the tuple.
	[[nodiscard]] static constexpr Ulen length() { return sizeof...(Ts); }

	constexpr Tuple() = default;

        /// @brief Constructs the tuple from exactly one value per element.
        /// @note Disabled for the single-element copy/move case so it never
        ///       shadows the copy or move constructor.
	template<typename... Us>
            requires (sizeof...(Us) == sizeof...(Ts)
                      && sizeof...(Ts) >= 1
                      && !(sizeof...(Ts) == 1 && (Same<RemoveCVRef<Us>, Tuple> && ...)))
	constexpr Tuple(Us&&... values)
            : storage_(forward<Us>(values)...)
	{}

	constexpr Tuple(const Tuple&) = default;
	constexpr Tuple(Tuple&&) = default;
	constexpr Tuple& operator=(const Tuple&) = default;
	constexpr Tuple& operator=(Tuple&&) = default;

        /// @brief Element-wise assignment from a (possibly different) tuple.
        /// Enables `tie(a, b) = some_tuple;`.
	template<typename... Us>
            requires (sizeof...(Us) == sizeof...(Ts))
	constexpr Tuple& operator=(const Tuple<Us...>& other) {
            assign(other, MakeIndexSequence<sizeof...(Ts)>{});
            return *this;
	}

	template<typename... Us>
            requires (sizeof...(Us) == sizeof...(Ts))
	constexpr Tuple& operator=(Tuple<Us...>&& other) {
            assign(move(other), MakeIndexSequence<sizeof...(Ts)>{});
            return *this;
	}

        /// @brief Accesses the element at index `I`.
	template<Ulen I>
	[[nodiscard]] constexpr auto& get() & { return tuple_leaf<I>(storage_); }

	template<Ulen I>
	[[nodiscard]] constexpr const auto& get() const & { return tuple_leaf<I>(storage_); }

	template<Ulen I>
	[[nodiscard]] constexpr auto&& get() && {
            using Elem = TupleElement<I, Ts...>;
            return static_cast<Elem&&>(tuple_leaf<I>(storage_));
	}

    private:
	template<typename Other, Ulen... Is>
	constexpr void assign(Other&& other, IndexSequence<Is...>) {
            ((this->template get<Is>() = forward<Other>(other).template get<Is>()), ...);
	}

	TupleStorageFor<Ts...> storage_;
    };

    /// @brief Deduction guide so `Tuple{1, 'a'}` deduces `Tuple<Sint32, char>`.
    template<typename... Ts>
    Tuple(Ts...) -> Tuple<Ts...>;

    /// @brief Free-function element access (mirrors the member `get<I>()`).
    template<Ulen I, typename... Ts>
    [[nodiscard]] constexpr auto& get(Tuple<Ts...>& t) { return t.template get<I>(); }
    template<Ulen I, typename... Ts>
    [[nodiscard]] constexpr const auto& get(const Tuple<Ts...>& t) { return t.template get<I>(); }
    template<Ulen I, typename... Ts>
    [[nodiscard]] constexpr auto&& get(Tuple<Ts...>&& t) { return move(t).template get<I>(); }

    /// @brief Builds a tuple, decaying each argument to a value type.
    /// @code auto t = make_tuple(1, 2.5); // Tuple<Sint32, Float64> @endcode
    template<typename... Ts>
    [[nodiscard]] constexpr Tuple<RemoveCVRef<Ts>...> make_tuple(Ts&&... values) {
	return Tuple<RemoveCVRef<Ts>...>(forward<Ts>(values)...);
    }

    /// @brief Builds a tuple of lvalue references, for unpacking into existing
    /// variables without structured bindings.
    /// @code Sint32 a; Float64 b; tie(a, b) = minmax(values); @endcode
    template<typename... Ts>
    [[nodiscard]] constexpr Tuple<Ts&...> tie(Ts&... values) {
	return Tuple<Ts&...>(values...);
    }

    namespace detail {
	template<typename F, typename Tup, Ulen... Is>
	constexpr decltype(auto) apply_(F&& f, Tup&& t, IndexSequence<Is...>) {
            return forward<F>(f)(get<Is>(forward<Tup>(t))...);
	}
    }

    /// @brief Invokes `f` with the tuple elements as individual arguments.
    template<typename F, typename... Ts>
    constexpr decltype(auto) apply(F&& f, Tuple<Ts...>& t) {
	return detail::apply_(forward<F>(f), t, MakeIndexSequence<sizeof...(Ts)>{});
    }
    template<typename F, typename... Ts>
    constexpr decltype(auto) apply(F&& f, const Tuple<Ts...>& t) {
	return detail::apply_(forward<F>(f), t, MakeIndexSequence<sizeof...(Ts)>{});
    }
    template<typename F, typename... Ts>
    constexpr decltype(auto) apply(F&& f, Tuple<Ts...>&& t) {
	return detail::apply_(forward<F>(f), move(t), MakeIndexSequence<sizeof...(Ts)>{});
    }

} // namespace ctl

// Structured-bindings support.
//
// `auto [a, b] = some_tuple;` requires the standard "tuple protocol":
// std::tuple_size, std::tuple_element and a get<I>. We only forward-declare
// the two traits and add specialisations for ctl::Tuple, so no STL header is
// pulled in. Define CTL_TUPLE_NO_STRUCTURED_BINDINGS to opt out entirely.
#ifndef CTL_TUPLE_NO_STRUCTURED_BINDINGS
namespace std {
    template<typename T> struct tuple_size;
    template<decltype(sizeof 0) I, typename T> struct tuple_element;

    template<typename... Ts>
    struct tuple_size<ctl::Tuple<Ts...>> {
	static constexpr decltype(sizeof 0) value = sizeof...(Ts);
    };

    template<decltype(sizeof 0) I, typename... Ts>
    struct tuple_element<I, ctl::Tuple<Ts...>> {
	using type = ctl::TupleElement<I, Ts...>;
    };
}
#endif

#endif // CTL_TUPLE_HPP
