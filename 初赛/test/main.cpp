// random_shuffle example
#include <iostream>     // std::cout
#include <algorithm>    // std::random_shuffle
#include <vector>       // std::vector
#include <ctime>        // std::time
#include <cstdlib>      // std::rand, std::srand
#include <string>
#include <map>
#include <set>
#include <cmath>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

using namespace std;



#define MAX_LINE_LEN 55000
#define MAX_INFO_NUM    50
#define MAX_DATA_NUM    50000


struct TrainDf
{
    string vm_name;     
    string r_time;   
    int i_time;       
};


struct PredictVM
{
    PredictVM(){
        vm_name = "";
        vm_num = 0;
    }
    string vm_name;   
    int vm_num;       
};

struct PhysicalServer
{  
    int core_num;      
    int mem_size;      
    int hd_size;       
}; 

struct VirtualMachine
{
    string vm_name;    
    int core_num;       
    int mem_size;      
};


int read_file(char ** const buff, const unsigned int spec, const char * const filename);
vector<TrainDf> ReadTrain(char *data[],int data_num);
time_t StringToDatetime(string str);
void write_file(const bool cover, const char * const buff, const char * const filename);
string OutPut(map<string,int> vm, int sum);
void ReadInput(char * info[]);
map<string,int> ReadOutput(char * info[],int &all_num);



PhysicalServer m_py_server;
vector<VirtualMachine> m_vm_kind;



//计算一下最后7天各虚拟机总的用量

int main () {
	char *data[MAX_DATA_NUM];
    char *info[MAX_INFO_NUM];
    char *out[MAX_INFO_NUM];
	int data_line_num;
    int info_line_num;


    // char *data_file = argv[1];
    char *data_file = (char *)"result.txt";
    data_line_num = read_file(data, MAX_DATA_NUM, data_file);
    printf("data file line num is :%d \n", data_line_num);
    if (data_line_num == 0)
    {
        printf("Please input valid data file.\n");
        return -1;
    }
    vector<TrainDf> m_train_df = ReadTrain(data, data_line_num);
    // 开始统计各个总量
    map<string,int> vm;
    int sum = 0;
    for(vector<TrainDf>::iterator it = m_train_df.begin(); it != m_train_df.end(); it++){
        sum++;
        vm[it->vm_name]++;
    }

    //输出文件
    string result = OutPut(vm, sum);
    char * result_file = (char *)result.c_str();
    write_file(1, result_file, (char*)"truevalue.txt");

    //读入input文件
    // char *input_file = argv[2];
    char *input_file = (char *)"input.txt";

    info_line_num = read_file(info, MAX_INFO_NUM, input_file);
    printf("input file line num is :%d \n", info_line_num);
    if (info_line_num == 0)
    {
        printf("Please input valid info file.\n");
        return -1;
    }
    ReadInput(info);
    //将此转为map形式
    map<string,VirtualMachine> inputvm;
    // set<string> inputvm;
    for(unsigned int i = 0; i < m_vm_kind.size(); i++){
        inputvm[m_vm_kind[i].vm_name] = m_vm_kind[i];
    }

    //读入output文件
    char *output_file = (char *)"output.txt";
    info_line_num = read_file(out, MAX_INFO_NUM, output_file);
    printf("input file line num is :%d \n", info_line_num);
    if (info_line_num == 0)
    {
        printf("Please input valid info file.\n");
        return -1;
    }
    int all_num = 0;
    map<string,int> pre_vm = ReadOutput(out,all_num);

    //开始计算得分
    double itc = 0.0;

    int fenzi = 0;
    int fenmu_one = 0;
    int fenmu_two = 0;
    int N = 0;
    for(map<string,VirtualMachine>::iterator it = inputvm.begin(); it != inputvm.end(); it++){
        int one = 0;
        int two = 0;
        if(vm.count(it->first)){
            one = vm[it->first];
        }
        if(pre_vm.count(it->first)){
            two = pre_vm[it->first];
        }
        fenmu_one += one*one;
        fenmu_two += two*two;
        fenzi += (one - two)*(one - two);
        N++;
    }
    itc = 1 - sqrt(fenzi * 1.0 / N)/(sqrt(fenmu_one * 1.0 / N) + sqrt(fenmu_two * 1.0 / N));

    //计算空间利用率得分
    int H_core = all_num * m_py_server.core_num;
    int H_mem = all_num * m_py_server.mem_size;

    int V_core = 0;
    int V_mem = 0;

    //用懒一下的算法得了
    
    for(map<string,int>::iterator it = pre_vm.begin(); it != pre_vm.end(); it++){
        V_core += inputvm[it->first].core_num * it->second;
        V_mem += inputvm[it->first].mem_size * it->second;
    }

    // itc = 0.646998;

    double score_core = itc * V_core / H_core * 100;
    double score_mem = itc * V_mem / H_mem * 100;

    printf("score_core = %f\nscore_mem = %f\n", score_core,score_mem);

    return 0;
}

int read_file(char ** const buff, const unsigned int spec, const char * const filename)
{
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
    	printf("Fail to open file %s, %s.\n", filename, strerror(errno));
        return 0;
    }
    printf("Open file %s OK.\n", filename);

    char line[MAX_LINE_LEN + 2];
    unsigned int cnt = 0;
    while ((cnt < spec) && !feof(fp))
    {
        line[0] = 0;
        if (fgets(line, MAX_LINE_LEN + 2, fp) == NULL)  continue;
        if (line[0] == 0)   continue;
        buff[cnt] = (char *)malloc(MAX_LINE_LEN + 2);
        strncpy(buff[cnt], line, MAX_LINE_LEN + 2 - 1);
        buff[cnt][MAX_LINE_LEN + 1] = 0;
        cnt++;
    }
    fclose(fp);
    printf("There are %d lines in file %s.\n", cnt, filename);

    return cnt;
}

vector<TrainDf> ReadTrain(char *data[],int data_num){
    vector<TrainDf> m_train_df;
    TrainDf train;
    for(int i = 0; i < data_num; i++){
        char str_name[12];
        char str_time[24];
        sscanf(data[i],"%*s\t%s\t%s",str_name,str_time);
        train.vm_name = str_name;
        train.r_time = str_time;
        train.i_time = StringToDatetime(train.r_time);
        m_train_df.push_back(train);
    }
    return m_train_df;
}


time_t StringToDatetime(string str)
{
    char *cha = (char*)str.data();             // 将string转换成char*。
    tm tm_;                                    // 定义tm结构体。
    int year, month, day, hour, minute, second;// 定义时间的各个int临时变量。
    sscanf(cha, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);// 将string存储的日期时间，转换为int临时变量。
    hour = 0;                                  //这里将时分秒全部转换为0，因为实际的处理是以天为单位。
    minute = 0;
    second = 0;
    tm_.tm_year = year - 1900;                 // 年，由于tm结构体存储的是从1900年开始的时间，所以tm_year为int临时变量减去1900。
    tm_.tm_mon = month - 1;                    // 月，由于tm结构体的月份存储范围为0-11，所以tm_mon为int临时变量减去1。
    tm_.tm_mday = day;                         // 日。
    tm_.tm_hour = hour;                        // 时。
    tm_.tm_min = minute;                       // 分。
    tm_.tm_sec = second;                       // 秒。
    tm_.tm_isdst = 0;                          // 非夏令时。
    time_t t_ = mktime(&tm_);                  // 将tm结构体转换成time_t格式。
    return t_/3600/24;                         // 返回值。 
}

void write_file(const bool cover, const char * const buff, const char * const filename)
{
    if (buff == NULL)
        return;
    const char *write_type = cover ? "w" : "a";//1:覆盖写文件，0:追加写文件
    FILE *fp = fopen(filename, write_type);
    if (fp == NULL)
    {
        printf("Fail to open file %s, %s.\n", filename, strerror(errno));
        printf("it's fail!\n");
        return;
    }
    printf("Open file %s OK.\n", filename);
    fputs(buff, fp);
    fputs("\n", fp);
    fclose(fp);
}

string OutPut(map<string,int> vm, int sum)
{
    string result = "";
    char cshuzi[100];
    string sshuzi;

    sprintf(cshuzi, "%d", sum);         
    sshuzi = cshuzi;
    result += sshuzi;
    result += "\n";

    for(map<string,int>::iterator it = vm.begin(); it != vm.end(); it++){
        result += it->first;
        result += " ";

        sprintf(cshuzi, "%d", it->second);
        sshuzi = cshuzi;
        result += sshuzi;

        result += "\n";
    }
    return result;
}

void ReadInput(char * info[]){
    //读入物理服务器的属性
    if(info[0] != NULL){
        sscanf(info[0],"%d %d %d",&m_py_server.core_num,&m_py_server.mem_size,&m_py_server.hd_size);
    }
    m_py_server.mem_size *= 1024;

    int m_vm_kind_num;
    //读入虚拟机规格数量
    if(info[2] != NULL){
        sscanf(info[2],"%d",&m_vm_kind_num);
    }
    //将虚拟机种类保存
    VirtualMachine vm;
    for(int i = 3; i < 3 + m_vm_kind_num; i++){
        char str[12];
        sscanf(info[i],"%s %d %d",str,&vm.core_num,&vm.mem_size);
        vm.vm_name = str;
        m_vm_kind.push_back(vm);
    }
    //读入需要优化的资源
    string m_optimize_resource;
    if(info[m_vm_kind_num+4] != NULL){
        char str[12];
        sscanf(info[m_vm_kind_num+4],"%s",str);
        m_optimize_resource = str;
    }
    // //读入需要预测的时间段
    // if(info[m_vm_kind_num+6] != NULL){
    //     char str[24];
    //     sscanf(info[m_vm_kind_num+6],"%s",str);
    //     m_predict_begin_time = str;

    // }
    // if(info[m_vm_kind_num+7] != NULL){
    //     char str[24];
    //     sscanf(info[m_vm_kind_num+7],"%s",str);
    //     m_predict_end_time = str;
    // }
    // //计算需要预测的天数
    // m_predict_day = (StringToDatetime(m_predict_end_time) - StringToDatetime(m_predict_begin_time));
    // if(m_predict_day <= 0){
    //     m_predict_day++;
    // }
}

map<string,int> ReadOutput(char * info[],int &all_num){
    map<string,int> vm;
    int all = 0;
    if(info[0] != NULL){
        sscanf(info[0],"%d",&all);
    }
    int i = 1;
    while(strcmp(info[i],(char*)"\n")){
        char str[12];
        int num = 0;
        sscanf(info[i],"%s %d",str,&num);
        vm[str] = num;
        i++;
    }
    i++;
    if(info[i] != NULL){
        sscanf(info[i],"%d",&all_num);
    }
    return vm;
}


// // random_shuffle example
// #include <iostream>     // std::cout
// #include <algorithm>    // std::random_shuffle
// #include <vector>       // std::vector
// #include <ctime>        // std::time
// #include <cstdlib>      // std::rand, std::srand

// // random generator function:
// int myrandom (int i) { return std::rand()%i;}

// int main () {
//   std::srand ( unsigned ( std::time(0) ) );
//   std::vector<int> myvector;

//   // set some values:
//   for (int i=1; i<10; ++i) myvector.push_back(i); // 1 2 3 4 5 6 7 8 9

//   // using built-in random generator:
  


//   // using myrandom:
// //   std::random_shuffle ( myvector.begin(), myvector.end(), myrandom);

//   // print out content:

//   for(int i = 0; i < 20; i++){
//     std::random_shuffle ( myvector.begin(), myvector.end() );
//     std::cout << "myvector contains:";
//     for (std::vector<int>::iterator it=myvector.begin(); it!=myvector.end(); ++it)
//         std::cout << ' ' << *it;

//     std::cout << '\n';
//     std::cout << rand() / (RAND_MAX * 1.0) << std::endl;
//   }
//   return 0;
// }
