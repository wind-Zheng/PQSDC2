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
#include<bitset>
#include<queue>
#include<map>
#include<bitset>
#include <omp.h>
#include<time.h>
#include <stdexcept>

using namespace std;



void compress(std::string &inputfile,int parnum,int thread);


void decompress(std::string &inputfile);
string lcqs(std::string &Qscore);
int lcqs1(std::string &Qscore,vector<int> &outstring);
string rle_lcqs(std::string &Qscore);
int rle_lcqs1(std::string &Qscore,vector<int> &outstring);
string de_lcqs(std::string &instring,int lens);
string de_rle_lcqs(std::string &instring,int lens);


int main(int argc, char** argv){
    string A = std::string(argv[4]);
    //string A = "/public/home/jd_sunhui/genCompressor/Data/110902_I244_FCC02FUACXX_L4_006SCELL03AEAAAPEI-12/110902_I244_FCC02FUACXX_L4_006SCELL03AEAAAPEI-12_2.fq";
    //string A = "/public/home/jd_sunhui/genCompressor/Data/NCBI/PhiX/PhiX_12.fastq";
    string method=argv[1];
    int parnum=std::stoi(argv[2]);
    int thread=std::stoi(argv[3]);
    double start,end,duration;
    cout<<method<<" "<<thread<<" "<<A<<endl;
    if(method=="-c")
    {
//        start=omp_get_wtime();
//        old_compress(A);
//        end=omp_get_wtime();
//        duration=(double)(end-start);
//        cout<<"未采用omp并行消耗时间:"<<duration<<endl;

        start=omp_get_wtime();
        compress(A,parnum,thread);
        end=omp_get_wtime();
        duration=(double)(end-start);
        cout<<"采用omp并行消耗时间:"<<duration<<endl;
    }
    else if(method=="-d")
    {
        cout<<"decompress:"<<endl;
        decompress(A);
    }

    //cout<<method<<"  "<<method.size()<<endl;
    //cout<<A<<endl;
    return 0;
}
void compress(std::string &inputfile,int parnum,int thread){
    string pre_File = inputfile+".csv.pre";
    string out_data = inputfile+".PQSDC2";
    std::ifstream inFile,inPre;
    inFile.open(inputfile.c_str(), std::ios::in);
    inPre.open(pre_File.c_str(), std::ios::in);
    if (!inFile) throw("Source_File_Wrong!");
    std::ofstream out_char_File;
    out_char_File.open(out_data.c_str(),std::ios::trunc|std::ios::binary);

    double Count_F=0;
    int M=0,len,k=4;
    string Qscore,kmer,s;
    int num,qs_num;
    double start,end;

    start=omp_get_wtime();
    //Phase 4
    int Pre_begin=0;
    string Pre;
    getline(inPre,Pre);
    //cout<<"pre.size():"<<Pre.size()<<endl;


    getline(inFile,Qscore);
    len=Qscore.size();
    out_char_File<<len<<'\n';
    inFile.close();
    inFile.open(inputfile.c_str(), std::ios::in);

    int num1=0,num2=0;
    vector<string> a(10001),outs1(10001);
    vector<vector<int>> outs(10001,vector<int>(len*100+1,0));
    vector<int> ret_lens(10001,0);
    vector<vector<string>> apar(parnum,vector<string>(10001,""));
    vector<int> qs_nums(parnum,0),num_par(parnum,0);
   // StringBuffer sb[parnum],out_sb;
   // uint32_t cur[parnum+1]{0}, pre[parnum]{}, eline[parnum]{};
    while(inFile.peek()!=EOF)
    {
        qs_num=0;
//        cout<<1<<endl;
        //拼接成一条
        while(inFile.peek() != EOF && qs_num<10000)
        {
            a[qs_num]="";
            num=0;
            while(inFile.peek() != EOF && num<100)
            {
                getline(inFile,s);
                a[qs_num]+=s;
                ++num;
            }
            //outs[qs_num]=a[qs_num];
            ++qs_num;
            //cout<<outs[qs_num].size()<<endl     ;
        }
//        cout<<2<<endl;
//
//        cout<<qs_num<<endl;
//        double start,end,duration;
//        start=omp_get_wtime();

        #pragma omp parallel for num_threads(thread)
        for(int i=0;i<qs_num;i++)
        {
            string Qscore=a[i];
            string outss="";

            if(Pre[Pre_begin+i]=='1')
            {
                ret_lens[i]=rle_lcqs1(Qscore,outs[i]);
                //outs[i]=rle_lcqs(Qscore);
                #pragma omp critical
                    {
                        num2++;
                    }
            }
            else
            {
                ret_lens[i]=lcqs1(Qscore,outs[i]);
                //outs[i]=lcqs(Qscore);
                #pragma omp critical
                    {
                        num1++;
                    }
            }
        }
        Pre_begin+=qs_num;

        for(int i=0;i<qs_num;i++)
        {
            //out=outs[i];
            int len1=ret_lens[i];
            //cout<<len<<endl;
            //int len1=out.size();
            std::bitset<8> bits;
            unsigned char b;
            for(int j=0;j<len1;j++)
            {
                b=outs[i][j];
                bits = std::bitset<8>(b);
                out_char_File.write(reinterpret_cast<char*>(&bits), 1);
            }
            bits=std::bitset<8> (0);
            out_char_File.write(reinterpret_cast<char*>(&bits), 1);
        }

    }

    cout<<num1<<" "<<num2<<endl;




    inFile.close();
    inPre.close();
    out_char_File.close();
    return ;
}



void decompress(std::string &inputfile){
    string in_data = inputfile;
    string out_File=inputfile+".PQde";
    //.substr(0,inputfile.rfind('.'))
    std::ifstream in_data_File;
    in_data_File.open(in_data.c_str(), std::ios::in|std::ios::binary);
    std::ofstream outputFile;
    outputFile.open(out_File.c_str(),std::ios::trunc);
    string Qscore,instring;
    getline(in_data_File,instring);
    int lens =atoi(instring.c_str());//>0 ? instring[0] : instring[0]+256;
    //cout<<lens;
    instring="";
    std::bitset<8> bits;
    int count=0,C=0,len=0,k=7,c;
    while(in_data_File.peek() != EOF)
    {
        in_data_File.read(reinterpret_cast<char*>(&bits),1);
        c=bits.to_ulong();

        if(c)
            instring+=c;
        else if(c==0)
        {
            if(instring[0]>0&&instring[0]<=128)
                Qscore=de_lcqs(instring,lens);
            else
                Qscore=de_rle_lcqs(instring,lens);
            outputFile<<Qscore;
            instring="";
        }
        //getline(in_data_File,instring);


    }
    in_data_File.close();
    outputFile.close();

    return ;
}

int lcqs1(std::string &Qscore,vector<int> &outstring){
    int C,pre;//参考数C和用于找连续串的pre
    int len=0,num=0,begin,n,c,k,j;
    vector<int> ct(128,0);//统计找众数
    //string outstring="";
    pre=0;begin=0;n=Qscore.size();num=0;C=0;
    //统计众数C并且统计RLE
        for(int i=0;i<n;i++)
        {
            c=Qscore[i];
            ct[c]++;
            if(ct[c]>=ct[C])
                C=c;
        }
        outstring[len++]=C;
        for(int i=0;i<n;i++)
        {
            c=Qscore[i];
            if(c==C)
            {
                j=i+1;
                while(j<n && Qscore[j]==C)
                {
                    ++j;
                }
                while(j-i>54)
                {   outstring[len++]=255;i+=54;}
                if(j-i>0)
                    outstring[len++]=201+(j-i);
                i=j-1;continue;
            }

            if(i<n-2 && (Qscore[i]>=C-3 && Qscore[i]<=C) && (Qscore[i+1]>=C-3 && Qscore[i+1]<=C) && (Qscore[i+2]>=C-3 && Qscore[i+2]<=C))
            {
                outstring[len++]=(Qscore[i]-C+3)*16+(Qscore[i+1]-C+3)*4+(Qscore[i+2]-C+3)+137;
                i+=2;
            }
            else if(i<n-1  && (Qscore[i]>=C-7 && Qscore[i]<=C) && (Qscore[i+1]>=C-7 && Qscore[i+1]<=C))
            {
                outstring[len++]=(Qscore[i]-C+7)*8+(Qscore[i+1]-C+7)+73;
                i+=1;
            }
            else
                outstring[len++]=Qscore[i]-32;
        }
        return len;
}

string lcqs(std::string &Qscore){
    int C,pre;//参考数C和用于找连续串的pre
    int len=0,num=0,begin,n,c,k,j;
    vector<int> ct(128,0);//统计找众数
    string outstring="";
    pre=0;begin=0;n=Qscore.size();num=0;C=0;
    //统计众数C并且统计RLE
        for(int i=0;i<n;i++)
        {
            c=Qscore[i];
            ct[c]++;
            if(ct[c]>=ct[C])
                C=c;
        }
        outstring+=C;
        for(int i=0;i<n;i++)
        {
            c=Qscore[i];
            if(c==C)
            {
                j=i+1;
                while(j<n && Qscore[j]==C)
                {
                    ++j;
                }
                while(j-i>54)
                {   outstring+=255;i+=54;}
                if(j-i>0)
                    outstring+=201+(j-i);
                i=j-1;continue;
            }

            if(i<n-2 && (Qscore[i]>=C-3 && Qscore[i]<=C) && (Qscore[i+1]>=C-3 && Qscore[i+1]<=C) && (Qscore[i+2]>=C-3 && Qscore[i+2]<=C))
            {
                outstring+=(Qscore[i]-C+3)*16+(Qscore[i+1]-C+3)*4+(Qscore[i+2]-C+3)+137;
                i+=2;
            }
            else if(i<n-1  && (Qscore[i]>=C-7 && Qscore[i]<=C) && (Qscore[i+1]>=C-7 && Qscore[i+1]<=C))
            {
                outstring+=(Qscore[i]-C+7)*8+(Qscore[i+1]-C+7)+73;
                i+=1;
            }
            else
                outstring+=Qscore[i]-32;
        }
        return outstring;
}

int rle_lcqs1(std::string &Qscore,vector<int> &outstring){
    int C,pre;//参考数C和用于找连续串的pre
    int len=0,num=0,begin,n,c,k;
    //string outstring="";
    queue<int> rbegin,rnum;
    vector<int> ct(128,0);//统计找众数
    //std::fill(ct.begin(),ct.end(),0);
    pre=0;begin=0;n=Qscore.size();num=0;C=0;
    for(int i=0;i<n;i++)
        {
            c=Qscore[i];
            ct[c]++;
            if(ct[c]>ct[C])
                C=c;
            else if(ct[c]==ct[C] && c>C)
                C=c;
            if(Qscore[begin]==c)
                ++num;
            else
            {
                if(num>3)
                {
                    rbegin.push(begin);
                    rnum.push(num);
                    //cout<< pre<<" "<<begin<<" "<<num<<" ";
                }
                begin=i;
                num=1;

            }
            //pre=c;
        }
    if(num>3)
            {
                rbegin.push(begin);
                rnum.push(num);
            }
    outstring[len++]=C+128;
    //判别是否有连续子串
    if(!rbegin.empty())
    {
        begin=rbegin.front();rbegin.pop();
        num=rnum.front();rnum.pop();
    }
    else
        begin=256;
    for(int i=0;i<n;i++)
    {
        c=Qscore[i];
        if(i==begin)
        {
        //cout<<num<<" ";
            if(num>25)
                    k=(num-1)/25+1;
            else k=1;
            if(c==C)
            {
                i+=num-1;
                while(k)
                {
                    outstring[len++]=(num/k)+230;
                    num-=num/k;
                    k--;
                }
                outstring[len++]=c;
                if(!rbegin.empty())
                {
                    begin=rbegin.front();rbegin.pop();
                    num=rnum.front();rnum.pop();
                }
                else
                    begin=256;
                continue;
            }
            else if(c>=C-3 && c<C && num>6)
            {
                i+=num-1;
                while(k)
                {
                    outstring[len++]=(num/k)+230;
                    num-=num/k;
                    k--;
                }
                outstring[len++]=c;
                if(!rbegin.empty())
                {
                    begin=rbegin.front();rbegin.pop();
                    num=rnum.front();rnum.pop();
                }
                else
                    begin=256;
                continue;
            }
            else if(c>=C-8 && c<C-3 &&num>9)
            {
                i+=num-1;
                while(k)
                {
                    outstring[len++]=(num/k)+230;
                    num-=num/k;
                    k--;
                }
                outstring[len++]=c;
                if(!rbegin.empty())
                {
                    begin=rbegin.front();rbegin.pop();
                    num=rnum.front();rnum.pop();
                }
                else
                    begin=255;
                continue;
            }
            else if(c<C-8 || c>=C)
            {
                i+=num-1;
                while(k)
                {
                    outstring[len++]=(num/k)+230;
                    num-=num/k;
                    k--;
                }
                outstring[len++]=c;
                if(!rbegin.empty())
                {
                    begin=rbegin.front();rbegin.pop();
                    num=rnum.front();rnum.pop();
                }
                else
                    begin=255;
                continue;
            }
            if(!rbegin.empty())
                {
                    begin=rbegin.front();rbegin.pop();
                    num=rnum.front();rnum.pop();
                }
            else
                begin=255;
        }
        if(i<n-2  && i<begin-2 && (Qscore[i]>=C-3 && Qscore[i]<=C) && (Qscore[i+1]>=C-3 && Qscore[i+1]<=C) && (Qscore[i+2]>=C-3 && Qscore[i+2]<=C))
        {
            outstring[len++]=(Qscore[i]-C+3)*16+(Qscore[i+1]-C+3)*4+(Qscore[i+2]-C+3)+169;
            i+=2;
        }
        else if(i<n-1  && i<begin-1 && (Qscore[i]>=C-7 && Qscore[i]<=C) && (Qscore[i+1]>=C-7 && Qscore[i+1]<=C))
        {
            outstring[len++]=(Qscore[i]-C+7)*8+(Qscore[i+1]-C+7)+105;
            i+=1;
        }
        else
            outstring[len++]=Qscore[i];
    }
    return len;
}
string rle_lcqs(std::string &Qscore){
    int C,pre;//参考数C和用于找连续串的pre
    int len=0,num=0,begin,n,c,k;
    string outstring;
    queue<int> rbegin,rnum;
    vector<int> ct(128,0);//统计找众数
    //std::fill(ct.begin(),ct.end(),0);
    outstring="";
    pre=0;begin=0;n=Qscore.size();num=0;C=0;
    for(int i=0;i<n;i++)
        {
            c=Qscore[i];
            ct[c]++;
            if(ct[c]>ct[C])
                C=c;
            else if(ct[c]==ct[C] && c>C)
                C=c;
            if(Qscore[begin]==c)
                ++num;
            else
            {
                if(num>3)
                {
                    rbegin.push(begin);
                    rnum.push(num);
                    //cout<< pre<<" "<<begin<<" "<<num<<" ";
                }
                begin=i;
                num=1;

            }
            //pre=c;
        }
    if(num>3)
            {
                rbegin.push(begin);
                rnum.push(num);
            }
    outstring+=C+128;
    //判别是否有连续子串
    if(!rbegin.empty())
    {
        begin=rbegin.front();rbegin.pop();
        num=rnum.front();rnum.pop();
    }
    else
        begin=256;
    for(int i=0;i<n;i++)
    {
        c=Qscore[i];
        if(i==begin)
        {
        //cout<<num<<" ";
            if(num>25)
                    k=(num-1)/25+1;
            else k=1;
            if(c==C)
            {
                i+=num-1;
                while(k)
                {
                    outstring+=(num/k)+230;
                    num-=num/k;
                    k--;
                }
                outstring+=c;
                if(!rbegin.empty())
                {
                    begin=rbegin.front();rbegin.pop();
                    num=rnum.front();rnum.pop();
                }
                else
                    begin=256;
                continue;
            }
            else if(c>=C-3 && c<C && num>6)
            {
                i+=num-1;
                while(k)
                {
                    outstring+=(num/k)+230;
                    num-=num/k;
                    k--;
                }
                outstring+=c;
                if(!rbegin.empty())
                {
                    begin=rbegin.front();rbegin.pop();
                    num=rnum.front();rnum.pop();
                }
                else
                    begin=256;
                continue;
            }
            else if(c>=C-8 && c<C-3 &&num>9)
            {
                i+=num-1;
                while(k)
                {
                    outstring+=(num/k)+230;
                    num-=num/k;
                    k--;
                }
                outstring+=c;
                if(!rbegin.empty())
                {
                    begin=rbegin.front();rbegin.pop();
                    num=rnum.front();rnum.pop();
                }
                else
                    begin=255;
                continue;
            }
            else if(c<C-8 || c>=C)
            {
                i+=num-1;
                while(k)
                {
                    outstring+=(num/k)+230;
                    num-=num/k;
                    k--;
                }
                outstring+=c;
                if(!rbegin.empty())
                {
                    begin=rbegin.front();rbegin.pop();
                    num=rnum.front();rnum.pop();
                }
                else
                    begin=255;
                continue;
            }
            if(!rbegin.empty())
                {
                    begin=rbegin.front();rbegin.pop();
                    num=rnum.front();rnum.pop();
                }
            else
                begin=255;
        }
        if(i<n-2  && i<begin-2 && (Qscore[i]>=C-3 && Qscore[i]<=C) && (Qscore[i+1]>=C-3 && Qscore[i+1]<=C) && (Qscore[i+2]>=C-3 && Qscore[i+2]<=C))
        {
            outstring+=(Qscore[i]-C+3)*16+(Qscore[i+1]-C+3)*4+(Qscore[i+2]-C+3)+169;
            i+=2;
        }
        else if(i<n-1  && i<begin-1 && (Qscore[i]>=C-7 && Qscore[i]<=C) && (Qscore[i+1]>=C-7 && Qscore[i+1]<=C))
        {
            outstring+=(Qscore[i]-C+7)*8+(Qscore[i+1]-C+7)+105;
            i+=1;
        }
        else
            outstring+=Qscore[i];
    }
    return outstring;
}

string de_lcqs(std::string &instring,int lens)
{
    string Qscore="";
    int len = instring.size(),C,c,num,k=0;
    C=instring[0];
    //cout<<C<<" ";
    for(int i=1;i<len;i++)
    {
        c=instring[i];
        if(c<0)
            c+=256;
        if(c>200)
        {
            num=c-201;
            while(num--){
                Qscore+=C;++k;
                if(k==lens) {Qscore+='\n';k=0;}
            }

        }
        else if(c>136)
        {
            Qscore+=(c-137)/16+C-3;++k;if(k==lens) {Qscore+='\n';k=0;}
            Qscore+=((c-137)%16)/4+C-3;++k;if(k==lens) {Qscore+='\n';k=0;}
            Qscore+=(c-137)%4+(C-3);++k;if(k==lens) {Qscore+='\n';k=0;}

        }
        else if(c>72)
        {
            Qscore+=(c-73)/8+C-7;++k;if(k==lens) {Qscore+='\n';k=0;}
            Qscore+=(c-73)%8+C-7;++k;if(k==lens) {Qscore+='\n';k=0;}

        }
        else
        {
            Qscore+=c+32;++k;if(k==lens) {Qscore+='\n';k=0;}
        }
    }
    return Qscore;
}

string de_rle_lcqs(std::string &instring,int lens){
    string Qscore="";
    int len = instring.size(),C,c,num,k=0;
    Qscore="";
    C=instring[0]+128;
    for(int i=1;i<len;i++)
    {
        c=instring[i];
        if(c<0)
            c+=256;
        if(c>232)
        {
            num=0;
            while(c>232)
            {
                num+=c-230;
                c=instring[++i];
                if(c<0)
                    c+=256;
            }
            //if(c<0) c+=256;
            while(num--)
            {
                Qscore+=c;
                ++k;if(k==lens) {Qscore+='\n';k=0;}
            }

        }
        else
        {
            if(c>168)
            {
                Qscore+=(c-169)/16+C-3;++k;if(k==lens){Qscore+='\n';k=0;}
                Qscore+=((c-169)%16)/4+C-3;++k;if(k==lens) {Qscore+='\n';k=0;}
                Qscore+=(c-169)%4+(C-3);++k;if(k==lens) {Qscore+='\n';k=0;}
            }
            else if(c>104)
            {
                Qscore+=(c-105)/8+C-7;++k;if(k==lens) {Qscore+='\n';k=0;}
                Qscore+=(c-105)%8+C-7;++k;if(k==lens) {Qscore+='\n';k=0;}
            }
            else
            {
                Qscore+=c;++k;if(k==lens) {Qscore+='\n';k=0;}
            }
        }
    }
    return Qscore;
}
