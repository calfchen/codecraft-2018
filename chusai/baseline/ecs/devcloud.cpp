#include "devcloud.h"
#include<sstream>

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

        if(i == 0){
            m_min_day = train.i_time;
            m_max_day = train.i_time;
        }
        else{
            if(train.i_time > m_max_day){
                m_max_day = train.i_time;
            }
            if(train.i_time < m_min_day){
                m_min_day = train.i_time;
            }
        }
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

    // if(!CheckVMMEMSize()){
    //     return "";
    // }

    // if(m_vm_kind.size() <= 4){
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

vector<int> DevCloud::PrecessOutlier(vector<int> train){
    vector<int> temp = train;
    //对temp进行排序，找到四分位数
    sort(temp.begin(), temp.end());
    int q1 = (int)(temp.size() / 4);
    int q3 = temp.size() - q1 - 1;
    double k = 3;
    int max = (int)(temp[q3] + k*(temp[q3] - temp[q1]));

    for(unsigned int i = 1; i < train.size(); i++){
        if(train[i] > max){
            train[i] = max;
            // train[i] = (int)(train[i] * 0.7);
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
    vector<int> train_temp = FindTrain(vm);
    // vector<int> train = FindTrain(vm);
    //对划分出来的训练集进行异常值处理，用四分位数发对高值处理
    vector<int> train = PrecessOutlier(train_temp);

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
    int last_day = 7;                       //最后几天的平均值
    for(int i = (7 - last_day); i < 7; i++){
        // if(week_factor[2][i] < 0.001){
        //     last_week_total += week_original_value[2][i];
        // }
        // else{
        //     last_week_total += week_original_value[2][i] / week_factor[2][i];
        // }
        last_week_total += week_original_value[2][i];
        
    }
    float base = last_week_total / (last_day * 1.0);
    
    //用base乘以factor的中位数，即得相应预测的天数
    //或者用factor的均值，或者两者的组合
    
    //定义需要预测的天数
    vector<int> predict_day(m_predict_day,0);
    PredictVM p_vm;
    p_vm.vm_name = flavor;
    p_vm.vm_num = 0;

    for(unsigned int i = 0; i < predict_day.size(); i++){
        // predict_day[i] = (int)(base * factor_median[i % 7] + 0.5);
        // predict_day[i] = (int)(base * factor_mean[i % 7] + 0.5);
        predict_day[i] = (int)(base * (factor_mean[i % 7] * 0.4 + factor_median[i % 7] * 0.6)+ 0.5);
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
            p_vm.vm_num = (int)(p_vm.vm_num * 1.2 + 0.5);
            // p_vm.vm_num = (int)(p_vm.vm_num / 1.2 + 0.5);
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
    int begin_day = StringToDatetime(m_predict_begin_time) - m_predict_day;
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
        if(utilization_core < 0.7 && utilization_mem < 0.7){
            num++;
        }
    }
    if(num > 0){
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

        //此步为提高利用率
        // if(utilization_core < 0.7 && utilization_mem < 0.7){
        //     // num++;
        //     //对此服务器再进行填充处理，确定是第几号的服务器，以及放入的虚拟机
        //     FillVM(i, m_predict_py_server[i]); 
        // }
        // FillVM(i, m_predict_py_server[i]); 


        //如果提高利用率不能work，则可以尝试deletevm
        if(utilization_core < 0.7 && utilization_mem < 0.7){
            // num++;
            //对此服务器再进行填充处理，确定是第几号的服务器，以及放入的虚拟机
            DeleteVM(i, m_predict_py_server[i]); 
            i--;
        }
        else{
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

void DevCloud::DeleteVM(int num, vector<PredictVM>& v_pvm){
    //输入文件的虚拟机规格属性
    map<string,VirtualMachine> map_vm = VMToMap();
    //开始填充
    for(unsigned int i = 0; i < v_pvm.size(); i++){
        //更新此类型的预测虚拟机的数量
        for(unsigned int j = 0; j < m_predict_vm.size(); j++){
            if(v_pvm[i].vm_name == m_predict_vm[j].vm_name){
                // m_predict_vm[j].vm_num += max_num;
                //看下此类型有几个
                m_predict_vm[j].vm_num -= v_pvm[i].vm_num;
                m_predict_vm_num -= v_pvm[i].vm_num;
                //如果为0，则将此类型预测的虚拟机删除
                if(m_predict_vm[j].vm_num == 0){
                    //删除该vector的虚拟机
                    vector<PredictVM>::iterator it = m_predict_vm.begin() + j;
                    m_predict_vm.erase(it);
                    //同时移动j
                    j--;
                }

            }
        }
    }
    //删除该服务器
    vector<vector<PredictVM> >::iterator it = m_predict_py_server.begin() + num;
    m_predict_py_server.erase(it);
    m_predict_py_server_num--;
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
        p_vm2 = PredictOneKindVM_LastPredictDay(m_vm_kind[i].vm_name);
        // if(p_vm1.vm_num > 0 ){
        //     //对预测结果放大1.2倍
        //     p_vm1.vm_num = (int)(p_vm1.vm_num * 1.2 + 0.5);
        // }
        // if(p_vm2.vm_num > 0 ){
        //     //对预测结果放大1.2倍
        //     p_vm2.vm_num = (int)(p_vm2.vm_num * 1.2 + 0.5);
        // }
        PredictVM p_vm = p_vm1;
        p_vm.vm_num = (int)(p_vm1.vm_num * 0.5 + p_vm2.vm_num * 0.5 + 0.5);
        
        if(p_vm.vm_num > 0 ){
            //对预测结果放大1.2倍
            // p_vm.vm_num = (int)(p_vm.vm_num * 1.2 + 0.5);

            m_predict_vm.push_back(p_vm);
            //将虚拟机总数累加
            m_predict_vm_num += p_vm.vm_num;
        }
    }
}


bool DevCloud::CheckVMMEMSize(){
    bool result = true;
    for(vector<VirtualMachine>::iterator it = m_vm_kind.begin(); it != m_vm_kind.end(); it++){
        if(0 != it->mem_size % 1024){
            result = false;
            break;
        }
    }
    return result;
}


void DevCloud::DongTai(){
    //将服务器、虚拟机的内存规格都以GB为单位，这样使用动态规划能减少1024倍
    m_py_server.mem_size /= 1024;
    for(vector<VirtualMachine>::iterator it = m_vm_kind.begin(); it != m_vm_kind.end(); it++){
        it->mem_size /= 1024;
    }

    //跑一千次，找最优的
    int predict_py_server_num = 0;
    vector<vector<PredictVM> > predict_py_server;

    for(int i = 0; i < 1000; i++){
        //将预测的虚拟机排序并展开
        //第一步：拆箱：将预测的虚拟机全部拆开为单独的
        vector<RemainderVM> v_rvm = Unpacking();
        //第二步：根据需要优化的资源进行排序
        //第二步，将所有虚拟机的循序打乱
        random_shuffle(v_rvm.begin(), v_rvm.end());

        if(m_optimize_resource == string("CPU")){
            // sort(v_rvm.begin(), v_rvm.end(), SortVMByCPU);
            vector<vector<PredictVM> > py_server = OptimizeByCPU(v_rvm);
            TransferServer(py_server);
        }
        else{
            // sort(v_rvm.begin(), v_rvm.end(), SortVMByMEM);
            vector<vector<PredictVM> > py_server = OptimizeByMEM(v_rvm);
            TransferServer(py_server);
        }

        if(i == 0){
            predict_py_server_num = m_predict_py_server_num;
            predict_py_server = m_predict_py_server;
        }
        if(m_predict_py_server_num < predict_py_server_num){
            predict_py_server_num = m_predict_py_server_num;
            predict_py_server = m_predict_py_server;
        }
        //重新跑
        m_predict_py_server_num = 0;
        m_predict_py_server.clear();
    }

    //将最小的复制回来
    m_predict_py_server_num = predict_py_server_num;
    m_predict_py_server = predict_py_server;

    // vector<vector<PredictVM> > py_server = OptimizeByCPU(v_rvm);
    // vector<vector<PredictVM> > py_server = OptimizeByMEM(v_rvm);
    // TransferServer(py_server);
}

vector<vector<PredictVM> > DevCloud::OptimizeByMEM(vector<RemainderVM>& v_rvm){
    vector<vector<PredictVM> > py_server;
    PhysicalServer ps;
    ps.core_num = m_py_server.core_num;
    ps.mem_size = m_py_server.mem_size;
    ps.hd_size = m_py_server.hd_size;
    while(!v_rvm.empty()){
        //构建一个行为v_rvm个数，列为物理服务器CPU数+1的数组
        vector<vector<PhysicalServer> > dp;
        for(unsigned int i = 0; i < v_rvm.size(); i++){
            vector<PhysicalServer> temp(m_py_server.mem_size+1,ps);
            dp.push_back(temp);
        }
        //开始规划
        dp[0][m_py_server.mem_size - v_rvm[0].mem_size] = ps;
        dp[0][m_py_server.mem_size - v_rvm[0].mem_size].core_num -= v_rvm[0].core_num;
        dp[0][m_py_server.mem_size - v_rvm[0].mem_size].mem_size -= v_rvm[0].mem_size;

        unsigned int i = 1;
        int j = m_py_server.mem_size;
        for(i = 1; i < v_rvm.size(); i++){
            for(j = m_py_server.mem_size; j >= 0; j--){
                PhysicalServer ps_remainder;
                ps_remainder = dp[i-1][j];
                //尤其注意界限问题，同时还要考虑两个维度，暂时以CPU为主要优化
                if(j + v_rvm[i].mem_size <= m_py_server.mem_size){
                    //判断此剩余空间是否能将虚拟机放下，如果能则比较大小，如果不能则不放下
                    if(dp[i-1][j+v_rvm[i].mem_size].core_num -v_rvm[i].core_num >= 0 &&
                       dp[i-1][j+v_rvm[i].mem_size].mem_size -v_rvm[i].mem_size >= 0 &&
                       dp[i-1][j+v_rvm[i].mem_size].mem_size == j+v_rvm[i].mem_size){
                        //需要小心处理，比较与dp[i-1][j]之间信息的大小
                        if(dp[i-1][j+v_rvm[i].mem_size].mem_size -v_rvm[i].mem_size < dp[i-1][j].mem_size){
                            ps_remainder = dp[i-1][j+v_rvm[i].mem_size];
                            ps_remainder.core_num -= v_rvm[i].core_num;
                            ps_remainder.mem_size -= v_rvm[i].mem_size;
                        }
                    }
                    else{
                        ps_remainder = dp[i-1][j];
                    }
                }
                dp[i][j] = ps_remainder;
            }
            //如果此时CPU剩余空间为0，则退出，记录上此时的i，j
            if(dp[i][0].mem_size == 0){
                break;
            }
        }
        //找到对应的路径，压入py_server中, 并将其从v_rvm中删除
        //向后寻找，一直到v_rvm[0]
        //先找到剩余最小空间的位置i,j
        //需要将i从尾开始寻找
        if(i == v_rvm.size()){
            i = v_rvm.size() - 1;
        }
        for(j = 0; j <= m_py_server.mem_size; j++){
            if(dp[i][j].mem_size == j){
                break;
            }
        }

        // i = v_rvm.size() - 1;
        //再反向寻找，如果dp[i][j]与dp[i-1][j]相等，则此v_rvm[i]没有在路径上，否则压入，并删除
        vector<PredictVM> vpvm;
        while(i > 0){
            if(dp[i][j].mem_size == dp[i-1][j].mem_size){
                i--;
            }
            else{
                //将此虚拟机压入一台服务器中
                PredictVM pvm;
                pvm.vm_name = v_rvm[i].p_vm.vm_name;
                pvm.vm_num = 1;
                vpvm.push_back(pvm);
                //继续查找
                j += v_rvm[i].mem_size;
                //将此虚拟机删除
                vector<RemainderVM>::iterator iter = v_rvm.begin() + i;
                v_rvm.erase(iter);
                i--;
            }
        }
        //需要判断i == 0的虚拟机时候用上，
        if(dp[0][j].mem_size == j && j != m_py_server.mem_size){
            //将此虚拟机压入一台服务器中
            PredictVM pvm;
            pvm.vm_name = v_rvm[i].p_vm.vm_name;
            pvm.vm_num = 1;
            vpvm.push_back(pvm);
            //将此虚拟机删除
            vector<RemainderVM>::iterator iter = v_rvm.begin() + i;
            v_rvm.erase(iter);
        }
        //将此服务器放入到总的服务器中
        if(!vpvm.empty()){
            py_server.push_back(vpvm); 
        }         
    }
    return py_server;
}

vector<vector<PredictVM> > DevCloud::OptimizeByCPU(vector<RemainderVM>& v_rvm){
    vector<vector<PredictVM> > py_server;
    PhysicalServer ps;
    ps.core_num = m_py_server.core_num;
    ps.mem_size = m_py_server.mem_size;
    ps.hd_size = m_py_server.hd_size;
    while(!v_rvm.empty()){
        //构建一个行为v_rvm个数，列为物理服务器CPU数+1的数组
        vector<vector<PhysicalServer> > dp;
        for(unsigned int i = 0; i < v_rvm.size(); i++){
            vector<PhysicalServer> temp(m_py_server.core_num+1,ps);
            dp.push_back(temp);
        }
        //开始规划
        dp[0][m_py_server.core_num - v_rvm[0].core_num] = ps;
        dp[0][m_py_server.core_num - v_rvm[0].core_num].core_num -= v_rvm[0].core_num;
        dp[0][m_py_server.core_num - v_rvm[0].core_num].mem_size -= v_rvm[0].mem_size;

        unsigned int i = 1;
        int j = m_py_server.core_num;
        for(i = 1; i < v_rvm.size(); i++){
            for(j = m_py_server.core_num; j >= 0; j--){
                PhysicalServer ps_remainder;
                ps_remainder = dp[i-1][j];
                //尤其注意界限问题，同时还要考虑两个维度，暂时以CPU为主要优化
                if(j + v_rvm[i].core_num <= m_py_server.core_num){
                    //判断此剩余空间是否能将虚拟机放下，如果能则比较大小，如果不能则不放下
                    if(dp[i-1][j+v_rvm[i].core_num].core_num -v_rvm[i].core_num >= 0 &&
                       dp[i-1][j+v_rvm[i].core_num].mem_size -v_rvm[i].mem_size >= 0 &&
                       dp[i-1][j+v_rvm[i].core_num].core_num == j+v_rvm[i].core_num){
                        //需要小心处理，比较与dp[i-1][j]之间信息的大小
                        if(dp[i-1][j+v_rvm[i].core_num].core_num -v_rvm[i].core_num < dp[i-1][j].core_num){
                            ps_remainder = dp[i-1][j+v_rvm[i].core_num];
                            ps_remainder.core_num -= v_rvm[i].core_num;
                            ps_remainder.mem_size -= v_rvm[i].mem_size;
                        }
                    }
                    else{
                        ps_remainder = dp[i-1][j];
                    }
                }
                dp[i][j] = ps_remainder;
            }
            //如果此时CPU剩余空间为0，则退出，记录上此时的i，j
            if(dp[i][0].core_num == 0){
                break;
            }
        }
        //找到对应的路径，压入py_server中, 并将其从v_rvm中删除
        //向后寻找，一直到v_rvm[0]
        //先找到剩余最小空间的位置i,j
        //需要将i从尾开始寻找
        if(i == v_rvm.size()){
            i = v_rvm.size() - 1;
        }
        for(j = 0; j <= m_py_server.core_num; j++){
            if(dp[i][j].core_num == j){
                break;
            }
        }
        //再反向寻找，如果dp[i][j]与dp[i-1][j]相等，则此v_rvm[i]没有在路径上，否则压入，并删除
        vector<PredictVM> vpvm;
        while(i > 0){
            if(dp[i][j].core_num == dp[i-1][j].core_num){
                i--;
            }
            else{
                //将此虚拟机压入一台服务器中
                PredictVM pvm;
                pvm.vm_name = v_rvm[i].p_vm.vm_name;
                pvm.vm_num = 1;
                vpvm.push_back(pvm);
                //继续查找
                j += v_rvm[i].core_num;
                //将此虚拟机删除
                vector<RemainderVM>::iterator iter = v_rvm.begin() + i;
                v_rvm.erase(iter);
                i--;
            }
        }
        //需要判断i == 0的虚拟机时候用上，
        if(dp[0][j].core_num == j && j != m_py_server.core_num){
            //将此虚拟机压入一台服务器中
            PredictVM pvm;
            pvm.vm_name = v_rvm[i].p_vm.vm_name;
            pvm.vm_num = 1;
            vpvm.push_back(pvm);
            //将此虚拟机删除
            vector<RemainderVM>::iterator iter = v_rvm.begin() + i;
            v_rvm.erase(iter);
        }
        //将此服务器放入到总的服务器中
        if(!vpvm.empty()){
            py_server.push_back(vpvm); 
        }         
    }
    return py_server;
}


void DevCloud::LinearRegression(){
    //将所有虚拟机放在一起训练出来一组参数
    vector<double> w = TrainOnline();

    //对输入的每一种类型的虚拟机进行预测
    PredictVM p_vm;
    for(unsigned int i = 0; i < m_vm_kind.size(); i++){
        p_vm = PredictOneKindVM_LR(m_vm_kind[i].vm_name, w);
        if(p_vm.vm_num > 0){
            //对预测结果放大1.2倍
            // p_vm.vm_num = (int)(p_vm.vm_num * 1.2 + 0.5);
            // p_vm.vm_num = (int)(p_vm.vm_num / 1.3 + 0.5);
            m_predict_vm.push_back(p_vm);
            //将虚拟机总数累加
            m_predict_vm_num += p_vm.vm_num;
        }
    }
}

PredictVM DevCloud::PredictOneKindVM_LR(string flavor, vector<double> w){
    //将信息补充完全
    //将此种类型的虚拟机统计出来
    map<int,int> vm = StatisticsOneKindVM(flavor);
    // vector<int> one_day(m_max_day - m_min_day + 1, 0);
    vector<int> one_day(StringToDatetime(m_predict_begin_time) - m_min_day, 0);

    for(map<int,int>::iterator it = vm.begin(); it != vm.end(); it++){
        one_day[it->first - m_min_day] = it->second;
    }
    //根据需要预测的天数进行滑窗
    // int last_day = m_max_day - m_min_day + 1 - m_predict_day;
    int last_day = StringToDatetime(m_predict_begin_time) - m_min_day - m_predict_day;
    vector<int> sum_day(last_day + 1, 0);
    for(int i = 0; i<= last_day; i++){
        for(int j = 0; j < m_predict_day; j++){
            sum_day[i] += one_day[i+j];
        }
    }
    //需要对sum_day进行滤波处理
    // sum_day = PrecessOutlier(sum_day);
    //开始使用距离权重去预测  
    double total_day = 0.0;
    int sum_day_size = sum_day.size() - 1;
    //第一种方法，使用最后10天的数据
    // for(int i = 0; i < 10; i++){
    //     total_day += sum_day[sum_day_size - i] / 2.0 /(i + 2);
    // }
    //第二种方法，使用最后30天的数据
    // for(int i = 0; i < 30; i++){
    //     total_day += sum_day[sum_day_size - i] / 3.0 /(i + 2);
    // }
    //第三种方法
    // total_day = total_day + sum_day[sum_day_size] * 0.6;
    // total_day = total_day + sum_day[sum_day_size-1] * 0.3;
    // total_day = total_day + sum_day[sum_day_size-2] * 0.2;

    //第四种方法，使用线性回归进行训练
    //构造label，选择一周的量，即7个label
    //选择m个label
    // int m = 30;
    // vector<int> y;
    // for(int i = sum_day_size; i > sum_day_size - m; i--){
    //     y.push_back(sum_day[i]);
    // }
    // //构造特征X，选择3个特征，选择n个特征
    // int n = 1;
    // vector<vector<int> > X;
    // for(int i = 0; i < m; i++){
    //     vector<int> sample_x;
    //     sample_x.clear();
    //     for(int j = 0; j < n; j++){
    //         sample_x.push_back(sum_day[sum_day_size - m_predict_day - i - j]);
    //     }
    //     X.push_back(sample_x);
    // }
    // vector<double> w = Train(X,y); 

    //对新的数据进行预测
    // total_day = w[0] * sum_day[sum_day_size] + w[1] * sum_day[sum_day_size - 1] + w[2] * sum_day[sum_day_size - 2] + w[3];
    for(unsigned int i = 0; i < w.size() - 1; i++){
        total_day += w[i] * sum_day[sum_day_size - i];
    }
    total_day += w[w.size() - 1];



    PredictVM pvm;
    pvm.vm_name = flavor;
    pvm.vm_num = (int)(total_day + 0.5);
    // pvm.vm_num = sum_day[sum_day.size()-1];

    return pvm;
}

vector<double> Train(vector<vector<int> > X, vector<int> y){
    // int sample_size = X.size();
    int feature_size = X[0].size();
    //初始化参数
    vector<double> w(feature_size+1,0.5);
    //计算损失函数，损失函数两次区别不大，则停止训练，迭代50次
    double learn_rate = 0.000001;
    // double learn_rate = 0.0000001;
    int i = 0;
    while(i < 10000){
        //计算损失函数
        double pre_loss = ComputeLoss(w,X,y);
        //跟新参数
        GradientDescent(w,X,y,learn_rate);
        //再次计算损失函数
        double loss = ComputeLoss(w,X,y);
        //如果两次下降较下，则停止迭代
        if(pre_loss - loss < 0.000001){
            break;
        }
        i++;
    }
    return w;
}

double ComputeLoss(vector<double> &w, vector<vector<int> > &X, vector<int> &y){
    double loss = 0.0;
    int sample_size = X.size();
    int feature_size = X[0].size();
    double predict = 0.0;
    for(int i = 0; i < sample_size; i++){
        predict = 0.0;
        for(int j = 0; j < feature_size; j++){
            predict += w[j] * X[i][j];
        }
        predict += w[feature_size];
        loss += (y[i] - predict) * (y[i] - predict);
    }
    //加入L2正则
    // double lambda = 1000;
    // for(int i = 0; i < feature_size; i++){
    //     loss += lambda * w[i] * w[i];
    // }


    loss = loss/2.0/sample_size;
    return loss;
}

void GradientDescent(vector<double> &w,vector<vector<int> > &X, vector<int> &y, double learn_rate){
    //计算梯度
    // double gradient = 0.0;
    int sample_size = X.size();
    int feature_size = X[0].size();
    double predict_y = 0.0;

    vector<double> gradient(feature_size+1, 0.0);
    vector<double> predict(sample_size, 0.0);

    for(int i = 0; i < sample_size; i++){
        predict_y = 0.0;
        for(int j = 0; j < feature_size; j++){
            predict_y += w[j] * X[i][j];
        }
        predict_y += w[feature_size];

        predict[i] = predict_y;
    }

    double lambda = 1000;

    //计算w的梯度
    double gradient_one = 0.0;
    for(int i = 0; i < feature_size; i++){
        gradient_one = 0.0;
        for(int j = 0; j < sample_size; j++){
            gradient_one += (predict[j] - y[j]) * X[j][i];
        }
        gradient_one /= sample_size;
        //加入L2正则项
        // gradient_one += lambda * w[i] / sample_size;

        gradient[i] = gradient_one;
    }
    //计算b的梯度
    gradient_one = 0.0;
    for(int j = 0; j < sample_size; j++){
        gradient_one += (predict[j] - y[j]);
    }
    gradient_one /= sample_size;

    gradient[feature_size] = gradient_one;


    //更新参数
    for(int i = 0; i <= feature_size; i++){
        w[i] -= learn_rate * gradient[i];
    }
}


void DevCloud::CheckResult(){
    //如果在需要预测的中，但是预测为0，则添加进来
    // vector<VirtualMachine> m_vm_kind;                     //虚拟机规格
    // vector<PredictVM> m_predict_vm;                       //预测虚拟机的情况
    for(unsigned int i = 0; i < m_vm_kind.size(); i++){
        vector<PredictVM>::iterator it = m_predict_vm.begin();
        for(it = m_predict_vm.begin(); it != m_predict_vm.end(); it++){
            if(m_vm_kind[i].vm_name == it->vm_name){
                break;
            }
        }
        if(it == m_predict_vm.end()){
            PredictVM pvm;
            pvm.vm_name = m_vm_kind[i].vm_name;
            pvm.vm_num = 0;
            m_predict_vm.push_back(pvm);
        }
    }
}

vector<double> DevCloud::TrainOnline(){
    //划分df
    vector<vector<int> > df;
    for(unsigned int i = 0; i < m_vm_kind.size(); i++){
        map<int,int> vm = StatisticsOneKindVM(m_vm_kind[i].vm_name);
        vector<int> one_day(StringToDatetime(m_predict_begin_time) - m_min_day, 0);
        for(map<int,int>::iterator it = vm.begin(); it != vm.end(); it++){
            one_day[it->first - m_min_day] = it->second;
        }
        int last_day = StringToDatetime(m_predict_begin_time) - m_min_day - m_predict_day;
        vector<int> sum_day(last_day + 1, 0);
        for(int i = 0; i<= last_day; i++){
            for(int j = 0; j < m_predict_day; j++){
                sum_day[i] += one_day[i+j];
            }
        }
        sum_day = PrecessOutlier(sum_day);
        df.push_back(sum_day);
    }
    //找到label ： y
    int m = 10;
    vector<int> y;
    int n = 3;
    vector<vector<int> > X;

    for(unsigned int i = 0; i < df.size(); i++){
        int sum_day_size = df[0].size() - 1;
        for(int j = sum_day_size; j > sum_day_size - m; j--){
            y.push_back(df[i][j]);
        }

        for(int k = 0; k < m; k++){
            vector<int> sample_x;
            sample_x.clear();
            for(int p = 0; p < n; p++){
                sample_x.push_back(df[i][sum_day_size - m_predict_day - k - p]);
            }
            X.push_back(sample_x);
        }
    }
    return Train(X,y);
}


void DevCloud::ExponentialSmooth(){
    //对输入的每一种类型的虚拟机进行预测
    PredictVM p_vm;
    for(unsigned int i = 0; i < m_vm_kind.size(); i++){
        p_vm = PredictOneKindVM_ES(m_vm_kind[i].vm_name);
        if(p_vm.vm_num > 0){
            //对预测结果放大1.2倍
            // p_vm.vm_num = (int)(p_vm.vm_num * 1.2 + 0.5);
            // p_vm.vm_num = (int)(p_vm.vm_num / 1.3 + 0.5);
            m_predict_vm.push_back(p_vm);
            //将虚拟机总数累加
            m_predict_vm_num += p_vm.vm_num;
        }
    }
}

PredictVM DevCloud::PredictOneKindVM_ES(string flavor){
    //将信息补充完全
    //将此种类型的虚拟机统计出来
    map<int,int> vm = StatisticsOneKindVM(flavor);
    // vector<int> one_day(m_max_day - m_min_day + 1, 0);
    vector<int> one_day(StringToDatetime(m_predict_begin_time) - m_min_day, 0);

    for(map<int,int>::iterator it = vm.begin(); it != vm.end(); it++){
        one_day[it->first - m_min_day] = it->second;
    }

    //计算总的个数
    int total_val = StringToDatetime(m_predict_begin_time) - m_min_day;
    total_val /= m_predict_day;

    vector<int> sum_day(total_val, 0);

    for(int i = 0; i < total_val; i++){
        for(int j = 0; j < m_predict_day; j++){
            sum_day[i] += one_day[one_day.size() - 1 - i * m_predict_day - j];
        }
    }

    //将sum_day逆序
    reverse(sum_day.begin(),sum_day.end());

    /****************************************
     * 一次滑窗，a从[0,1:0.1]，取使MSE最小的a
     * 历史数据，取三周，21天的
    ****************************************/
    //使用最优的opt_a进行一次滑窗
    //计算初值
    // double initial = (sum_day[0] + sum_day[1] + sum_day[2]) / 3.0;
     double initial = (sum_day[0] + sum_day[1]) / 3.0;
    //初始化a为一组向量，选择MSE最小的
    vector<double> a;
    for(int i = 1; i < 10; i++){
        a.push_back(i / 10.0);
    }
    double opt_a = 0.0;
    double opt_mse = 0.0;
    //开始一次滑动
    for(unsigned int i = 0; i < a.size(); i++){
        vector<double> OneTimeSmooth(sum_day.size(),0.0);
        double mse = 0.0;
        //计算mse
        for(unsigned int j = 0; j < sum_day.size(); j++){
            if(j == 0){
                OneTimeSmooth[j] = initial;
            }
            else{
                OneTimeSmooth[j] = opt_a * sum_day[j-1] + (1 - opt_a) * OneTimeSmooth[j-1];
            }
            mse += (sum_day[j] - OneTimeSmooth[j]) * (sum_day[j] - OneTimeSmooth[j]);
        }

        if(i == 0){
            opt_mse = mse;
            opt_a = a[i];
        }
        else{
            if(mse < opt_mse){
                opt_mse = mse;
                opt_a = a[i];
            }
        }
    }
    //使用最优的opt_a进行一次滑窗
    opt_a = 0.6;
    vector<double> OneTimeSmooth(sum_day.size(),0.0);
    for(unsigned int j = 0; j < sum_day.size(); j++){
        if(j == 0){
            OneTimeSmooth[j] = initial;
        }
        else{
            // OneTimeSmooth[j] = opt_a * sum_day[j-1] + (1 - opt_a) * OneTimeSmooth[j-1];
            OneTimeSmooth[j] = sum_day[j-1] + (1 - opt_a) * OneTimeSmooth[j-1];
        }
    }


    //对最后的值进行预测
    double total_day = 0.0;
    // total_day = opt_a * original[original.size()-1] + (1 - opt_a) * OneTimeSmooth[original.size()-1];
    total_day = opt_a * sum_day[sum_day.size()-1] + (1 - opt_a) * OneTimeSmooth[sum_day.size()-1];

    PredictVM pvm;
    pvm.vm_name = flavor;
    pvm.vm_num = (int)(total_day + 0.5);
    // pvm.vm_num = sum_day[sum_day.size()-1];

    return pvm;
}



// 二次滑窗，效果实在太差
// PredictVM DevCloud::PredictOneKindVM_ES(string flavor){
//     //将信息补充完全
//     //将此种类型的虚拟机统计出来
//     map<int,int> vm = StatisticsOneKindVM(flavor);
//     // vector<int> one_day(m_max_day - m_min_day + 1, 0);
//     vector<int> one_day(StringToDatetime(m_predict_begin_time) - m_min_day, 0);

//     for(map<int,int>::iterator it = vm.begin(); it != vm.end(); it++){
//         one_day[it->first - m_min_day] = it->second;
//     }

//     //计算总的个数
//     int total_val = StringToDatetime(m_predict_begin_time) - m_min_day;
//     total_val /= m_predict_day;

//     vector<int> sum_day(total_val, 0);

//     for(int i = 0; i < total_val; i++){
//         for(int j = 0; j < m_predict_day; j++){
//             sum_day[i] += one_day[one_day.size() - 1 - i * m_predict_day - j];
//         }
//     }

//     //将sum_day逆序
//     reverse(sum_day.begin(),sum_day.end());

//     /****************************************
//      * 二次滑窗，a从[0,1:0.1]，取使MSE最小的a
//      * 历史数据，取三周，21天的
//     ****************************************/
//     //使用最优的opt_a进行一次滑窗
//     //计算初值
//     double initial = (sum_day[0] + sum_day[1] + sum_day[2]) / 3.0;
//     // double initial = (sum_day[0] + sum_day[1]) / 3.0;
//     double opt_a = 0.0;
//     //开始一次滑动
//     //使用最优的opt_a进行一次滑窗
//     opt_a = 0.6;
//     vector<double> OneTimeSmooth(sum_day.size(),0.0);
//     for(unsigned int j = 0; j < sum_day.size(); j++){
//         if(j == 0){
//             OneTimeSmooth[j] = initial;
//         }
//         else{
//             // OneTimeSmooth[j] = opt_a * sum_day[j-1] + (1 - opt_a) * OneTimeSmooth[j-1];
//             OneTimeSmooth[j] = sum_day[j-1] + (1 - opt_a) * OneTimeSmooth[j-1];
//         }
//     }
//     //进行二次滑窗
//     vector<double> TwoTimeSmooth(sum_day.size(),0.0);
//     for(unsigned int j = 0; j < sum_day.size(); j++){
//         if(j == 0){
//             TwoTimeSmooth[j] = initial;
//         }
//         else{
//             TwoTimeSmooth[j] = opt_a * OneTimeSmooth[j] + (1 - opt_a) * TwoTimeSmooth[j-1];
//             // TwoTimeSmooth[j] = OneTimeSmooth[j] + (1 - opt_a) * TwoTimeSmooth[j-1];
//         }
//     }

//     //对最后的值进行预测
//     double total_day = 0.0;
//     // total_day = opt_a * original[original.size()-1] + (1 - opt_a) * OneTimeSmooth[original.size()-1];
//     // total_day = opt_a * sum_day[sum_day.size()-1] + (1 - opt_a) * OneTimeSmooth[sum_day.size()-1];

//     double At = 2.0 * OneTimeSmooth[OneTimeSmooth.size() - 1] - TwoTimeSmooth[TwoTimeSmooth.size() - 1];
//     double Bt = (opt_a / (1 - opt_a)) * (OneTimeSmooth[OneTimeSmooth.size() - 1] - TwoTimeSmooth[TwoTimeSmooth.size() - 1]);
//     total_day = At + Bt;

//     PredictVM pvm;
//     pvm.vm_name = flavor;
//     pvm.vm_num = (int)(total_day + 0.5);
//     // pvm.vm_num = sum_day[sum_day.size()-1];

//     return pvm;
// }