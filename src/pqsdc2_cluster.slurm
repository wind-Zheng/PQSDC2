#!/bin/bash
#sbatch -p gpu1 -N 2 -c 1 /public/home/jd_sunhui/genCompressor/PQSDC/pqsdc_v1/pqsdc_v1_cluster.slurm
#sbatch -p gpu -N 2 -n 4 -c 2 --ntasks-per-node=2 pqsdc_v1_cluster.slurm
#SBATCH -o output.log         # 指定输出日志文件
#SBATCH -e error.log          # 指定错误日志文件

function Memory() {
  local name=$1
  local mem=$(cat ${name} | grep -o 'Maximum resident set size.*' | grep -o '[0-9]*')
  echo ${mem}
}
# 0 得到集群运行的一些参数
mode=$1                                            # 选择压缩模式、-c压缩；-d解压缩
parnum=$2                                               # 分区数量
threads=$3                                         # 单节点上OpenMP启用核心数目
fileName=$4                                       # 测试文件名称

nodelist=$(scontrol show hostname $SLURM_NODELIST) # 获得列表节点
#echo "nodelist: ${nodelist}"
echo "fileName: ${fileName}"
echo "threads per-node: ${threads}"
#echo "G: ${G}"
logName=$(pwd)/${fileName}.pqsdc_cluster.log
echo "log info:${logName}"
echo "log info:" >${logName}
P_W_D=$(pwd)

if [ ${mode} == '-c' ]; then
  echo "cluster compression mode"
  date1=$(date)
  date
  timestamp1=$(date -d "$date1" +%s)
  echo "1 主节点执行并行序列分区模型"
  first_node=$(echo $nodelist | awk '{print $1}')
  rm -rf ${fileName}.partition_all
  (srun -n 1 -w ${first_node} -c ${threads} /bin/time -v -p ../src/partition_all_ESort  -c ${parnum} ${threads} ${fileName}) &>${P_W_D}/${fileName}.partition_c.log

  nodes=($nodelist)
  num_processors=${SLURM_NNODES}
  echo "2 主机群运行分区映射、并将映射后数据分块"
  if [ ${SLURM_NNODES} -lt 1 ]; then
    echo "Nu must be grater than 0"
  else
    echo "  ${SLURM_NNODES} Node"
    for ((i = 0; i < num_processors; i++)); do
      for ((j = i; j < parnum; j += num_processors)); do
        echo "Processing file ${fileName}.partition_all/data_${i}.dat on processor node ${nodes[i]}"
        (srun -n 1 -c 4 -w ${nodes[i]} /bin/time -v -p ../src/pre -c ${fileName}.partition_all/data_${j}.dat) &>${P_W_D}/${fileName}.pre_${j}_c.log &
      done
    done
    wait
  fi

  echo "3 多节点执行预测计算"
  for ((i = 0; i < num_processors; i++)); do
    for ((j = i; j < parnum; j += num_processors)); do
      echo "Processing file ${fileName}.partition_all/data_${i}.dat.csv on processor node ${nodes[i]}"
      (srun -n 1 -w ${nodes[i]} /bin/time -v -p python ../src/mlp_test_torch.py ${fileName}.partition_all/data_${j}.dat.csv) &>${P_W_D}/${fileName}.mlp_${j}_c.log &
    done
  done
  wait
  for ((i = 0; i < num_processors; i++)); do
    for ((j = i; j < parnum; j += num_processors)); do
      echo "Processing file ${fileName}.partition_all/data_${i}.pre on processor node ${nodes[i]}"
      (srun -n 1 -c 4 -w ${nodes[i]} /bin/time -v -p  ../src/pqsdc2 -c ${parnum} ${threads} ${fileName}.partition_all/data_${j}.dat) &>${P_W_D}/${fileName}.mlp_${j}_c.log &
    done
  done
  wait
  
  echo "3 多节点执行ZPAQ并行压缩计算"
  pwdPath=$(pwd)
  cd ${fileName}.partition_all
  rm -rf *_key*
  files=$(ls -l *PQSDC2* | awk '{print $9}')
  files+=" partition_dat"
  files=($files)
  nodes=($nodelist)

  num_files=${#files[@]}
  num_processors=${SLURM_NNODES}
  num_files_per_processor=$((num_files / num_processors))
  for ((i = 0; i < num_processors; i++)); do
    for ((j = i; j < num_files; j += num_processors)); do
      echo "Processing file ${files[j]} on processor node ${nodes[i]}"
      (srun -n 1 -w ${nodes[i]} /bin/time -v -p zpaq a ${files[j]}.zpaq ${files[j]} -method 5 -threads ${threads}) &>${P_W_D}/${fileName}.zpaq_${files[j]}_c.log &
    done
  done
  wait
  echo "4 主节点执行打包操作"
  tar -cf result.pqsdc_v2 *.zpaq
  fsize=$(ls -lah --block-size=1 result.pqsdc_v2 | awk '/^[-d]/ {print $5}')
  echo "file-size:${fsize}"
  echo "5 删除所有中间结果文件"
  rm -rf *dat*
  cd ${pwdPath}
  # 遍历log文件
  cd ${P_W_D}
  files=$(ls -l ${fileName}*_c.log | awk '{print $9}')
  Max_mem=0
  for file in ${files}; do
    mem=$(Memory "${file}")
    if [ "${mem}" -gt "${Max_mem}" ]; then
      Max_mem=${mem}
    fi
    echo "${file}, Memory:${mem}"
  done
  echo "max-memory:${Max_mem}"

elif [ ${mode} == '-d' ]; then
  echo "cluster de-compression mode"
  prefix="${a%.partition_all}"
  echo "1 解压PQSDC2算法生成文件"
  pwdPath=$(pwd)
  cd ${fileName}
  (tar -xvf result.pqsdc_v2) &>>${logName}
  echo "2 使用zpaq算法解压缩文件"
  files=$(ls -l *.zpaq* | awk '{print $9}')
  files=($files)
  nodes=($nodelist)
  num_files=${#files[@]}
  num_processors=${SLURM_NNODES}
  num_files_per_processor=$((num_files / num_processors))
  for ((i = 0; i < num_processors; i++)); do
    for ((j = i; j < num_files; j += num_processors)); do
      echo "Processing file ${files[j]} on processor node ${nodes[i]}"
      (srun -n 1 -w ${nodes[i]} /bin/time -v -p zpaq x ${files[j]} -method 5 -threads ${threads}) &>${P_W_D}/${fileName}.zpaq_${files[j]}_d.log &
    done
  done
  wait
  echo "3 游程预测映射恢复原始文件、拼接恢原始文件"
  rm -rf *zpaq*
  for ((i = 0; i < num_processors; i++)); do
    for ((j = i; j < parnum; j += num_processors)); do
      echo "Processing file ${fileName}.partition_all/data_${i}.pre on processor node ${nodes[i]}"
      (srun -n 1 -c 4 -w ${nodes[i]} /bin/time -v -p ../../src/pqsdc2 -d ${parnum} ${threads} data_${j}.dat.PQSDC2) &>${P_W_D}/${fileName}.PQSDC2_${files[j]}_d.log &
    done
  done
  wait
  echo "4 合并分区恢复原始文件"
  cd ${pwdPath}
  (/bin/time -v -p ../src/partition_all_ESort -d ${parnum} ${threads} ${fileName}) &>${P_W_D}/${fileName}.partition_d.log
  #rm -rf ${fileName}
  cd ${pwdPath}
  wait
  #/public/home/jd_sunhui/genCompressor/SHcompressor/code/Compare ${prefix}.qualities  ${prefix}.qualities.PQVRC2_par_de
  #rm -rf ${file}.qualities.PQVRC2_par_de
  # 遍历log文件
  cd ${P_W_D}
  files=$(ls -l ${fileName}*_d.log | awk '{print $9}')
  Max_mem=0
  for file in ${files}; do
    mem=$(Memory "${file}")
    if [ "${mem}" -gt "${Max_mem}" ]; then
      Max_mem=${mem}
    fi
    echo "${file}, Memory:${mem}"
  done
  echo "max-memory:${Max_mem}"
else
  echo "wrong parameter!"
fi

# 根据时间戳计算时间开销
date2=$(date)
timestamp2=$(date -d "$date1" +%s)
diff=$((timestamp2 - timestamp1))
#echo "------------------------------------------------------------------"
date
echo "sacct -j $SLURM_JOB_ID -o JobID,JobName,State,Start,End,Elapsed,MaxRSS"
sacct -j $SLURM_JOB_ID -o "JobID,JobName,State,Start,End,Elapsed,MaxRSS"
#echo date1
#echo date2
#echo "时间差为：$diff 秒"
#echo "------------------------------------------------------------------"
exit 0
