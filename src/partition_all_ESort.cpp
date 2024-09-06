#include<stdio.h>
#include<iostream>
#include<string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include<iostream>
#include<fstream>
#include<math.h>
#include<vector>
#include<queue>
#include<map>
#include<bitset>
#include <omp.h>
#include<time.h>
#include <cmath>
#include <algorithm>
using namespace std;
void compress(std::string &inputfile,int parnum,int thread);
void decompress(std::string &inputfile,int parnum);
void ExternalSort(std::string &file1,std::string &file2,std::string &outfile);
int main(int argc, char** argv){
    string A = std::string(argv[4]);
    //string A = "/public/home/jd_sunhui/genCompressor/Data/110902_I244_FCC02FUACXX_L4_006SCELL03AEAAAPEI-12/110902_I244_FCC02FUACXX_L4_006SCELL03AEAAAPEI-12_2.fq";
    //string A = "/public/home/jd_sunhui/genCompressor/Data/NCBI/PhiX/PhiX_12.fastq";
    string method=argv[1];
    int parnum=std::stoi(argv[2]);
    int thread=std::stoi(argv[3]);
    if(method=="-c")
    {
//        old_compress(A,thread);
        compress(A,parnum,thread);
    }
    else if(method=="-d")
        decompress(A,parnum);
    //cout<<method<<"  "<<method.size()<<endl;
    //cout<<A<<endl;
    return 0;
}

void compress(std::string &inputfile,int parnum,int thread){
    string out_File = inputfile+".partition_all";
    string out_partition=out_File+"/partition_dat";
    if (access(out_File.c_str(), 0) == 0)
        rmdir(out_File.c_str());
    int isCreate = mkdir(out_File.c_str(),S_IRUSR | S_IWUSR | S_IXUSR | S_IRWXG | S_IRWXO);
    if( !isCreate)
        cout<<"create path:"<<out_File<<"\n";
    else
        cout<<"create path failed! \n";
    std::ifstream inFile;
    inFile.open(inputfile.c_str(), std::ios::in);
    if (!inFile) throw("Source_File_Wrong!");
    std::ofstream outpar;
    vector<std::ofstream*> outdata(parnum);
    for (int i = 0; i < parnum; i++) {
        string filename = out_File+"/data_"+ std::to_string(i) + ".dat";
        outdata[i] = new std::ofstream(filename);
    }

    outpar.open(out_partition.c_str(),std::ios::trunc|std::ios::binary);
    double Count_F=0;
    int M=0,len,k=2,l;
    string Qscore,kmer;

    //std::map<std::string, int>* maps = new std::map<std::string, int>[thread]; //根据线程束定义各个块的哈希表
    map<string,int> F;
    map<string,double> S;

    //获取文件序列条数以实现全采样
//    while(inFile.peek() != EOF)
//    {
//        getline(inFile,Qscore);
//        ++M;
//    }
//    //--M;
//    inFile.close();
//    inFile.open(inputfile.c_str(), std::ios::in);

    cout<<"t0:读入数据"<<endl;
    vector<string> a(100001);

    while(inFile.peek() != EOF)
    {
        l=0;
        while(inFile.peek() != EOF && l<100000)
        {
            getline(inFile,a[l]);
            ++l;
        }
        M+=l;
        for(int i=0;i<l/10;i++)
        {
            Qscore=a[i];
            for(int j=0;j<Qscore.size();j++)
            {
                kmer=Qscore.substr(j,k);
                ++F[kmer];
                ++Count_F;
            }
        }
    }
    cout<<"t1:采样完成!!!"<<endl;
    inFile.close();
    inFile.open(inputfile.c_str(), std::ios::in);


    //Phase 1
    double start,end,duration;
    int blockSize = M/thread;


    //Phase 2 计算各kmer所占比重
    auto iter=F.begin();
    while(iter!=F.end())
    {
        S[iter->first]=(double)iter->second/Count_F;
        ++iter;
    }
    cout<<"t2:kmer比重计算完成!!!"<<endl;


    start=omp_get_wtime();
    //Phase 3 计算所有特征值并寻找分区切割点
    string out_kmer=out_File+"/partition_key";

    int limit=(M+9)/10;
    vector<double> ev(limit,0),ei(100001,0);//ei表示单块序列数据结果,ev存储预设外部排序分块的特征值
    double Line_Weight;
    int begin=0,ordnum=0,pushed_num=0;//偏移量
    while(inFile.peek() != EOF)
    {
        l=0;
        while(inFile.peek() != EOF && l<100000)
        {
            getline(inFile,a[l]);
            ++l;
        }
        #pragma omp parallel for num_threads(thread),private(kmer,Line_Weight,Qscore)
        for(int i=0;i<l;i++)
        {
            Line_Weight=0;
            Qscore=a[i];
            for(int j=0;j<Qscore.size();j++)
            {
                kmer=Qscore.substr(j,k);
                Line_Weight+=S[kmer];
            }
            Line_Weight=Line_Weight/(double)Qscore.size();
            ei[i]=Line_Weight;
        }
        for(int i=0;i<l;i++)
        {
            ev[pushed_num++]=ei[i];
            if(pushed_num==limit)
            {
                sort(ev.begin(),ev.end());
                string out_kmer=out_File+"/partition_key_"+std::to_string(ordnum);
                std::ofstream outKmer(out_kmer,std::ios::binary);
                for(auto &num:ev)
                {
                    outKmer.write(reinterpret_cast<const char*>(&num),sizeof(num));
                }
                outKmer.close();
                pushed_num=0;
                ++ordnum;
            }
        }
    }
    if(pushed_num)
    {
        ev.resize(pushed_num);
        sort(ev.begin(),ev.end());
        string out_kmer=out_File+"/partition_key_"+std::to_string(ordnum);
        std::ofstream outKmer(out_kmer,std::ios::binary);
        for(auto &num:ev)
        {
        //cout<<num<<endl;

            outKmer.write(reinterpret_cast<const char*>(&num),sizeof(num));
            //outKmer<<'\n';
        }
        outKmer.close();
        pushed_num=0;
        ++ordnum;
    }
    end=omp_get_wtime();
    duration=(double)(end-start);
    cout<<"计算特征值消耗时间:"<<duration<<endl;

    inFile.close();
    inFile.open(inputfile.c_str(), std::ios::in);

    start=omp_get_wtime();
    //排序
    while(begin!=ordnum-1)
    {
        string k_file1=out_File+"/partition_key_"+std::to_string(begin);
        string k_file2=out_File+"/partition_key_"+std::to_string(begin+1);
        string k_file3=out_File+"/partition_key_"+std::to_string(ordnum);
        ExternalSort(k_file1,k_file2,k_file3);
        begin+=2;
        ordnum++;
    }

    vector<double> divline(parnum,0);
    string final_key= out_File+"/partition_key_"+std::to_string(ordnum-1);
    std::ifstream final_key_file(final_key, std::ios::binary);
    int limie=M/parnum;
    for(int i=1;i<parnum;i++)
    {
        pushed_num=limie;
        double div_num;
        while(pushed_num--)
        {
            final_key_file.read(reinterpret_cast<char*>(&div_num), sizeof(div_num));
            //cout<<div_num<<endl;
        }
        divline[i]=div_num;
        cout<<divline[i]<<endl;
    }


    end=omp_get_wtime();
    duration=(double)(end-start);
    cout<<"排序消耗时间:"<<duration<<endl;

    cout<<"t3:分区分割点寻找完成!!!"<<endl;

    start=omp_get_wtime();
    //Phase 4  输出分区文件
    int bitsize=log2(parnum*2-1),bs,parnumber,bslen=0;
    std::bitset<24> bits;
    begin=0;
    while(inFile.peek() != EOF)
    {
        l=0;
        while(inFile.peek() != EOF && l<100000)
        {
            getline(inFile,a[l]);
            ++l;
        }
        #pragma omp parallel for num_threads(thread),private(kmer,Line_Weight,Qscore)
        for(int i=0;i<l;i++)
        {
            Line_Weight=0;
            Qscore=a[i];
            for(int j=0;j<Qscore.size();j++)
            {
                kmer=Qscore.substr(j,k);
                Line_Weight+=S[kmer];
            }
            Line_Weight=Line_Weight/(double)Qscore.size();
            ei[i]=Line_Weight;
        }

        for(int i=0;i<l;i++)
        {
            for(int j=parnum-1;j>=0;j--)
            {
                if(ei[i]>=divline[j])
                {
                    parnumber=j;
                    break;
                }
            }
            bs=bitsize;
            while(bs--)
            {
                bits<<=1;
                bits[0]=(parnumber>>bs)%2;
            }
            bslen+=bitsize;

            *outdata[parnumber]<<a[i]<<'\n';
            if(bslen==24)
            {
                outpar.write(reinterpret_cast<const char*>(&bits), 3);
                bslen=0;
            }
        }
        begin+=l;
    }
    if(bslen)
    {
        bits<<=(24-bslen);
        outpar.write(reinterpret_cast<const char*>(&bits), 3);
        //cout<<bits<<endl;
    }
    //cout<<bitsize<<" "<<bslen<<endl;
    cout<<"t4:分区完成!!!"<<endl;
    end=omp_get_wtime();
    duration=(double)(end-start);
    cout<<"采用omp并行消耗时间:"<<duration<<endl;

    // 关闭文件和释放内存
    for (int i = 0; i < parnum; i++) {
        outdata[i]->close();
        delete outdata[i];
    }

    inFile.close();
    outpar.close();
    return ;
}


void decompress(std::string &inputfile,int parnum){
    string in_partition=inputfile+"/partition_dat";
    string out_File=inputfile.substr(0,inputfile.rfind('.'))+".PQSDC2_de";

    vector<std::ifstream*> indata(parnum);
    for (int i = 0; i < parnum; i++) {
        string filename = inputfile+"/data_"+ std::to_string(i) + ".dat.PQSDC2.PQde";
        indata[i] = new std::ifstream(filename);
    }
    cout<<"t0:准备工作"<<endl;
    std::ifstream inpartition;
    inpartition.open(in_partition.c_str(),std::ios::in|std::ios::binary);

    std::ofstream outputFile;
    outputFile.open(out_File.c_str(),std::ios::trunc);
    string Qscore;
    int c,k;
    int bitsize=log2(parnum*2-1),parnumber;
    std::bitset<24> bits;
    cout<<"t1:开始合并"<<endl;

    int pp=pow(2,bitsize);
    while(inpartition.peek() != EOF )
    {
        inpartition.read(reinterpret_cast<char*>(&bits),3);
        c=bits.to_ulong();
        k=24;
        while(k)
        {
            k-=bitsize;
            parnumber=(c>>k)%pp;
            if(indata[parnumber]->peek()!=EOF)
                getline(*indata[parnumber],Qscore);
            else
                break;
            outputFile<<Qscore<<'\n';
        }

    }
    //cout<<bits<<endl;
    cout<<"t2:合并完成"<<endl;
    // 关闭文件和释放内存
    for (int i = 0; i < parnum; i++) {
        indata[i]->close();
        delete indata[i];
    }
    inpartition.close();
    outputFile.close();

    return ;
}


void ExternalSort(std::string &file1,std::string &file2,std::string &outfile)
{
    std::ifstream inFile1(file1,std::ios::binary),inFile2(file2,std::ios::binary);
    std::ofstream outFile(outfile, std::ios::binary);
    std::ifstream inFile3(outfile, std::ios::binary);
    cout<<file1<<'\n'<<file2<<'\n'<<outfile<<endl<<endl<<endl;
    double num1,num2,num3;
    inFile1.read(reinterpret_cast<char*>(&num1), sizeof(num1));
    inFile2.read(reinterpret_cast<char*>(&num2), sizeof(num2));
    while(1)
    {
    //cout<<num1<<" "<<num2<<endl;
        if(num1<=num2)
        {
            num3=num1;
            //cout<< num3<<endl;
            //outF.write(reinterpret_cast<const char*>(&num),sizeof(num));
            outFile.write(reinterpret_cast<const char*>(&num3), sizeof(num3));
            //inFile3.read(reinterpret_cast<char*>(&num3), sizeof(num3));
            //cout<< num3<<endl;
            //outFile<<'\n';
            if(inFile1.peek()==EOF)
            {
                num3=num2;
                outFile.write(reinterpret_cast<const char*>(&num3), sizeof(num3));
                //outFile<<'\n';
                break;
            }
            inFile1.read(reinterpret_cast<char*>(&num1), sizeof(num1));
        }
        else
        {
            num3=num2;
            //cout<< num3<<" ";
            outFile.write(reinterpret_cast<const char*>(&num3), sizeof(num3));
//            inFile3.read(reinterpret_cast<char*>(&num3), sizeof(num3));
//            cout<< num3<<endl;
            //outFile<<'\n';
            if(inFile2.peek()==EOF)
            {
                num3=num1;
                outFile.write(reinterpret_cast<const char*>(&num3), sizeof(num3));
                //outFile<<'\n';
                break;
            }
            inFile2.read(reinterpret_cast<char*>(&num2), sizeof(num2));
        }
    }
    while(inFile1.peek()!=EOF)
    {
        inFile1.read(reinterpret_cast<char*>(&num1), sizeof(num1));
        num3=num1;
        outFile.write(reinterpret_cast<const char*>(&num3), sizeof(num3));
        //outFile<<'\n';
    }
    while(inFile2.peek()!=EOF)
    {
        inFile2.read(reinterpret_cast<char*>(&num2), sizeof(num2));
        num3=num2;
        outFile.write(reinterpret_cast<const char*>(&num3), sizeof(num3));
        //outFile<<'\n';
    }
    inFile1.close();
    inFile2.close();
    outFile.close();
    inFile3.close();
    return ;
}
