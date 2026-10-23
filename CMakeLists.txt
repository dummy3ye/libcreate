cmake_minimum_required(VERSION 3.19)

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" LIBCREATE_VERSION)
string(STRIP "${LIBCREATE_VERSION}" LIBCREATE_VERSION)
project(libcreate VERSION "${LIBCREATE_VERSION}" LANGUAGES C)

include(GNUInstallDirs)
include(CTest)

option(LIBCREATE_WERROR "Treat compiler warnings as errors" OFF)
option(LIBCREATE_COVERAGE "Enable GCC/Clang coverage instrumentation" OFF)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(LIBCREATE_SOURCES src/create.c)
set(LIBCREATE_TEST_SOURCES
    tests/main.c
    tests/test.c
    tests/errors.c
    tests/parity.c
)

function(libcreate_configure_target target)
    target_include_directories("${target}"
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
    target_compile_definitions("${target}" PRIVATE
        _POSIX_C_SOURCE=200809L
        _FILE_OFFSET_BITS=64
    )
    if(APPLE)
        target_compile_definitions("${target}" PRIVATE _DARWIN_C_SOURCE)
    endif()
    target_compile_options("${target}" PRIVATE
        -Wall -Wextra -Wpedantic -Wshadow -Wconversion -O2
    )
    if(LIBCREATE_WERROR)
        target_compile_options("${target}" PRIVATE -Werror)
    endif()
    if(LIBCREATE_COVERAGE)
        target_compile_options("${target}" PRIVATE --coverage)
        target_link_options("${target}" PRIVATE --coverage)
    endif()
endfunction()

add_library(create_static STATIC ${LIBCREATE_SOURCES})
set_target_properties(create_static PROPERTIES OUTPUT_NAME create)
libcreate_configure_target(create_static)

add_library(create_shared SHARED ${LIBCREATE_SOURCES})
set_target_properties(create_shared PROPERTIES
    OUTPUT_NAME create
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"
    C_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
)
libcreate_configure_target(create_shared)

if(BUILD_TESTING)
    add_executable(test_static ${LIBCREATE_TEST_SOURCES})
    target_include_directories(test_static PRIVATE tests)
    target_compile_definitions(test_static PRIVATE
        VERSION_FILE="${CMAKE_CURRENT_SOURCE_DIR}/VERSION"
    )
    target_link_libraries(test_static PRIVATE create_static)
    libcreate_configure_target(test_static)
    add_test(NAME static COMMAND test_static)

    add_executable(test_shared ${LIBCREATE_TEST_SOURCES})
    target_include_directories(test_shared PRIVATE tests)
    target_compile_definitions(test_shared PRIVATE
        VERSION_FILE="${CMAKE_CURRENT_SOURCE_DIR}/VERSION"
    )
    target_link_libraries(test_shared PRIVATE create_shared)
    libcreate_configure_target(test_shared)
    add_test(NAME shared COMMAND test_shared)
endif()

set(PREFIX "prefix=${CMAKE_INSTALL_PREFIX}")
set(EXEC_PREFIX "exec_prefix=\${prefix}")
set(LIBDIR "libdir=\${exec_prefix}/${CMAKE_INSTALL_LIBDIR}")
set(INCLUDEDIR "includedir=\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}")
set(VERSION "${PROJECT_VERSION}")
configure_file(libcreate.pc.in "${CMAKE_CURRENT_BINARY_DIR}/libcreate.pc" @ONLY)

install(TARGETS create_static create_shared
    EXPORT libcreateTargets
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)
install(FILES include/create.h DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libcreate.pc"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
)

install(EXPORT libcreateTargets
    FILE libcreateTargets.cmake
    NAMESPACE libcreate::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/libcreate"
)
