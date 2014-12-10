#!/bin/bash

set -e

install_root=/tmp/protobuf-test

build_base() {
  sh ./autogen.sh
  ./configure --prefix=$install_root
  make -j12
  make check -j12
  make install
}

test_ruby() {
  pushd ruby/
  rake test
  popd
}

case "$1" in
  install)
    ;;
  test)
    build_base
    export PATH=$install_root/bin:$PATH
    test_ruby
    ;;
esac

exit 0
