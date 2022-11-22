#!/bin/bash -e

# Set the CLANG compilers to use if unset
if test -z "$CC" ; then
  export CC=clang
  export CXX=clang++
fi

export REPO_PATH=`pwd` ;
rm -rf build/ ; 
mkdir build ; 
cd build ; 
mkdir -p ${REPO_PATH}/inst ;
#cmake3 -DCMAKE_INSTALL_PREFIX="${REPO_PATH}/inst" -DCMAKE_BUILD_TYPE=Debug ../ ; 
cmake3 -DCMAKE_INSTALL_PREFIX="${REPO_PATH}/inst" -DCMAKE_BUILD_TYPE=Debug \
  -DMEMORYTOOL_DISABLE_LOOP_INVARIANT_STORE_OPT=ON \
  -DMEMORYTOOL_DISABLE_MOSTLY_LOOP_INVARIANT_STORE_OPT=ON \
  -DMEMORYTOOL_DISABLE_LOOP_INDUCTION_STORE_OPT=ON \
  -DMEMORYTOOL_DISABLE_BASE_ADDRESS_MOSTLY_INVARIANT_STORE_OPT=ON \
  -DMEMORYTOOL_DISABLE_PIN=OFF \
  -DMEMORYTOOL_DISABLE_ONLY_READ_OPT=ON \
  -DMEMORYTOOL_DISABLE_STORE_DFA_OPT=OFF \
  -DMEMORYTOOL_DISABLE_DIRECT_STATE=ON \
  -DMEMORYTOOL_DISABLE_SKIP_ROIS_LOADS_STORES=OFF \
  ../ ; 
make -j ;
make install ;
cd ../ 
