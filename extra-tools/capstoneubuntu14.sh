#!/bin/bash

cd $(dirname $0)

"wget" https://github.com/aquynh/capstone/archive/3.0.5-rc2.tar.gz
"tar" -xvzf capstone-3.0.5-rc2.tar.gz
"cd" capstone-3.0.5-rc2.tar.gz
"make"
sudo make install
"cd" ..
