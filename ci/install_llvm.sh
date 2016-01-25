pushd $HOME

if [[ "${TRAVIS_OS_NAME}" == "linux" ]]; then
   CMAKE_URL="http://www.cmake.org/files/v3.4/cmake-3.4.1-Linux-x86_64.tar.gz"
   mkdir cmake && wget --no-check-certificate --quiet -O - ${CMAKE_URL} | tar --strip-components=1 -xz -C cmake
   export PATH=${HOME}/cmake/bin:${PATH}
fi

export LLVM_VERSION="3.7.0"

LLVM_URL="https://github.com/llvm-mirror/llvm/archive/master.tar.gz"
LIBCXX_URL="https://github.com/llvm-mirror/libcxx/archive/master.tar.gz"
LIBCXXABI_URL="https://github.com/llvm-mirror/libcxxabi/archive/master.tar.gz"
TAR_FLAGS="-xz"

mkdir -p llvm llvm/build llvm/projects/libcxx llvm/projects/libcxxabi

wget --quiet -O - ${LLVM_URL} | tar --strip-components=1 ${TAR_FLAGS} -C llvm
wget --quiet -O - ${LIBCXX_URL} | tar --strip-components=1 ${TAR_FLAGS} -C llvm/projects/libcxx
wget --quiet -O - ${LIBCXXABI_URL} | tar --strip-components=1 ${TAR_FLAGS} -C llvm/projects/libcxxabi

(cd llvm/build && cmake .. -DCMAKE_INSTALL_PREFIX=${HOME}/local -DCMAKE_CXX_COMPILER=clang++-3.7)
(cd llvm/build/projects/libcxx && make install -j2)
(cd llvm/build/projects/libcxxabi && make install -j2)

popd
