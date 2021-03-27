#!/usr/bin/env bash

git clone https://github.com/flat-os/limine.git --branch=trunk
cd limine && make toolchain && make && sudo make install && cd ..
cp limine/bin/limine.sys resources/limine.sys
