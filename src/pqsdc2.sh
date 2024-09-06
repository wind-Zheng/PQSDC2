#!/bin/bash
# ../src/PQVRC.sh [mode] [parnum] [threads] [fileName] 
echo "pqsdc2 algorithm"
mode=$1
parnum=$2
threads=$3
fileName=$4

echo $mode
echo $fileName
echo $threads

if [ "${mode}" = "c" ]; then
  echo "compression mode"
  
  # 记录开始时间
  start_time=$(date +%s)

  # 1 序列分区 生成${fileName}.partition文件
  ../src/partition_all_ESort -c ${parnum} ${threads} ${fileName}

  # 记录第一部分执行时间
  part1_end_time=$(date +%s)
  part1_duration=$((part1_end_time - start_time))
  echo "第一部分执行时间: ${part1_duration} 秒"

  # 2 进行分区文件进行游程预测映射
  pwdPath=$(pwd)

  for ((i = 0; i < parnum; i++)); do
    {
       ../src/pre -c ${fileName}.partition_all/data_${i}.dat
       python ../src/mlp_test_torch.py ${fileName}.partition_all/data_${i}.dat.csv
       ../src/pqsdc2 -c ${parnum} ${threads} ${fileName}.partition_all/data_${i}.dat
    } &
  done
  wait

  # 记录第二部分执行时间
  part2_end_time=$(date +%s)
  part2_duration=$((part2_end_time - part1_end_time))
  echo "第二部分执行时间: ${part2_duration} 秒"
  
  cd ${fileName}.partition_all	
  # 3 使用ZPAQ算法进行级联压缩
  for ((i = 0; i <= parnum; i++)); do
    {
      if (( i == parnum )); then
        zpaq a partition_dat.zpaq partition_dat -method 5 -threads ${threads}
      fi
      if (( i != parnum )); then
        zpaq a data_${i}.dat.PQVRC.zpaq data_${i}.dat.PQVRC -method 5 -threads ${threads}
      fi
    } &
  # 记录第三部分执行时间
  part3_end_time=$(date +%s)
  part3_duration=$((part3_end_time - part2_end_time))
  echo "第三部分执行时间: ${part3_duration} 秒"
  done
  wait
  
  # 4 打包为压缩文件
  mkdir temp
  mv *.zpaq temp/
  tar -cf result.pqsdc_v2 temp/
  ls -l --block-size=1 result.pqsdc_v2
  # 删除临时目录
  rm -r temp
  rm -rf *.dat*
  rm -r *key*
  # 5 删除所有文件
  #mv ${fileName%%.qualities}.pqsrc_v1 ${pwdPath}
  #rm -rf *dat*
  cd ${pwdPath}
  #rm -rf ${fileName}.partition
fi

if [ "${mode}" = "d" ]; then # 输入文件夹
  echo "de-compression mode"
  # 1 使用tar解包文件
  pwdPath=$(pwd)
  
  cd ${fileName}
  tar -xvf result.pqsdc_v2
  
  cd temp
  mv *.zpaq ../
  cd ..

  # 2 使用zpaq算法解压缩文件
  for ((i = 0; i <= parnum; i++)); do
    {
      if (( i == parnum )); then
        zpaq x partition_dat.zpaq -method 5 -threads ${threads}
      fi
      if (( i != parnum )); then
        zpaq x data_${i}.dat.PQVRC.zpaq -method 5 -threads ${threads}
      fi
    } &
  done
  wait

  # 3 进行分区文件进行游程预测映射
  for ((i = 0; i < parnum; i++)); do
    {
      ../../src/pqsdc2 -d ${parnum} ${threads} data_${i}.dat.PQVRC
    } &
  done
  wait
  # 4 合并分区恢复原始文件
  cd ..
  ../src/partition_all_ESort -d ${parnum} ${threads} ${fileName}
 

  rm -rf ${fileName}
  cd ${pwdPath}

fi
