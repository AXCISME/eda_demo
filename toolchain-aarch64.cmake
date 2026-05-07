set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Avoid try-run checks when configuring for a foreign architecture.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

if(DEFINED ENV{CROSS_COMPILE} AND NOT "$ENV{CROSS_COMPILE}" STREQUAL "")
  set(_smartwell_toolchain_prefix "$ENV{CROSS_COMPILE}")
else()
  set(_smartwell_toolchain_prefix "aarch64-none-linux-gnu")
endif()
string(REGEX REPLACE "-$" "" _smartwell_toolchain_prefix "${_smartwell_toolchain_prefix}")

set(
  SMARTWELL_TOOLCHAIN_PREFIX
  "${_smartwell_toolchain_prefix}"
  CACHE STRING
  "Cross compiler prefix without the trailing dash"
)

if(NOT DEFINED SMARTWELL_SYSROOT OR SMARTWELL_SYSROOT STREQUAL "")
  set(_smartwell_default_sysroot "/root/workspace/gittest/cat_sysroot")
  set(
    SMARTWELL_SYSROOT
    "${_smartwell_default_sysroot}"
    CACHE PATH
    "Sysroot used by the aarch64 cross toolchain"
  )
endif()

if(NOT EXISTS "${SMARTWELL_SYSROOT}")
  message(FATAL_ERROR "SMARTWELL_SYSROOT does not exist: ${SMARTWELL_SYSROOT}")
endif()

find_program(_smartwell_c_compiler NAMES "${SMARTWELL_TOOLCHAIN_PREFIX}-gcc")
find_program(_smartwell_cxx_compiler NAMES "${SMARTWELL_TOOLCHAIN_PREFIX}-g++")

if(NOT _smartwell_c_compiler)
  message(FATAL_ERROR "Unable to find ${SMARTWELL_TOOLCHAIN_PREFIX}-gcc in PATH")
endif()

if(NOT _smartwell_cxx_compiler)
  message(FATAL_ERROR "Unable to find ${SMARTWELL_TOOLCHAIN_PREFIX}-g++ in PATH")
endif()

set(CMAKE_C_COMPILER "${_smartwell_c_compiler}" CACHE FILEPATH "C compiler" FORCE)
set(CMAKE_CXX_COMPILER "${_smartwell_cxx_compiler}" CACHE FILEPATH "C++ compiler" FORCE)

# Use external sysroot for both compilation and linking so that
# runtime libraries (pthread etc.) are resolved properly.
set(CMAKE_SYSROOT "" CACHE PATH "Global sysroot is disabled intentionally" FORCE)
set(CMAKE_SYSROOT_COMPILE "${SMARTWELL_SYSROOT}" CACHE PATH "Compile-time sysroot" FORCE)
set(CMAKE_SYSROOT_LINK "${SMARTWELL_SYSROOT}" CACHE PATH "Link-time sysroot" FORCE)

# Add multiarch library directories from sysroot that --sysroot alone does not cover
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${SMARTWELL_SYSROOT}/lib/aarch64-linux-gnu -L${SMARTWELL_SYSROOT}/usr/lib/aarch64-linux-gnu" CACHE STRING "" FORCE)
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -L${SMARTWELL_SYSROOT}/lib/aarch64-linux-gnu -L${SMARTWELL_SYSROOT}/usr/lib/aarch64-linux-gnu" CACHE STRING "" FORCE)

# Rpath-link allows the linker to resolve transitive shared library dependencies
# (e.g. toolchain's libstdc++.so → sysroot libm.so.6) through the multiarch paths
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath-link,${SMARTWELL_SYSROOT}/lib/aarch64-linux-gnu -Wl,-rpath-link,${SMARTWELL_SYSROOT}/usr/lib/aarch64-linux-gnu" CACHE STRING "" FORCE)

# Cross-compilation with STATIC_LIBRARY try-compile prevents CMake from
# accurately detecting Threads / implicit libraries. Provide results directly.
set(CMAKE_THREAD_LIBS_INIT "-lpthread" CACHE STRING "" FORCE)
set(Threads_FOUND TRUE CACHE BOOL "" FORCE)
set(CMAKE_USE_PTHREADS_INIT 1 CACHE BOOL "" FORCE)

# Ensure -lpthread is on the link line even if FindThreads resolves to empty
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lpthread" CACHE STRING "" FORCE)

find_program(_smartwell_ar NAMES "${SMARTWELL_TOOLCHAIN_PREFIX}-gcc-ar" "${SMARTWELL_TOOLCHAIN_PREFIX}-ar")
find_program(_smartwell_ranlib NAMES "${SMARTWELL_TOOLCHAIN_PREFIX}-gcc-ranlib" "${SMARTWELL_TOOLCHAIN_PREFIX}-ranlib")
find_program(_smartwell_strip NAMES "${SMARTWELL_TOOLCHAIN_PREFIX}-strip")

if(_smartwell_ar)
  set(CMAKE_AR "${_smartwell_ar}" CACHE FILEPATH "Archiver" FORCE)
endif()

if(_smartwell_ranlib)
  set(CMAKE_RANLIB "${_smartwell_ranlib}" CACHE FILEPATH "Ranlib" FORCE)
endif()

if(_smartwell_strip)
  set(CMAKE_STRIP "${_smartwell_strip}" CACHE FILEPATH "Strip" FORCE)
endif()

set(CMAKE_FIND_ROOT_PATH "${SMARTWELL_SYSROOT}" CACHE STRING "Cross compilation root path" FORCE)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
