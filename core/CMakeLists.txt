cmake_minimum_required(VERSION 3.19)
project(glamcore)
set(CMAKE_CXX_STANDARD 17)

set(LOCAL_DIR ${PROJECT_SOURCE_DIR}/local)

set(EXPORT_HEADERS src/glam/multipoint.h src/glam/types.h src/glam/utilities.h)

if(DEFINED ENV{EMSDK})
    add_subdirectory(${LOCAL_DIR})

    include_directories(${LOCAL_DIR}/include ${LOCAL_DIR}/src/binaryen/src)

    add_executable(glamcore src/glam/types.cpp src/glam/multipoint.cpp src/glam/utilities.cpp src/glam/colors.h src/glam/morphemes.h src/glam/web/bindings.cpp src/glam/web/glamcore.cpp src/glam/fxn.cpp src/glam/jit/globals.cpp src/glam/jit/math_compiler.cpp src/glam/morphemes.cpp src/glam/builtins/elliptic.h)
    target_link_libraries(glamcore ${LOCAL_DIR}/lib/libgmp.a ${LOCAL_DIR}/lib/libmpc.a ${LOCAL_DIR}/lib/libmpfr.a ${LOCAL_DIR}/src/binaryen/lib/libbinaryen.a)


    set_target_properties(glamcore PROPERTIES
            COMPILE_FLAGS "--bind -s USE_BOOST_HEADERS=1"
            LINK_FLAGS "--bind -s USE_BOOST_HEADERS=1 --export-table --growable-table -s ALLOW_TABLE_GROWTH=1 -s ALLOW_MEMORY_GROWTH=1 -s EXPORTED_RUNTIME_METHODS=addFunction,ccall -s EXPORT_NAME=\"'GlamCore'\" --extern-pre-js ${PROJECT_SOURCE_DIR}/pre.js -s ENVIRONMENT=web")
else()
    add_library(glamcore SHARED src/glam/types.h src/glam/types.cpp src/glam/multipoint.cpp src/glam/multipoint.h src/glam/utilities.cpp src/glam/utilities.h)
    target_link_libraries(glamcore ${Boost_MULTIPRECISION_LIBRARY} ${Boost_NUMERIC_LIBRARY} gmp mpfr mpc)

    include(FetchContent)
    FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/53495a2a7d6ba7e0691a7f3602e9a5324bba6e45.zip
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()

    add_executable(glam_test test_src/test_utilities.cpp test_src/test_functions.cpp)
    target_include_directories(glam_test PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(glam_test glam gtest_main)

    include(GoogleTest)
    gtest_discover_tests(glam_test)
endif()

install(TARGETS glamcore
        DESTINATION ${PROJECT_SOURCE_DIR}/../ui/public/)
install(FILES ${CMAKE_BINARY_DIR}/glamcore.wasm DESTINATION ${PROJECT_SOURCE_DIR}/../ui/public/)

install(FILES ${EXPORT_HEADERS} DESTINATION ${LOCAL_DIR}/include/glam)
