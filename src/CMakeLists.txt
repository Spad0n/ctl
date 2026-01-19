set(CTL_SOURCES
  allocator.cpp
  cpprt.cpp
  file.cpp
  pool.cpp
  slab.cpp
  stream.cpp
  string.cpp
  system.cpp
  unicode.cpp
)

add_library(ctl ${CTL_SOURCES})

target_include_directories(ctl
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(ctl PUBLIC cxx_std_20)

# in case there is some bugs
option(CTL_NO_STDLIBCXX "Disable C++ standard library" ON)

if(MSVC)
  target_compile_options(ctl
    PUBLIC
    /EHs-
    /GR-
  )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  # GCC / Clang
  target_compile_options(ctl PUBLIC
    -fno-exceptions
    -fno-rtti
  )
  if (CTL_NO_STDLIBCXX)
    target_link_options(ctl PUBLIC "-nostdlib++")
  endif()
endif()
