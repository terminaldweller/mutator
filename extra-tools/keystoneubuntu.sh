#!/bin/bash

cd $(dirname $0)

"wget" https://github.com/keystone-engine/keystone/archive/0.9.1.tar.gz -o keystone.tar.gz
"tar" -xvzf 0.9.1.tar.gz
"cd" keystone-0.9.1
"mkdir" build
"cd" build
../make-share.sh
sudo make install
"cd" ../..
