#ifndef CTL_SYSTEM_HPP
#define CTL_SYSTEM_HPP
#include "string.hpp"

namespace ctl {
    struct Filesystem {
	struct File;
	struct Directory;

	struct Item {
            enum class Kind {
                FILE,
                LINK,
                DIR,
            };
            StringView name;
            Kind       kind;
	};

	enum class Access : Uint8 { RD, WR };
	static File* open_file(StringView name, Access access);
	static void close_file(File* file);
	static Uint64 read_file(File* file, Uint64 offset, Slice<Uint8> data);
	static Uint64 write_file(File* file, Uint64 offset, Slice<const Uint8> data);
	static Uint64 tell_file(File* file);

	Directory* open_dir(StringView name);
	void close_dir(Directory*);
	Bool read_dir(Directory*, Item& item);
    };

    struct Heap {
	static void *allocate(Ulen len, Bool zero);
	static void deallocate(void* addr, Ulen len);
    };

    struct Console {
	static void print(StringView data);
    };

    struct Linker {
	struct Library;
	Library* load(StringView name);
	void close(Library* library);
        void (*link(Linker::Library* lib, const char* symbol))(void);
    };
} // namespace ctl

#endif // CTL_SYSTEM_HPP
