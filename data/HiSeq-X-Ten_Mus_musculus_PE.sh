#!/bin/bash
# This script is used to download the NCBI dataset,
# We provided you have Sratools installed.
# For Sratools information, see the https://github.com/NCBI/sra-tools

# dataSet Info:
# HiSeq-X-Ten_Mus_musculus_PE

dir_pwd=`pwd`
head="SRR69565"
data_set_name="HiSeq-X-Ten_Mus_musculus_PE"
mkdir ${data_set_name}
cd ${data_set_name}
for index in `seq 17 19`
do
  prefetch ${head}${index}
  fastq-dump --split-files ${head}${index}
  cat ${head}${index}_1.fastq >>  ${data_set_name}.fastq
  cat ${head}${index}_2.fastq >>  ${data_set_name}.fastq
  rm -rf ${head}${index}
done
for index in `seq 21 22`
do
  prefetch ${head}${index}
  fastq-dump --split-files ${head}${index}
  cat ${head}${index}_1.fastq >>  ${data_set_name}.fastq
  cat ${head}${index}_2.fastq >>  ${data_set_name}.fastq
  rm -rf ${head}${index}
done