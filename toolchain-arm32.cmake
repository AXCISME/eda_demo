set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch32)

# Avoid try-run checks when configuring for a foreign architecture.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

if(DEFINED ENV{CROSS_COMPILE} AND NOT "$ENV{CROSS_COMPILE}" STREQUAL "")
  set(_arm32_toolchain_prefix "$ENV{CROSS_COMPILE}")
else()
  set(_arm32_toolchain_prefix "arm-linux-gnueabihf")
endif()
string(REGEX REPLACE "-$" "" _arm32_toolchain_prefix "${_arm32_toolchain_prefix}")

set(
  ARM32_TOOLCHAIN_PREFIX
  "${_arm32_toolchain_prefix}"
  CACHE STRING
  "Cross compiler prefix without the trailing dash"
)

find_program(_arm32_c_compiler NAMES "${ARM32_TOOLCHAIN_PREFIX}-gcc")
find_program(_arm32_cxx_compiler NAMES "${ARM32_TOOLCHAIN_PREFIX}-g++")

if(NOT _arm32_c_compiler)
  message(FATAL_ERROR "Unable to find ${ARM32_TOOLCHAIN_PREFIX}-gcc in PATH")
endif()

if(NOT _arm32_cxx_compiler)
  message(FATAL_ERROR "Unable to find ${ARM32_TOOLCHAIN_PREFIX}-g++ in PATH")
endif()

set(CMAKE_C_COMPILER "${_arm32_c_compiler}" CACHE FILEPATH "C compiler" FORCE)
set(CMAKE_CXX_COMPILER "${_arm32_cxx_compiler}" CACHE FILEPATH "C++ compiler" FORCE)

execute_process(
  COMMAND "${_arm32_c_compiler}" -print-prog-name=ar
  OUTPUT_VARIABLE _arm32_compiler_ar
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
  COMMAND "${_arm32_c_compiler}" -print-prog-name=ranlib
  OUTPUT_VARIABLE _arm32_compiler_ranlib
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
  COMMAND "${_arm32_c_compiler}" -print-prog-name=strip
  OUTPUT_VARIABLE _arm32_compiler_strip
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

find_program(_arm32_ar NAMES
  "${ARM32_TOOLCHAIN_PREFIX}-gcc-ar"
  "${ARM32_TOOLCHAIN_PREFIX}-ar"
  "arm-none-linux-gnueabihf-gcc-ar"
  "arm-none-linux-gnueabihf-ar"
)
find_program(_arm32_ranlib NAMES
  "${ARM32_TOOLCHAIN_PREFIX}-gcc-ranlib"
  "${ARM32_TOOLCHAIN_PREFIX}-ranlib"
  "arm-none-linux-gnueabihf-gcc-ranlib"
  "arm-none-linux-gnueabihf-ranlib"
)
find_program(_arm32_strip NAMES
  "${ARM32_TOOLCHAIN_PREFIX}-strip"
  "arm-none-linux-gnueabihf-strip"
)

if(EXISTS "${_arm32_compiler_ar}")
  set(_arm32_ar "${_arm32_compiler_ar}")
endif()

if(EXISTS "${_arm32_compiler_ranlib}")
  set(_arm32_ranlib "${_arm32_compiler_ranlib}")
endif()

if(EXISTS "${_arm32_compiler_strip}")
  set(_arm32_strip "${_arm32_compiler_strip}")
endif()

if(_arm32_ar)
  set(CMAKE_AR "${_arm32_ar}" CACHE FILEPATH "Archiver" FORCE)
endif()

if(_arm32_ranlib)
  set(CMAKE_RANLIB "${_arm32_ranlib}" CACHE FILEPATH "Ranlib" FORCE)
endif()

if(_arm32_strip)
  set(CMAKE_STRIP "${_arm32_strip}" CACHE FILEPATH "Strip" FORCE)
endif()

# Use the sysroot bundled/configured with arm-linux-gnueabihf instead of a
# project-local sysroot path.
execute_process(
  COMMAND "${_arm32_c_compiler}" -print-sysroot
  OUTPUT_VARIABLE _arm32_compiler_sysroot
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(ARM32_SYSROOT "${_arm32_compiler_sysroot}" CACHE PATH "Sysroot reported by the arm32 cross toolchain" FORCE)

if(ARM32_SYSROOT AND EXISTS "${ARM32_SYSROOT}")
  set(CMAKE_SYSROOT "${ARM32_SYSROOT}" CACHE PATH "Global sysroot from cross compiler" FORCE)
  set(CMAKE_SYSROOT_COMPILE "${ARM32_SYSROOT}" CACHE PATH "Compile-time sysroot" FORCE)
  set(CMAKE_SYSROOT_LINK "${ARM32_SYSROOT}" CACHE PATH "Link-time sysroot" FORCE)
  set(CMAKE_FIND_ROOT_PATH "${ARM32_SYSROOT}" CACHE STRING "Cross compilation root path" FORCE)

  set(_arm32_multiarch_lib_dirs
    "${ARM32_SYSROOT}/lib/arm-linux-gnueabihf"
    "${ARM32_SYSROOT}/usr/lib/arm-linux-gnueabihf"
  )

  foreach(_arm32_lib_dir IN LISTS _arm32_multiarch_lib_dirs)
    if(EXISTS "${_arm32_lib_dir}")
      string(APPEND CMAKE_EXE_LINKER_FLAGS " -L${_arm32_lib_dir} -Wl,-rpath-link,${_arm32_lib_dir}")
      string(APPEND CMAKE_SHARED_LINKER_FLAGS " -L${_arm32_lib_dir} -Wl,-rpath-link,${_arm32_lib_dir}")
    endif()
  endforeach()

  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}" CACHE STRING "" FORCE)
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}" CACHE STRING "" FORCE)
else()
  message(STATUS "No explicit sysroot reported by ${ARM32_TOOLCHAIN_PREFIX}-gcc; using compiler default library search paths")
endif()

# Cross-compilation with STATIC_LIBRARY try-compile prevents CMake from
# accurately detecting Threads / implicit libraries. Provide results directly.
set(CMAKE_THREAD_LIBS_INIT "-lpthread" CACHE STRING "" FORCE)
set(Threads_FOUND TRUE CACHE BOOL "" FORCE)
set(CMAKE_USE_PTHREADS_INIT 1 CACHE BOOL "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lpthread" CACHE STRING "" FORCE)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
