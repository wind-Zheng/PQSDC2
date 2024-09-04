#!/bin/bash
# This script is used to download the NCBI dataset,
# We provided you have Sratools installed.
# For Sratools information, see the https://github.com/NCBI/sra-tools

# dataSet Info:
# BGISEQ
# BGISEQ-500_Macaca_fascicularis_PE


dir_pwd=`pwd`
head="SRR8386"
data_set_name="BGISEQ-500_Macaca_fascicularis_PE"
cd ${data_set_name}
for index in `seq 199 206`
do
  prefetch ${head}${index}
  fastq-dump --split-files ${head}${index}
  cat ${head}${index}_1.fastq >>  ${data_set_name}.fastq
  cat ${head}${index}_2.fastq >>  ${data_set_name}.fastq
  rm -rf ${head}${index}
done
