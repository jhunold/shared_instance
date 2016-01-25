#!/bin/bash

git config --global user.email "shared_instance@travis-ci.org"
git config --global user.name "Travis CI"

set -e


if [ ! -d "$HOME/boost/.git" ]; then
    echo "clone"
    pushd $HOME
    git clone --recursive  https://github.com/boostorg/boost.git
    popd
else
    echo "update"
    pushd $HOME/boost
    git pull
    git submodule update --recursive
    popd
fi

export BOOST_ROOT=$HOME/boost

pushd $HOME/boost && ./bootstrap.sh && ./b2 headers && popd

