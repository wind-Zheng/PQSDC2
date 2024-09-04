# PQSDC2
![made-with-C++](https://img.shields.io/badge/Made%20with-C++11-brightgreen)
![made-with-OpenMP](https://img.shields.io/badge/Made%20with-OpenMP-blue)
![made-with-MPI](https://img.shields.io/badge/Made%20with-MPI-red)

## About The PQSDC 
PQSDC2 is an experimental open-source quality-score data compressor that leverages parallel sequential multipartitioning and a parallel redundant coding model based on deep learning networks to improve compression rates while minimising memory and time consumption. In addition, the compression process can be accelerated by using a multi-core CPU cluster to significantly reduce time overhead.

## Copy Our Project

Firstly, clone our tools from GitHub:
```shell script
git clone https://github.com/wind-Zheng/PQSDC2.git
```
Secondly, turn to src directory：
```shell script
cd PQSDC2/src
```
Thirdly, Run the following command：
```shell script
bash install.sh
#Warning!:GNU Make > 3.82.
```
Finally, Configure the environment variables with the following command:
```shell script
export PATH=$PATH:`pwd`/
export PQSDC_V2_PATH="`pwd`/"
source ~/.bashrc
```

## Usage
```sh
    Basic Useage: PQSDC2 [command option]
       -c [qualities file] [threads]                      *compression mode.
       -d [pqsdc2 generate directory] [threads]            *decompression mode.
    Advanced Usage:pqsdc_tools [command option]
       -fileinfo [input-fastq-file]                       *print basic statistic information.
       -dirinfo [input-dir-name]                          *print basic statistic information.
       -verify [source-fastq-file] <mode> [verify-file]   *verify decompression.
          <mode> = reads
          <mode> = qualities
       -filesplite [input-fastq-file] mode <mode>         *splite a FastQ file according <mode>.
          <mode> = ids
          <mode> = reads
          <mode> = describes
          <mode> = qualities
          <mode> = all
```

## Examples
We present the validation dataset `PQSDC2/data/test.qualities` 


