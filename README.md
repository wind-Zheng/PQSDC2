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
```
Finally, Configure the environment variables with the following command:
```shell script
export PATH=$PATH:`pwd`/
export PQSDC2_PATH="`pwd`/"
source ~/.bashrc
```

## Usage
```sh
    Basic Useage: PQSDC2.sh [command option]
       c [parnum] [threads] [qualities file]                     *compression mode.
       d [parnum] [threads] [pqsdc2 generate directory]          *decompression mode.
    Cluster Useage: pqsdc2_cluster.slurm [command option]
       -c [parnum] [threads] [qualities file]                     *compression mode.
       -d [parnum] [threads] [qualities file]                      *decompression mode.
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
#### 1、Using 8 CPU cores for compression.
```sh
cd ${PQSDC2_PATH}
cd ..
cd data
pqsdc2.sh c 4 8 test.qualities
```
results:
```sh
----------------------------------------------------------------------
Compression mode
FileName : test.qualities
Threads : 8
SavePath : test.qualities.paitition_all/result.pqsdc_v2
```
#### 2、Using 8 CPU cores for decompression.
```sh
pqsdc2.sh d 4 8 test.qualities.partition_all
```
results:
```sh
----------------------------------------------------------------------
Decompression mode
FileName : test.qualities.partition_all
Threads : 8
SavePath : test.qualities.PQSDC2_de
```
#### 3、Verify if the decompression is successful.
```sh
pqsdc_tools -verify test.fastq qualities test.qualities.PQSDC2_de
```
results:
```sh
lossless recover all qualities.
```
#### 4、Using 4 nodes and 6 threads for cluster parallel compression
```sh
sbatch -p gpu1 -N 4 -c 6 -n 4 -e cluster/test_4_c.err -o cluster/test_4_c.out ../src/pqsdc2_cluster.slurm -c 4 4 test.qualities
```
#### 4、Using 4 nodes and 6 threads for cluster parallel decompression
```sh
sbatch -p gpu1 -N 4 -c 6 -n 4 -e cluster/test_4_c.err -o cluster/test_4_c.out ../src/pqsdc2_cluster.slurm -d 4 4 test.qualities
```
## Our Experimental Configuration
Our experiment was conducted on the SUGON-7000A supercomputer system at the Nanning Branch of the National Supercomputing Center, using a queue of CPU/GPU heterogeneous computing nodes. The compute nodes used in the experiment were configured as follows: 
  
  2\*Intel Xeon Gold 6230 CPU (2.1Ghz, total 40 cores), 
  
  2\*NVIDIA Tesla-T4 GPU (16GB CUDA memory, 2560 CUDA cores), 
  
  512GB DDR4 memory, and 
  
  8\*900GB external storage.

## Dataset Acquisition
We experimentally evaluated using the real publicly available sequencing datasets from the NCBI database.
download this dataset by the following command(Refer to the .sh script in the data folder）:
```sh
nohup bash BGISEQ-500_Macaca_fascicularis_PE.sh > BGISEQ-500_Macaca_fascicularis_PE_download.log &
```
Dataset download and extraction using the `SRA-Tools：https://github.com/ncbi/sra-tools tool`.

## Acknowledgements
- Thanks to [@HPC-GXU](https://hpc.gxu.edu.cn) for the computing device support.   
- Thanks to [@NCBI](https://www.freelancer.com/u/Ostokhoon) for all available datasets.

## Additional Information

**Authors:**     NBJL-BioGrop.

**Contact us:**  https://nbjl.nankai.edu.cn OR zhengyf@nbjl.nankai.edu.cn OR sunh@nbjl.naikai.edu.cn
