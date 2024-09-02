#!/bin/bash

# 1 安装配置环境依赖https://github.com/zpaq/zpaq.git
# git clone https://github.com/zpaq/zpaq.git 这里已经下载完毕
cd zpaq
make
cd ..

# 2 编译PQSDC
g++ partition_all_ESort.cpp -std=c++11 -fopenmp -O3 -o partition_all_ESort
g++ pre.cpp -std=c++11 -fopenmp -O3 -o pre
g++ pqsdc2.cpp -std=c++11 -fopenmp -O3 -o pqsdc2
#g++ -std=c++11 -Wall -Werror -Wno-unused-variable -O3 -o pqsdc_tools pqsdc_tools.cpp tools.cpp message.cpp

# 3 配置环境(需要外部全局环境执行)
#export PATH=$PATH:`pwd`/
#export PQSDC_V2_PATH="`pwd`/"
#source ~/.bashrc

# 4 激活运行脚本
chmod +x pqsdc2.sh
