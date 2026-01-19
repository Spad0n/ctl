#ifndef CTL_FILE_HPP
#define CTL_FILE_HPP
#include "array.hpp"
#include "maybe.hpp"
#include "system.hpp"

namespace ctl {

    struct File {
	using Access = Filesystem::Access;

	static Maybe<File> open(StringView name, Access access);

	constexpr File(File&& other)
            : file_{exchange(other.file_, nullptr)}
	{}

	//~File() { close(); }

	File& operator=(File&& other) {
            return *new (drop(), Nat{}) File{move(other)};
	}

	Uint64 read(Uint64 offset, Slice<Uint8> data) const;
	[[nodiscard]] Uint64 write(Uint64 offset, Slice<const Uint8> data) const;
	[[nodiscard]] Uint64 tell() const;

	void close();

	Array<Uint8> map(Allocator& allocator) const;

    private:
	File* drop() {
            close();
            return this;
	}
	constexpr File(Filesystem::File* file)
            : file_{file}
	{
	}
	Filesystem::File* file_;
    };

} // namespace ctl

#endif // CTL_FILE_HPP
