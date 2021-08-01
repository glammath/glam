set(LOCAL_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(LOCAL_LIB ${CMAKE_CURRENT_SOURCE_DIR}/lib)

set(EMCONFIGURE_ARGS --host none --prefix=${CMAKE_CURRENT_SOURCE_DIR})

add_custom_command(OUTPUT ${LOCAL_SRC}/gmp.dir
        COMMAND wget "https://gmplib.org/download/gmp/gmp-6.1.2.tar.lz"
        COMMAND mkdir -p gmp.dir
        COMMAND tar xf gmp-6.1.2.tar.lz -C gmp.dir --strip-components 1
        WORKING_DIRECTORY ${LOCAL_SRC}
        VERBATIM)

add_custom_target(gmp
        COMMAND emconfigure ./configure ${EMCONFIGURE_ARGS} --disable-assembly --enable-cxx
        COMMAND make
        COMMAND make install
        DEPENDS ${LOCAL_SRC}/gmp.dir
        WORKING_DIRECTORY ${LOCAL_SRC}/gmp.dir
        VERBATIM)

add_custom_command(OUTPUT ${LOCAL_SRC}/mpfr-4.1.0
        COMMAND wget "https://www.mpfr.org/mpfr-current/mpfr-4.1.0.tar.xz"
        COMMAND wget "https://www.mpfr.org/mpfr-current/allpatches"
        COMMAND tar xf mpfr-4.1.0.tar.xz
        WORKING_DIRECTORY ${LOCAL_SRC}
        VERBATIM)

add_custom_target(mpfr
        COMMAND patch -N -Z -p1 < ../allpatches
        COMMAND emconfigure ./configure ${EMCONFIGURE_ARGS} --with-gmp=${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND make
        COMMAND make install
        DEPENDS ${LOCAL_SRC}/mpfr-4.1.0 gmp
        WORKING_DIRECTORY ${LOCAL_SRC}/mpfr-4.1.0)

add_custom_command(OUTPUT ${LOCAL_SRC}/mpc.dir
        COMMAND wget "https://ftp.gnu.org/gnu/mpc/mpc-1.1.0.tar.gz"
        COMMAND mkdir -p mpc.dir
        COMMAND tar xf mpc-1.1.0.tar.gz -C mpc.dir --strip-components 1
        WORKING_DIRECTORY ${LOCAL_SRC}
        VERBATIM)

add_custom_target(mpc
        COMMAND emconfigure ./configure ${EMCONFIGURE_ARGS} --with-gmp=${CMAKE_CURRENT_SOURCE_DIR} --with-mpfr=${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND make
        COMMAND make install
        DEPENDS ${LOCAL_SRC}/mpc.dir mpfr
        WORKING_DIRECTORY ${LOCAL_SRC}/mpc.dir
        VERBATIM)

add_custom_target(binaryen
        COMMAND emcmake cmake -DBUILD_STATIC_LIB=ON
        COMMAND emmake make
        WORKING_DIRECTORY ${LOCAL_SRC}/binaryen
        VERBATIM)
