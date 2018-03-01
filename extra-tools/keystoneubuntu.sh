#!/bin/bash

cd $(dirname $0)

"wget" https://github.com/keystone-engine/keystone/archive/0.9.1.tar.gz -o keystone.tar.gz
"tar" -xvzf keystone.tar.gz
"cd" keystone*
"mkdir" build
"cd" build
../make-share.sh
sudo make install
"cd" ../..
