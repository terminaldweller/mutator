#!/bin/bash

cd $(dirname $0)

"wget" https://github.com/aquynh/capstone/archive/3.0.5-rc2.tar.gz -o capstone.tar.gz
"tar" -xvzf 3.0.5-rc2.tar.gz
"cd" capstone-3.0.5-rc2
"make"
sudo make install
"cd" ..
