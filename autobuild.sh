#!/bin/bash

set -e

if [ ! -d `pwd`/build/ ]; then
    sudo mkdir `pwd`/build/
fi

sudo rm -rf `pwd`/build/*
cd `pwd`/build/ && 
    cmake .. &&
    make

cd ..

# 把头文件拷贝到 /usr/include/mymuduo 把so库拷贝到 /usr/lib，用于安装，这个路径在PATH中
if [ ! -d /usr/include/mymuduo/ ]; then
    sudo mkdir /usr/include/mymuduo/
fi

for header in `ls *.hpp`
do 
    cp $header /usr/include/mymuduo/
done

cp `pwd`/lib/libmymuduo.so /usr/lib/

# 刷新动态库缓存
ldconfig