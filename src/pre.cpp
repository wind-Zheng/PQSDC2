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
#include <unordered_map>
using namespace std;
void get_data(std::string &inputfile,std::string &Qscore);
void compress(std::string &inputfile);
string lcqs(std::string &Qscore);
string rle_lcqs(std::string &Qscore);
double calculateEntropy(const std::string& str);

int main(int argc, char** argv){
    string A = std::string(argv[2]);
    //string A = "/public/home/jd_sunhui/genCompressor/Data/110902_I244_FCC02FUACXX_L4_006SCELL03AEAAAPEI-12/110902_I244_FCC02FUACXX_L4_006SCELL03AEAAAPEI-12_2.fq";
    //string A = "/public/home/jd_sunhui/genCompressor/Data/NCBI/PhiX/PhiX_12.fastq";
    string method=argv[1];
    if(method=="-c")
        compress(A);

    //cout<<method<<"  "<<method.size()<<endl;
    //cout<<A<<endl;
    return 0;
}

void compress(std::string &inputfile){
    std::ifstream inFile;
    inFile.open(inputfile.c_str(), std::ios::in);
    if (!inFile) throw("Source_File_Wrong!");

    string Qscore;
    int C,pre;//参考数C和用于找连续串的pre
    int len=0,begin,n,c,k;
    string outstring,s;
    queue<int> rbegin,rnum;
    vector<int> ct(128,0);//统计找众数
    int num1=0,num2=0,num3=0,num=0;

    string out_csv=inputfile.substr(0,inputfile.find('.'));
    out_csv=inputfile+".csv";
    ofstream csv;
    csv.open(out_csv,std::ios_base::trunc);

    //csv<<"C_Proportion"<<','<<"RLE_num"<<','<<"score_num"<<','<<"Difference_ratio"<<','<<"result"<<'\n';
    csv.close();
    //return ;

    while (inFile.peek() != EOF){
        Qscore="";
        //cout<<Qscore.size()<<endl;
        num=0;
        //cout<<"xx"<<endl;

//        cout<<inFile.is_open()<<" ";
//        cout<<inFile.peek()<<" ";
        while(num<100 && getline(inFile,s))
        {
        //cout<<s<<endl;
            //getline(inFile,s);
            Qscore+=s;
            ++num;
            //cout<<num<<endl;
        }


        get_data(out_csv,Qscore);
        //cout<<0;
        csv.open(out_csv,std::ios_base::app);
        //cout<<Qscore<<endl;
        string outs1,outs2,outs;
        outs1=lcqs(Qscore);
        //cout<<1;
        outs2=rle_lcqs(Qscore);
        //cout<<2;
        double entropy1=calculateEntropy(outs1);
        //cout<<3;
        double entropy2=calculateEntropy(outs2);
        //cout<<4<<endl;
        if(entropy1<entropy2)
        {
            ++num1;outs=outs1;
            csv<<'0'<<'\n';
        }
        else if(outs1.size()==outs2.size())
        {
            ++num2;outs=outs1;
            csv<<'0'<<'\n';
        }
        else
        {
            ++num3;outs=outs2;
            csv<<'1'<<'\n';
        }
        //cout<<5<<endl;
        csv.close();
        //cout<<inFile.peek()<<" ";
        //cout<<inFile.peek()<<" ";
    }
    inFile.close();


    return ;
}

void get_data(std::string &out_csv,std::string &Qscore){
    ofstream csv;
    csv.open(out_csv,std::ios_base::app);
    double C_Proportion,Difference_ratio=0,RLE_num_ratio=0,score_num_ratio=0;
    int num=0,begin=0;
    int n=Qscore.size(),C=0,c;
    vector<int> ct(128,0);//统计找众数
    for(int i=0;i<n;i++)
        {
            if(i>0&&abs(Qscore[i]-Qscore[i-1])>7)
                ++Difference_ratio;
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
                if(num>3) RLE_num_ratio+=num;
                begin=i;
                num=1;

            }

            //pre=c;
        }
    C_Proportion=(double)ct[C]/n;
    Difference_ratio=(double)Difference_ratio/(n-1);
    RLE_num_ratio=(double)RLE_num_ratio/(n);
    for(int num : ct)
    {
        if(num)
            ++score_num_ratio;
    }
    score_num_ratio=(double)score_num_ratio/72;
    //cout<<C_Proportion<<','<<RLE_num<<','<<score_num<<','<<Difference_ratio<<endl;
    csv<<C_Proportion<<','<<RLE_num_ratio<<','<<score_num_ratio<<','<<Difference_ratio<<',';
    //csv<<C_Proportion<<','<<RLE_num_ratio<<','<<score_num_ratio<<','<<Difference_ratio<<'\n';
    csv.close();

    return ;
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

double calculateEntropy(const std::string& str) {
    std::unordered_map<char, int> charCount;
    int strLength = str.length();

    // 统计字符串中每个字符的出现次数
    for (char c : str) {
        charCount[c]++;
    }

    double entropy = 0.0;
    for (const auto& pair : charCount) {
        double probability = static_cast<double>(pair.second) / strLength;
        entropy -= probability * log2(probability);
    }

    return entropy;
}