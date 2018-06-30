#include "devcloud.h"
#include<sstream>
#include "Matrix.h"
#include "OnlineSVR.h"
#include <cmath>
using namespace onlinesvr;


//此函数返回的值为距离1900年有多少天
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

bool SortVMByCPU( const RemainderVM &v1, const RemainderVM &v2){
    return v1.core_num > v2.core_num;          //降序排列
}

bool SortVMByMEM( const RemainderVM &v1, const RemainderVM &v2){
    return v1.mem_size > v2.mem_size;          //降序排列   
}

void DevCloud::ReadInput(char * info[]){
    //读入物理服务器的属性
    if(info[0] != NULL){
        sscanf(info[0],"%d %d %d",&m_py_server.core_num,&m_py_server.mem_size,&m_py_server.hd_size);
    }
    m_py_server.mem_size *= 1024;
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
    if(info[m_vm_kind_num+4] != NULL){
        char str[12];
        sscanf(info[m_vm_kind_num+4],"%s",str);
        m_optimize_resource = str;
    }
    //读入需要预测的时间段
    if(info[m_vm_kind_num+6] != NULL){
        char str[24];
        sscanf(info[m_vm_kind_num+6],"%s",str);
        m_predict_begin_time = str;

    }
    if(info[m_vm_kind_num+7] != NULL){
        char str[24];
        sscanf(info[m_vm_kind_num+7],"%s",str);
        m_predict_end_time = str;
    }
    //计算需要预测的天数
    m_predict_day = (StringToDatetime(m_predict_end_time) - StringToDatetime(m_predict_begin_time));
    if(m_predict_day <= 0){
        m_predict_day++;
    }
}

void DevCloud::ReadTrain(char *data[],int data_num){
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
}

void DevCloud::Transform(){
    //检查输入info的虚拟机的个数
    m_predict_vm_num = m_vm_kind_num * 3;
    for(int i = 0; i < m_vm_kind_num; i++){
        PredictVM vm;
        vm.vm_name = m_vm_kind[i].vm_name;
        vm.vm_num = 3;
        m_predict_vm.push_back(vm); 
    }

    //一个虚拟机分配一台主机
    m_predict_py_server_num = m_predict_vm_num;
    for(int i = 0; i < m_predict_py_server_num; i++){
        vector<PredictVM> vec_vm;
        vec_vm.push_back(m_predict_vm[i]);
        m_predict_py_server.push_back(vec_vm);
    }
}

string DevCloud::OutPut(){
    string result = "";
    char cshuzi[100];
    string sshuzi;

    //预测虚拟机总数
    sprintf(cshuzi, "%d", m_predict_vm_num);         
    sshuzi = cshuzi;
    result += sshuzi;
    result += "\n";

    //预测虚拟机的情况
    for(unsigned int i = 0; i < m_predict_vm.size(); i++){
        result += m_predict_vm[i].vm_name;
        result += " ";
        sprintf(cshuzi, "%d", m_predict_vm[i].vm_num); 
        sshuzi = cshuzi;
        result += sshuzi;
        result += "\n";
    }

    //添加空行
    result += "\n";

    //预测需要的物理服务器总数
    sprintf(cshuzi, "%d", m_predict_py_server_num);         
    sshuzi = cshuzi;
    result += sshuzi;
    result += "\n";

    //预测需要的物理服务器的分配情况
    for(unsigned int i = 0; i < m_predict_py_server.size(); i++){
        
        sprintf(cshuzi, "%d", i+1);         
        sshuzi = cshuzi;
        result += sshuzi;
        result += " ";

        for(unsigned int j = 0; j < m_predict_py_server[i].size(); j++){
            result += m_predict_py_server[i][j].vm_name;
            result += " ";
            sprintf(cshuzi, "%d", m_predict_py_server[i][j].vm_num); 
            sshuzi = cshuzi;
            result += sshuzi;
            //如果不是最后一个，则需要添加空格
            if(j != m_predict_py_server[i].size()-1){
                result += " ";
            }
        }
        result += "\n";
    }
    
    // string str_temp = "2016-04-10";
    // int time_temp = StringToDatetime(str_temp);
    // if(m_train_df[m_train_df.size()-1].i_time >= time_temp){
    //     return "";
    // }

    // if(m_predict_day == 8){
    //     return "";
    // }

    // if(m_py_server.core_num == 56){
    //     return "";
    // }
    // if(m_py_server.mem_size == 128*1024){
    //     return "";
    // }


    return result;
}

map<int,int> DevCloud::StatisticsOneKindVM(string flavor){
    //需要将对应的flavor从m_train_df中提取出来，暂时想到只能遍历一遍
    map<int,int> vm;
    for(unsigned int i = 0; i < m_train_df.size(); i++){
        if(m_train_df[i].vm_name == flavor){
            vm[m_train_df[i].i_time]++;
        }
    }
    return vm;
}

double DevCloud::Mean(map<int,int> vm){
    int sum = 0;
    double mean = 0.0;
    int min_day = 0, max_day = 0, total_day = 0;

    if(!vm.empty()){
        min_day = vm.begin()->first;
        max_day = vm.begin()->first;

        for(map<int,int>::iterator it = vm.begin(); it != vm.end(); it++){
            min_day = it->first < min_day ? it->first : min_day;
            max_day = it->first > max_day ? it->first : max_day;
            sum += it->second;
        }

        total_day = max_day - min_day + 1;

        if(total_day > 0){
            mean = sum / (total_day * 1.0);
        }
    }
    return mean;
}

PredictVM DevCloud::PredictOneKindVM(string flavor){
    //将此种类型的虚拟机统计出来
    map<int,int> vm = StatisticsOneKindVM(flavor);
    //求出均值
    double mean = Mean(vm);
    //将预测的结果返回
    PredictVM p_vm;
    p_vm.vm_name = flavor;
    if(mean < 0.001){
        p_vm.vm_num = 0;
    }
    else{
        p_vm.vm_num = (int)(mean * m_predict_day + 0.5);
    }
    return p_vm;
}

void DevCloud::PredictAllVM(){
    //对输入的每一种类型的虚拟机进行预测
    PredictVM p_vm;
    for(unsigned int i = 0; i < m_vm_kind.size(); i++){
        p_vm = PredictOneKindVM(m_vm_kind[i].vm_name);
        if(p_vm.vm_num > 0){
            //对预测结果放大1.2倍
            p_vm.vm_num = (int)(p_vm.vm_num * 2 + 0.5);

            m_predict_vm.push_back(p_vm);
            //将虚拟机总数累加
            m_predict_vm_num += p_vm.vm_num;
        }
    }
}

map<string,VirtualMachine> DevCloud::VMToMap(){
    map<string,VirtualMachine> map_vm;
    for(unsigned int i = 0; i < m_vm_kind.size(); i++){
        map_vm[m_vm_kind[i].vm_name] = m_vm_kind[i];
    }
    return map_vm;
}

void DevCloud::MaxResourcePrior(){
    //输入文件的虚拟机规格属性
    map<string,VirtualMachine> map_vm = VMToMap();
    //一台服务器的剩余资源
    // PhysicalServer sur_res = m_py_server;                 
    //按顺序进行摆放
    for(unsigned int i = 0; i < m_predict_vm.size(); i++){
        //对此种规格的虚拟机全部摆放
        //程序总是异常，怀疑
        int core_max = m_py_server.core_num / map_vm[m_predict_vm[i].vm_name].core_num;
        int mem_max = m_py_server.mem_size / map_vm[m_predict_vm[i].vm_name].mem_size;
        //计算需要几台服务器才能装完
        int core_num = m_predict_vm[i].vm_num / core_max;
        if(m_predict_vm[i].vm_num % core_max != 0){
            core_num++;
        }
        int mem_num = m_predict_vm[i].vm_num / mem_max;
        if(m_predict_vm[i].vm_num % mem_max != 0){
            mem_num++;
        }
        //服务器最多放入此类型的虚拟机个数
        int max_vm_num = core_max < mem_max ? core_max : mem_max;
        //需要的总共服务器数量
        int total_server_num = core_num > mem_num ? core_num : mem_num;
        //最后一个服务器可能装不满
        for(int j = 0; j < total_server_num-1; j++){
            vector<PredictVM> vp_vm;
            PredictVM p_vm;
            p_vm.vm_name = m_predict_vm[i].vm_name;
            p_vm.vm_num = max_vm_num;
            vp_vm.push_back(p_vm);
            //需要的服务器数量加1
            m_predict_py_server_num++;
            m_predict_py_server.push_back(vp_vm);
        }
        //剩下的装入最后一个服务器中
        int res_num = m_predict_vm[i].vm_num % max_vm_num;
        //如果res_num为0,则将res_num置到最大
        if(res_num == 0){
            res_num = max_vm_num;
        }
        vector<PredictVM> vp_vm;
        PredictVM p_vm;
        p_vm.vm_name = m_predict_vm[i].vm_name;
        p_vm.vm_num = res_num;
        vp_vm.push_back(p_vm);
        //需要的服务器数量加1
        m_predict_py_server_num++;
        m_predict_py_server.push_back(vp_vm);
    }
}

void DevCloud::OneServerToVM(){
    for(unsigned int i = 0; i < m_predict_vm.size(); i++){
        //这样就出现了问题，在于服务器的内存没有虚拟机的大
        for(int j = 0; j < m_predict_vm[i].vm_num; j++){
            vector<PredictVM> vp_vm;
            PredictVM p_vm;
            p_vm.vm_name = m_predict_vm[i].vm_name;
            p_vm.vm_num = 1;
            vp_vm.push_back(p_vm);
            //需要的服务器数量加1
            m_predict_py_server_num++;
            m_predict_py_server.push_back(vp_vm);
        }
    }
}

vector<RemainderVM> DevCloud::PredictToRemainder(){
    //输入文件的虚拟机规格属性
    map<string,VirtualMachine> map_vm = VMToMap();
    //需要转化的情况
    vector<RemainderVM> v_rvm;
    for(unsigned int i = 0; i < m_predict_vm.size(); i++){
        RemainderVM rvm;
        rvm.p_vm = m_predict_vm[i];
        rvm.core_num = map_vm[m_predict_vm[i].vm_name].core_num;
        rvm.mem_size = map_vm[m_predict_vm[i].vm_name].mem_size;
        v_rvm.push_back(rvm);
    }
    return v_rvm;
}

void DevCloud::FirstFit(){
    //计算还有多少虚拟机没有摆放进去
    vector<RemainderVM> v_rvm = PredictToRemainder();

    //根据需要优化的资源进行排序
    if(m_optimize_resource == string("CPU")){
        sort(v_rvm.begin(), v_rvm.end(), SortVMByCPU);
    }
    else{
        sort(v_rvm.begin(), v_rvm.end(), SortVMByMEM);
    }

    // sort(v_rvm.begin(), v_rvm.end(), SortVMByMEM);

    //提前准备好一些箱子
    vector<PhysicalServer> v_server_res;            //记录每个服务器的剩余资源

    //将此准备的箱子，同时m_predict_py_server中添加一个箱子的信息
    vector<PredictVM> v_pm;

    /*注意需要保证v_server_res 与 m_predict_py_server 同步*/

    bool flag_erase_vm = false;
    //按照上述排序好的虚拟机规格进行填充
    while(!v_rvm.empty()){

        if(!flag_erase_vm){
            //但凡没能装完，就需要补充新的服务器
            v_server_res.push_back(m_py_server);            //此时箱子为空
            v_pm.clear();
            m_predict_py_server.push_back(v_pm);
        }

        //如果v_rvm里的p_vm.vm_num为0,则将此项的v_rvm删除
        for(vector<RemainderVM>::iterator it = v_rvm.begin(); it != v_rvm.end(); it++){
            flag_erase_vm = false;
            //遍历箱子
            for(unsigned int i = 0; i < v_server_res.size(); i++){
                //计算第i个服务器还有的剩余资源，能够最多摆放多少个it的虚拟机
                int max_num_core = v_server_res[i].core_num / it->core_num;
                int max_num_mem = v_server_res[i].mem_size / it->mem_size;
                int max_num = max_num_core < max_num_mem ? max_num_core :max_num_mem;
                /*
                *将it类型的虚拟机放入第i个服务器
                *情况1:max_num >= it->p_vm.vm_num，直接将此类型的所有的虚拟机放入该服务器
                *情况2:该服务器能放入一部分。
                *情况3:一个也放不下
                */
                if(max_num > 0){
                    if(max_num >= it->p_vm.vm_num){
                        PredictVM pm;
                        pm = it->p_vm;
                        //放入对应的服务器中
                        m_predict_py_server[i].push_back(pm);
                        //更新该服务器的剩余资源
                        v_server_res[i].core_num -= (it->p_vm.vm_num * it->core_num);
                        v_server_res[i].mem_size -= (it->p_vm.vm_num * it->mem_size);
                        //将对应数量减掉
                        it->p_vm.vm_num = 0;
                        //同时将该型号虚拟机删除。
                        v_rvm.erase(it);
                        flag_erase_vm = true;
                        break;
                    }
                    else{
                        PredictVM pm;
                        pm = it->p_vm;
                        pm.vm_num = max_num;
                        //放入对应的服务器中
                        m_predict_py_server[i].push_back(pm);
                        //更新该服务器的剩余资源
                        v_server_res[i].core_num -= (max_num * it->core_num);
                        v_server_res[i].mem_size -= (max_num * it->mem_size);
                        //将对应数量减掉
                        it->p_vm.vm_num -= max_num;
                    }
                }
            }
            if(flag_erase_vm){
                break;
            }
        }
    }
    //最后，返回需要的服务器的个数
    m_predict_py_server_num = m_predict_py_server.size();
}



//测试集取最后7天
// vector<int> DevCloud::FindTest(map<int,int> vm, int num){
//     //定义一周的vector，0,1,2,3,4,5,6分别对应周日，一，二，。。。六
//     vector<int> week(7,0);                      //初始值都设置为0
//     //计算最后7天的时间点
//     int begin_day = StringToDatetime(m_predict_begin_time) - 7;
//     //将此种类型的虚拟机统计出来
//     if(!vm.empty()){
//         for(map<int,int>::iterator it = vm.begin(); it != vm.end(); it++){
//             if(it->first >= begin_day){
//                 //计算出当天属于周几
//                 int day_of_week = (it->first - StringToDatetime("2015-01-01") + 4)%7;
//                 week[day_of_week] = it->second;
//             }   
//         }
//     }
//     return week;
// }


//将最后的3周最为训练集进行处理,从测试集的前一天begin_day倒数到end_day
vector<int> DevCloud::FindTrain(map<int,int> vm){
    int begin_num = 0, end_num = 21;
    vector<int> train(end_num - begin_num, 0);
    //计算所需的时间点，服从前闭后开原则
    int end_day = StringToDatetime(m_predict_begin_time) - begin_num;
    int begin_day = StringToDatetime(m_predict_begin_time) - end_num;
    //如果时间点在其中，则保存
    if(!vm.empty()){
        for(map<int,int>::iterator it = vm.begin(); it != vm.end(); it++){
            if(it->first >= begin_day && it->first < end_day){
                //则讲其保存其中
                train[it->first - begin_day] = it->second;
            }   
        }
    }
    return train;
}


PredictVM DevCloud::PredictOneKindVM_GuiZe(string flavor){
    //将此种类型的虚拟机统计出来
    map<int,int> vm = StatisticsOneKindVM(flavor);
    //划分出来测试集
    // vector<int> test = FindTest(vm, 7);
    //划分出来训练集
    vector<int> train = FindTrain(vm);
    vector<float> week_mean;
    //计算三周的周均值
    for(int i = 0; i < 3; i++){
        int sum = 0;
        for(int j = 0; j < 7; j++){
            sum += train[i*7 + j];
        }
        float mean = sum / 7.0;
        week_mean.push_back(mean);
    }
    //对此train进行规则预测
    //定义3个vector<int>，长度各为7
    vector< vector<int> > week_original_value(3);
    vector< vector<float> > week_factor(3);
    //根据train对week_Original_value赋值
    for(int i = 0; i < 3; i++){
        vector<int> original_value(7,0);
        vector<float> factor(7,0.0);
        for(int j = 0; j < 7; j++){
             original_value[j] = train[i*7 + j];
             if(week_mean[i] < 0.0001){
                factor[j] = 0;
             }
             else{
                factor[j] = train[i*7 + j] / week_mean[i];
             }
        }
        week_original_value[i] = original_value;
        week_factor[i] = factor;
    }
    //求三周因子的中位数,以及均值
    vector<float> factor_median(7,0.0);
    vector<float> factor_mean(7,0.0);
    for(int i = 0; i < 7; i++){
        vector<float> factor(3,0.0);
        for(int j = 0; j < 3; j++){
            factor[j] = week_factor[j][i];
        }
        //对factor排序
        sort(factor.begin(),factor.end());
        factor_median[i] = factor[1];
        factor_mean[i] = (factor[0] + factor[1] + factor[2]) / 3;
    }
    //求base，利用最后一周的均值
    //或者最后3,5天的均值
    int last_week_total = 0;
    for(int i = 0; i < 7; i++){
        last_week_total += week_original_value[2][i];
    }
    float base = last_week_total / 7.0;
    
    //用base乘以factor的中位数，即得相应预测的天数
    //或者用factor的均值，或者两者的组合
    
    //定义需要预测的天数
    vector<int> predict_day(m_predict_day,0);
    PredictVM p_vm;
    p_vm.vm_name = flavor;
    p_vm.vm_num = 0;

    for(unsigned int i = 0; i < predict_day.size(); i++){
        predict_day[i] = (int)(base * factor_median[i % 7] + 0.5);
        p_vm.vm_num += predict_day[i];
    }

    return p_vm;
}



void DevCloud::GuiZe(){
    //对输入的每一种类型的虚拟机进行预测
    PredictVM p_vm;
    for(unsigned int i = 0; i < m_vm_kind.size(); i++){
        p_vm = PredictOneKindVM_GuiZe(m_vm_kind[i].vm_name);
        if(p_vm.vm_num > 0){
            //对预测结果放大1.2倍
            // p_vm.vm_num = (int)(p_vm.vm_num * 1.2 + 0.5);
            m_predict_vm.push_back(p_vm);
            //将虚拟机总数累加
            m_predict_vm_num += p_vm.vm_num;
        }
    }
}


PredictVM DevCloud::PredictOneKindVM_LastPredictDay(string flavor){
    //将此种类型的虚拟机统计出来
    map<int,int> vm = StatisticsOneKindVM(flavor);
    //求出最后几天的总和
    int sum = 0;
    //计算需要预测的开始的天
    int begin_day = StringToDatetime(m_predict_begin_time) - m_predict_day + 1;
    if(!vm.empty()){
        for(map<int,int>::iterator it = vm.begin(); it != vm.end(); it++){
            if(it->first >= begin_day){
                sum += it->second;
            }   
        }
    }
    //将预测的结果返回
    PredictVM p_vm;
    p_vm.vm_name = flavor;
    p_vm.vm_num = sum;
    return p_vm;
}

void DevCloud::LastPredictDay(){
    //对输入的每一种类型的虚拟机进行预测
    PredictVM p_vm;
    for(unsigned int i = 0; i < m_vm_kind.size(); i++){
        p_vm = PredictOneKindVM_LastPredictDay(m_vm_kind[i].vm_name);
        if(p_vm.vm_num > 0){
            //对预测结果放大1.2倍
            // p_vm.vm_num = (int)(p_vm.vm_num * 1.2 + 0.5);
            m_predict_vm.push_back(p_vm);
            //将虚拟机总数累加
            m_predict_vm_num += p_vm.vm_num;
        }
    }
}


//对预测的虚拟机进行拆箱
vector<RemainderVM> DevCloud::Unpacking(){
    //输入文件的虚拟机规格属性
    map<string,VirtualMachine> map_vm = VMToMap();
    //需要转化的情况
    vector<RemainderVM> v_rvm;
    for(unsigned int i = 0; i < m_predict_vm.size(); i++){
        for(int j = 0; j < m_predict_vm[i].vm_num; j++){
            RemainderVM rvm;
            rvm.p_vm = m_predict_vm[i];
            //将数量置为1
            rvm.p_vm.vm_num = 1;
            rvm.core_num = map_vm[m_predict_vm[i].vm_name].core_num;
            rvm.mem_size = map_vm[m_predict_vm[i].vm_name].mem_size;
            v_rvm.push_back(rvm);
        }
    }
    return v_rvm;

}


void DevCloud::TransferServer(vector<vector<PredictVM> > py_server){
    for(unsigned int i = 0; i < py_server.size(); i++){
        if(py_server[i].empty()){
            break;
        }
        vector<PredictVM> vp_vm;
        for(unsigned int j = 0; j < py_server[i].size(); j++){
            //如果有相同的，则数量相加，否则，压入
            unsigned int k = 0;
            for(k = 0; k < vp_vm.size(); k++){
                if(py_server[i][j].vm_name == vp_vm[k].vm_name){
                    break;
                }
            }
            if(k == vp_vm.size()){
                vp_vm.push_back(py_server[i][j]);
            }
            else{
                vp_vm[k].vm_num += 1;
            }
        }
        //将所有压入总服务器
        m_predict_py_server.push_back(vp_vm);
    }
    //计算总数
    m_predict_py_server_num = m_predict_py_server.size();
}

void DevCloud::BestFit(){
    //第一步：拆箱：将预测的虚拟机全部拆开为单独的
    vector<RemainderVM> v_rvm = Unpacking();
    //第二步：根据需要优化的资源进行排序
    if(m_optimize_resource == string("CPU")){
        sort(v_rvm.begin(), v_rvm.end(), SortVMByCPU);
    }
    else{
        sort(v_rvm.begin(), v_rvm.end(), SortVMByMEM);
    }

    // sort(v_rvm.begin(), v_rvm.end(), SortVMByCPU);

    //第三步：用bestFit装箱
    //首先摆放最多的服务器 vector<PhysicalServer> v_server_res;            //记录每个服务器的剩余资源
    int max_server_num = v_rvm.size();
    int res = 0;
    int bin_rem_core[max_server_num];
    int bin_rem_mem[max_server_num];
    //首先分配最大的服务器
    vector<vector<PredictVM> > py_server(max_server_num);

    for(int i = 0; i < max_server_num; i++){
        int min_core = m_py_server.core_num + 1;
        int min_mem = m_py_server.mem_size + 1; 
        int bi = 0;
        for(int j = 0; j< res; j++){
            if(m_optimize_resource == string("CPU")){
                if(bin_rem_core[j] >= v_rvm[i].core_num && bin_rem_mem[j] >= v_rvm[i].mem_size \
                    && bin_rem_core[j] - v_rvm[i].core_num < min_core){
                    bi = j;
                    min_core = bin_rem_core[j] - v_rvm[i].core_num;
                    min_mem = bin_rem_mem[j] - v_rvm[i].mem_size;
                }
            }
            else{
                if(bin_rem_core[j] >= v_rvm[i].core_num && bin_rem_mem[j] >= v_rvm[i].mem_size \
                    && bin_rem_mem[j] - v_rvm[i].mem_size < min_mem){
                    bi = j;
                    min_core = bin_rem_core[j] - v_rvm[i].core_num;
                    min_mem = bin_rem_mem[j] - v_rvm[i].mem_size;
                }
            }
        }
        //如果放不下，则整一个新箱子
        if(min_core == (m_py_server.core_num + 1) && min_mem == (m_py_server.mem_size + 1)){
            bin_rem_core[res] = m_py_server.core_num - v_rvm[i].core_num;
            bin_rem_mem[res] = m_py_server.mem_size - v_rvm[i].mem_size;
            //将此虚拟机放入箱子
            PredictVM p_vm;
            p_vm.vm_name = v_rvm[i].p_vm.vm_name;
            p_vm.vm_num = 1;
            py_server[res].push_back(p_vm);
            //将此箱子加一个
            res++;
        }
        else{
            bin_rem_core[bi] -= v_rvm[i].core_num;
            bin_rem_mem[bi] -= v_rvm[i].mem_size;
            //将此虚拟机放入箱子
            PredictVM p_vm;
            p_vm.vm_name = v_rvm[i].p_vm.vm_name;
            p_vm.vm_num = 1;
            py_server[bi].push_back(p_vm);
        }
    }

    //第四步：重新装箱，将py_server转移到m_predict_py_server中
    TransferServer(py_server);
}

void DevCloud::CheckUtilization(){
    //输入文件的虚拟机规格属性
    map<string,VirtualMachine> map_vm = VMToMap();
    //开始计算，
    int num = 0;
    double utilization_core = 0.0;
    double utilization_mem = 0.0;
    for(unsigned int i = 0; i < m_predict_py_server.size(); i++){
        int core_num = 0;
        int mem_num = 0;
        for(unsigned int j = 0; j < m_predict_py_server[i].size(); j++){
            core_num += m_predict_py_server[i][j].vm_num * map_vm[m_predict_py_server[i][j].vm_name].core_num;
            mem_num += m_predict_py_server[i][j].vm_num * map_vm[m_predict_py_server[i][j].vm_name].mem_size;
        }
        //计算利用率
        utilization_core = core_num / (m_py_server.core_num * 1.0);
        utilization_mem = mem_num / (m_py_server.mem_size * 1.0);
        if(utilization_core < 0.5 && utilization_mem < 0.5){
            num++;
        }
    }
    if(num > 1){
        m_predict_py_server_num = 0;
    }
}

void DevCloud::ImproveUtilization(){
    //检查一下装箱利用率，如果小于50%，则需要处理
    //输入文件的虚拟机规格属性
    map<string,VirtualMachine> map_vm = VMToMap();
    //开始计算，
    // int num = 0;
    double utilization_core = 0.0;
    double utilization_mem = 0.0;
    for(unsigned int i = 0; i < m_predict_py_server.size(); i++){
        int core_num = 0;
        int mem_num = 0;
        for(unsigned int j = 0; j < m_predict_py_server[i].size(); j++){
            core_num += m_predict_py_server[i][j].vm_num * map_vm[m_predict_py_server[i][j].vm_name].core_num;
            mem_num += m_predict_py_server[i][j].vm_num * map_vm[m_predict_py_server[i][j].vm_name].mem_size;
        }
        //计算利用率
        utilization_core = core_num / (m_py_server.core_num * 1.0);
        utilization_mem = mem_num / (m_py_server.mem_size * 1.0);
        if(utilization_core < 0.7 && utilization_mem < 0.7){
            // num++;
            //对此服务器再进行填充处理，确定是第几号的服务器，以及放入的虚拟机
            FillVM(i, m_predict_py_server[i]); 
        }
    }
}

void DevCloud::FillVM(int num, vector<PredictVM>& v_pvm){
    //输入文件的虚拟机规格属性
    map<string,VirtualMachine> map_vm = VMToMap();
    //计算剩余空间
    int rem_core = m_py_server.core_num;
    int rem_mem = m_py_server.mem_size;
    for(unsigned int i = 0; i < v_pvm.size(); i++){
        rem_core -= v_pvm[i].vm_num * map_vm[v_pvm[i].vm_name].core_num;
        rem_mem -= v_pvm[i].vm_num * map_vm[v_pvm[i].vm_name].mem_size;
    }
    //开始填充
    for(unsigned int i = 0; i < v_pvm.size(); i++){
        //计算可以填充此类型的虚拟机几个
        int core_num = rem_core / map_vm[v_pvm[i].vm_name].core_num;
        int mem_num = rem_mem / map_vm[v_pvm[i].vm_name].mem_size;
        int max_num = core_num < mem_num ? core_num : mem_num;

        //将max_num填充进去，此只更新了服务器里面的数量，服务器的总量不会改变
        // v_pvm[i].vm_num += max_num;
        // m_predict_vm_num += max_num;

        //只提升1个虚拟机
        if(max_num > 0){
            v_pvm[i].vm_num += 1;
            m_predict_vm_num += 1;
            max_num = 1;
        }

        //更新此类型的预测虚拟机的数量
        for(unsigned int j = 0; j < m_predict_vm.size(); j++){
            if(v_pvm[i].vm_name == m_predict_vm[j].vm_name){
                m_predict_vm[j].vm_num += max_num;
            }
        }
        //更新剩余空间
        rem_core -= max_num * map_vm[v_pvm[i].vm_name].core_num;
        rem_mem -= max_num * map_vm[v_pvm[i].vm_name].mem_size;
    }

}


int DevCloud::StatisticsOneKindLastThreeWeekVM(string flavor){
    //需要将对应的flavor从m_train_df中提取出来，暂时想到只能遍历一遍
    int sum = 0;
    //最计算最后的三周的内容
    int day_time = StringToDatetime(m_predict_begin_time) - 21;
    for(unsigned int i = 0; i < m_train_df.size(); i++){
        if(m_train_df[i].vm_name == flavor && m_train_df[i].i_time >= day_time){
            //如果在最后三周
            sum += 1;
        }
    }
    return sum;
}


void DevCloud::ClassificationPredict(){
    //对输入的每一种类型的虚拟机进行预测
    PredictVM p_vm;
    for(unsigned int i = 0; i < m_vm_kind.size(); i++){
        //计算该类型的虚拟机在总体训练集的使用量
        int sum = StatisticsOneKindLastThreeWeekVM(m_vm_kind[i].vm_name);
        if(sum >= 21){
            p_vm = PredictOneKindVM_GuiZe(m_vm_kind[i].vm_name);
            //引入报错，一定要注销掉
            // m_predict_vm_num = 0;
        }
        else{
            p_vm = PredictOneKindVM_LastPredictDay(m_vm_kind[i].vm_name);
        }
        if(p_vm.vm_num > 0){
            //对预测结果放大1.2倍
            // p_vm.vm_num = (int)(p_vm.vm_num * 1.2 + 0.5);
            m_predict_vm.push_back(p_vm);
            //将虚拟机总数累加
            m_predict_vm_num += p_vm.vm_num;
        }
    }
}

void DevCloud::Merge(){
    //对输入的每一种类型的虚拟机进行预测
    PredictVM p_vm1, p_vm2;
    for(unsigned int i = 0; i < m_vm_kind.size(); i++){
        p_vm1 = PredictOneKindVM_GuiZe(m_vm_kind[i].vm_name);
        // p_vm2 = PredictOneKindVM_LastPredictDay(m_vm_kind[i].vm_name);
        p_vm2 = PredictOneKindVM_SVR(m_vm_kind[i].vm_name);


        if(p_vm1.vm_num > 0 ){
            //对预测结果放大1.2倍
            p_vm1.vm_num = (int)(p_vm1.vm_num * 1.2 + 0.5);
        }
        if(p_vm2.vm_num > 0 ){
            //对预测结果放大1.2倍
            p_vm2.vm_num = (int)(p_vm2.vm_num * 1.2 + 0.5);
        }


        PredictVM p_vm = p_vm1;
        p_vm.vm_num = (int)(p_vm1.vm_num * 0.7 + p_vm2.vm_num * 0.3);
        
        if(p_vm.vm_num > 0 ){
            //对预测结果放大1.2倍
            // p_vm.vm_num = (int)(p_vm.vm_num * 1.2 + 0.5);

            m_predict_vm.push_back(p_vm);
            //将虚拟机总数累加
            m_predict_vm_num += p_vm.vm_num;
        }
    }
}


PredictVM DevCloud::PredictOneKindVM_SVR(string flavor){
    //将此种类型的虚拟机统计出来
    map<int,int> vm = StatisticsOneKindVM(flavor);
    //根据此vm，构造训练集
    // Build training set
    /*
     *分为3来来进行
     * 第一步，只用一个week信息
     * 第二步，使用week和上周的总量的cnt
     * 第三步，构建更复杂的特征，使用AddColCopy()方法
     */
    //需要特别小心，这里自己定义的Vector大写的，和vector不兼容的
    //构造目标向量
    Vector<double>* TrainingSetY = Vector<double>::ZeroVector(21);
    //计算倒数第21天的时间
    int first_day = StringToDatetime(m_predict_begin_time) - 21;
    for(map<int,int>::iterator it = vm.begin(); it != vm.end(); it++){
        if(it->first - first_day >= 0){
            TrainingSetY->Values[it->first - first_day] = it->second;
        }
    }

    //打印出来
    // TrainingSetY->Print("TrainingSetY = ");

    //构造特征向量week
	Matrix<double>* TrainingSetX = new Matrix<double>();
    //构造week特征，倒数21天的信息(即3周)
    Vector<double>* three_week = Vector<double>::ZeroVector(21);
    //计算倒数21天的信息属性，周日为0，其他以此类推
    for(int i = 0; i < 21; i++){
        three_week->Values[i] = (first_day + i - StringToDatetime("2015-01-01") + 4) % 7;
    }
    //打印出来
    // three_week->Print("three_week = ");


    //将该列特征，加入到矩阵中
    TrainingSetX->AddColCopy(three_week);

    //打印出来
    // TrainingSetX->Print("TrainingSetX = ");

    //构造cnt的特征，以上周的cnt的总和为特征
    //总计需要四周的总量，最后一周拿来做预测用
    int one_week_begin = StringToDatetime(m_predict_begin_time) - 28;
    int two_week_begin = StringToDatetime(m_predict_begin_time) - 21;
    int three_week_begin = StringToDatetime(m_predict_begin_time) - 14;
    int four_week_begin = StringToDatetime(m_predict_begin_time) - 7;
    //采用前闭后开进行计算
    double one_cnt = 0.0;            
    double two_cnt = 0.0;
    double three_cnt = 0.0;
    double four_cnt = 0.0;
    //又重新遍历一遍，很费时间，类似这样的在程序中很多，后续需要优化了
    for(map<int,int>::iterator it = vm.begin(); it != vm.end(); it++){
        if(it->first >= one_week_begin && it->first < two_week_begin){
            one_cnt += it->second;
        }
        else if(it->first >= two_week_begin && it->first < three_week_begin){
            two_cnt += it->second;
        }
        else if(it->first >= three_week_begin && it->first < four_week_begin){
            three_cnt += it->second;
        }
        else if(it->first >= four_week_begin){
            four_cnt += it->second;
        }
        else{
            continue;
        }
    }

    //将四个特征取对数
    // if(one_cnt > 0.5){
    //     one_cnt = log(one_cnt);
    // }
    // if(two_cnt > 0.5){
    //     two_cnt = log(two_cnt);
    // }
    // if(three_cnt > 0.5){
    //     three_cnt = log(three_cnt);
    // }
    // if(four_cnt > 0.5){
    //     four_cnt = log(four_cnt);
    // }

    //将int one_cnt、two_cnt、three_cnt 扩充为21的向量
    Vector<double>* train_cnt = Vector<double>::ZeroVector(21);
    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 7; j++){
            double temp = 0.0;
            if(i == 0){
                temp = one_cnt;
            }
            else if(i == 1){
                temp = two_cnt;
            }
            else{
                temp = three_cnt;
            }

            train_cnt->Values[i*7 + j] = temp;
        }
    }
    //将该列特征添加到矩阵中
    TrainingSetX->AddColCopy(train_cnt);

    // TrainingSetX->Print("train ");

    //构造测试集的特征，只预测7天的，其它以此类推
    Matrix<double>* TestSetX = new Matrix<double>();
    //构造week特征,一周的即可，从预测的当天开始计算
    Vector<double>* one_week = Vector<double>::ZeroVector(7);
    //计算倒数21天的信息属性，周日为0，其他以此类推
    for(int i = 0; i < 7; i++){
        one_week->Values[i] = (StringToDatetime(m_predict_begin_time) + i - StringToDatetime("2015-01-01") + 4) % 7;
    }
    //将该列特征，加入到矩阵中
    TestSetX->AddColCopy(one_week);

    //构造cnt的特征
    Vector<double>* test_cnt = Vector<double>::ZeroVector(7);
    for(int i = 0; i < 7; i++){
        test_cnt->Values[i] = four_cnt;
    }
    //将该列特征添加到矩阵中
    TestSetX->AddColCopy(test_cnt);

    // TestSetX->Print("test ");

    //开始训练
    // Make a new OnlineSVR
    OnlineSVR* SVR = new OnlineSVR();
    SVR->SetC(1.5);
    SVR->SetEpsilon(0.01);
    SVR->SetKernelType(OnlineSVR::KERNEL_RBF_GAUSSIAN);
    SVR->SetKernelParam(26);
    SVR->SetVerbosity(OnlineSVR::VERBOSITY_NORMAL);	
    // Train OnlineSVR
	SVR->Train(TrainingSetX,TrainingSetY);
    // Show OnlineSVR info
	// SVR->ShowInfo();
    // Predict some new values
    Vector<double>* PredictedY = SVR->Predict(TestSetX);
    // PredictedY->Print("predict = ");

    //对需要预测的天数的总量进行汇总
    PredictVM p_vm;
    p_vm.vm_name = flavor;
    p_vm.vm_num = 0;

    double total_num = 0.0;
    for(int i = 0; i < m_predict_day; i++){
        total_num += PredictedY->Values[i % 7];
    }
    p_vm.vm_num = (int)(total_num + 0.5);


    //删除掉所创建的资源
    delete TrainingSetY;
    delete TrainingSetX;
    delete three_week;
    delete train_cnt;
    delete TestSetX;
    delete one_week;
    delete test_cnt;
    delete SVR;
    delete PredictedY;


    //返回最终结果
    return p_vm;
}



void DevCloud::SVRPredict(){
        //对输入的每一种类型的虚拟机进行预测
    PredictVM p_vm;
    for(unsigned int i = 0; i < m_vm_kind.size(); i++){
        p_vm = PredictOneKindVM_SVR(m_vm_kind[i].vm_name);
        if(p_vm.vm_num > 0 ){
            //对预测结果放大1.2倍
            p_vm.vm_num = (int)(p_vm.vm_num * 1.2 + 0.5);
            m_predict_vm.push_back(p_vm);
            //将虚拟机总数累加
            m_predict_vm_num += p_vm.vm_num;
        }
    }
}
