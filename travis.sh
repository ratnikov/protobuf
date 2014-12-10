#!/bin/bash

set -e

install_root=/tmp/protobuf-test

install_deps() {
  sudo apt-get install ruby libruby1.9.1-dev
  sudo gem install update_rubygems
  pushd /var/lib/gems/1.9.1/bin
  sudo ./update_rubygems
  popd
  sudo gem install bundler rake rake-compiler rspec rubygems-tasks
}

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
    install_deps
    ;;
  test)
    #build_base
    #export PATH=$install_root/bin:$PATH
    test_ruby
    ;;
esac

exit 0
