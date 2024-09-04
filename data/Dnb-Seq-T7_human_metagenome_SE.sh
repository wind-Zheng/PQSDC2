#!/bin/bash
# This script is used to download the NCBI dataset,
# We provided you have Sratools installed.
# For Sratools information, see the https://github.com/NCBI/sra-tools

# dataSet Info:
# Dnb-Seq
# Dnb-Seq-T7_human_metagenome_SE

dir_pwd=`pwd`
head="SRR165530"
data_set_name="Dnb-Seq-T7_human_metagenome_SE"
cd ${data_set_name}
for index in `seq 33 43`
do
  prefetch ${head}${index}
  fastq-dump --split-files ${head}${index}
  cat ${head}${index}_1.fastq >>  ${data_set_name}.fastq
  rm -rf ${head}${index}
done
head="SRR16553"
for index in `seq 111 117`
do
  prefetch ${head}${index}
  fastq-dump --split-files ${head}${index}
  cat ${head}${index}_1.fastq >>  ${data_set_name}.fastq
  rm -rf ${head}${index}
done
head="SRR16553"
for index in `seq 119 128`
do
  prefetch ${head}${index}
  fastq-dump --split-files ${head}${index}
  cat ${head}${index}_1.fastq >>  ${data_set_name}.fastq
  rm -rf ${head}${index}
done
head="SRR16553"
for index in `seq 130 139`
do
  prefetch ${head}${index}
  fastq-dump --split-files ${head}${index}
  cat ${head}${index}_1.fastq >>  ${data_set_name}.fastq
  rm -rf ${head}${index}
done
head="SRR16553"
for index in `seq 141 143`
do
  prefetch ${head}${index}
  fastq-dump --split-files ${head}${index}
  cat ${head}${index}_1.fastq >>  ${data_set_name}.fastq
  rm -rf ${head}${index}
done