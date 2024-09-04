#!/bin/bash
# This script is used to download the NCBI dataset,
# We provided you have Sratools installed.
# For Sratools information, see the https://github.com/NCBI/sra-tools

# dataSet Info:
# MgiSeq
# MgiSeq-2000RS_Mus-musculus_PE


dir_pwd=`pwd`
head="SRR17794"
data_set_name="MgiSeq-2000RS_Mus-musculus_PE"
cd ${data_set_name}
for index in 724 733 735 741 745 766 873 879 925 926 960
do
  prefetch ${head}${index}
  fastq-dump --split-files ${head}${index}
  #PLRC -filesplite ${head}${index}_1.fastq mode qualities
  #PLRC -filesplite ${head}${index}_2.fastq mode qualities
  cat ${head}${index}_1.fastq >>  ${data_set_name}.fastq
  cat ${head}${index}_2.fastq >>  ${data_set_name}.fastq
  rm -rf ${head}${index}
done
