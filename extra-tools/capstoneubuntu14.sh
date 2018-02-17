#!/bin/bash

cd $(dirname $0)

"wget" https://github.com/aquynh/capstone/archive/3.0.5-rc2.tar.gz -o capstone.tar.gz
"tar" -xvzf capstone.tar.gz
"cd" capstone*
"make"
sudo make install
"cd" ..
