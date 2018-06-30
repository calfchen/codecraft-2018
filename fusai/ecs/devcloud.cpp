#include "devcloud.h"

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

void DevCloud::ReadInput(char * info[]){
    //读入物理服务器类型数量
    if(info[0] != NULL){
        sscanf(info[0],"%d",&m_py_server_num);
    }
    //读入物理服务器属性
    PhysicalServer ps;
    for(int i = 1; i < 1 + m_py_server_num; i++){
        char str[20];
        sscanf(info[i],"%s %d %d %d",str,&ps.core_num,&ps.mem_size,&ps.hd_size);
        ps.mem_size *= 1024;        //转化为以MB为单位
        ps.name = str;
        m_py_server.push_back(ps);
    }
    //读入虚拟机规格数量
    if(info[2+m_py_server_num] != NULL){
        sscanf(info[2+m_py_server_num],"%d",&m_vm_kind_num);
    }
    //读入虚拟机种类
    VirtualMachine vm;
    for(int i = 3 + m_py_server_num; i < 3 + m_py_server_num + m_vm_kind_num; i++){
        char str[12];
        sscanf(info[i],"%s %d %d",str,&vm.core_num,&vm.mem_size);
        vm.vm_name = str;
        m_vm_kind.push_back(vm);
    }
    //读入需要预测的时间段
    if(info[4 + m_py_server_num + m_vm_kind_num] != NULL){
        char str[24];
        sscanf(info[4 + m_py_server_num + m_vm_kind_num],"%s",str);
        m_predict_begin_time = str;
    }
    if(info[5 + m_py_server_num + m_vm_kind_num] != NULL){
        char str[24];
        sscanf(info[5 + m_py_server_num + m_vm_kind_num],"%s",str);
        m_predict_end_time = str;
    }
    //计算需要预测的天数
    m_predict_day = (StringToDatetime(m_predict_end_time) - StringToDatetime(m_predict_begin_time));
    m_predict_day++;
    if(m_predict_day <= 0){
        m_predict_day++;
    }
    //多算一天的
    // m_predict_day++;
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

    //物理服务器分配情况
    for(unsigned int i = 0; i < m_predict_py_server.size(); i++){
        if(m_predict_py_server[i].py_server_num > 0){
            //加入物理服务器名称
            result += m_predict_py_server[i].py_server_name;
            //加入装入虚拟机总数量
            result += " ";
            sprintf(cshuzi, "%d", m_predict_py_server[i].py_server_num); 
            sshuzi = cshuzi;
            result += sshuzi;
            result += "\n";
            //对此类型的每一台服务器进行添加
            for(unsigned int j = 0; j < m_predict_py_server[i].onekind_py_server.size(); j++){
                
                sshuzi = m_predict_py_server[i].py_server_name;
                sshuzi += "-";
                sprintf(cshuzi, "%d", j+1);         
                sshuzi += cshuzi;
                result += sshuzi;
                result += " ";

                for(unsigned int k = 0; k < m_predict_py_server[i].onekind_py_server[j].size(); k++){
                    result += m_predict_py_server[i].onekind_py_server[j][k].vm_name;
                    result += " ";
                    sprintf(cshuzi, "%d", m_predict_py_server[i].onekind_py_server[j][k].vm_num); 
                    sshuzi = cshuzi;
                    result += sshuzi;
                    //如果不是最后一个，则需要添加空格
                    if(k != m_predict_py_server[i].onekind_py_server[j].size()-1){
                        result += " ";
                    }
                }
                result += "\n";
            }
            //不同类型之间需要加空行
            if(i != m_predict_py_server.size() - 1){
                result += "\n";
            }
        }
    }

    //测试物理服务器的种类个数
    // if(m_py_server_num == 3){
    //     return "";
    // }

    //测试线上物理服务器每种的规格
    // if(m_py_server[0].name == string("General") &&
    //     m_py_server[1].name == string("Large-Memory") &&
    //     m_py_server[2].name == string("High-Performance")){
        
    //     return "";
    // }
    // if(m_py_server[0].name == string("General") &&
    //     m_py_server[0].core_num == 56 &&
    //     m_py_server[0].mem_size == 128*1024){
    //     return "";
    // }
    // if(m_py_server[1].name == string("Large-Memory") &&
    //     m_py_server[1].core_num == 84 &&
    //     m_py_server[1].mem_size == 256*1024){
    //     return "";
    // }
    // if(m_py_server[2].name == string("High-Performance") &&
    //     m_py_server[2].core_num == 112 &&
    //     m_py_server[2].mem_size == 192*1024){
    //     return "";
    // }

    //测试线上训练集起止时间
    // if(m_train_df_num >= 5000){
    //     return "";
    // }
    // if(m_max_day - m_min_day >= 70){
    //     return "";
    // }
    // if(m_min_day == StringToDatetime("2016-05-01")){
    //     return "";
    // }
    // if(m_max_day <= StringToDatetime("2016-07-15")){
    //     return "";
    // }

    //测试预测的情况
    // if(StringToDatetime(m_predict_begin_time) - m_max_day == 8){
    //     return "";
    // }
    // if(m_predict_day >= 7){
    //     return "";
    // }

    //测试虚拟机规格
    // if(m_vm_kind_num <= 8){
    //     return "";
    // }
    // for(unsigned int i = 0; i < m_vm_kind.size(); i++){
    //     if(m_vm_kind[i].mem_size % 1024 != 0){
    //         return "";
    //     }
    // }

    // if(m_predict_py_server[0].py_server_num == 0){
    //     return "";
    // }
    // if(m_predict_vm_num == 0){
    //     return "";
    // }

    // if(m_predict_vm[0].vm_num > 2000){
    //     return "";
    // }
    // if(m_predict_vm[4].vm_num > 200){
    //     return "";
    // }
    // if(m_predict_vm.size() == 8){
    //     return "";
    // }

    return result;
}


void DevCloud::Transform(){
    //检查输入info的虚拟机的个数
    m_predict_vm_num = m_vm_kind_num * m_py_server_num;
    for(int i = 0; i < m_vm_kind_num; i++){
        PredictVM vm;
        vm.vm_name = m_vm_kind[i].vm_name;
        vm.vm_num = m_py_server_num;
        m_predict_vm.push_back(vm); 
    }

    //一个虚拟机分配一台主机
    for(unsigned int i = 0; i < m_py_server.size(); i++){
        VMOfPyServer oneKindPyServer;
        oneKindPyServer.py_server_name = m_py_server[i].name;
        oneKindPyServer.py_server_num = m_predict_vm.size();

        for(unsigned int i = 0; i < m_predict_vm.size(); i++){
            vector<PredictVM> vec_vm;
            vec_vm.push_back(m_predict_vm[i]);
            vec_vm[0].vm_num = 1;
            oneKindPyServer.onekind_py_server.push_back(vec_vm);
        }
        m_predict_py_server.push_back(oneKindPyServer);
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

//将最后的3周最为训练集进行处理,从测试集的前一天begin_day倒数到end_day
vector<int> DevCloud::FindTrain(map<int,int> vm, int total_week){
    // int begin_num = 0, end_num = 21;
    int begin_num = 0, end_num = total_week * 7;
    vector<int> train(end_num - begin_num, 0);
    //计算所需的时间点，服从前闭后开原则
    int end_day = m_max_day - begin_num + 1;
    int begin_day = m_max_day - end_num + 1;

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

    for(unsigned int i = 0; i < train.size(); i++){
        if(train[i] > max){
            train[i] = max;
            // train[i] = (int)(train[i] * 0.7);
        }
    }
    return train;
}

PredictVM DevCloud::PredictOneKindVM_GuiZe(string flavor, int total_week){
    //将此种类型的虚拟机统计出来
    map<int,int> vm = StatisticsOneKindVM(flavor);
    //划分出来训练集
    vector<int> train_temp = FindTrain(vm, total_week);
    // vector<int> train = FindTrain(vm);
    //对划分出来的训练集进行异常值处理，用四分位数发对高值处理
    vector<int> train = PrecessOutlier(train_temp);

    vector<float> week_mean;
    //计算三周的周均值
    for(int i = 0; i < total_week; i++){
        int sum = 0;
        for(int j = 0; j < 7; j++){
            sum += train[i*7 + j];
        }
        float mean = sum / 7.0;
        week_mean.push_back(mean);
    }
    //对此train进行规则预测
    //定义3个vector<int>，长度各为7
    vector< vector<int> > week_original_value(total_week);
    vector< vector<float> > week_factor(total_week);
    //根据train对week_Original_value赋值
    for(int i = 0; i < total_week; i++){
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
        vector<float> factor(total_week,0.0);
        for(int j = 0; j < total_week; j++){
            factor[j] = week_factor[j][i];
        }
        //对factor排序
        sort(factor.begin(),factor.end());

        factor_median[i] = factor[total_week / 2];

        float mean_total = 0.0;
        for(int k = 0; k < total_week; k++){
            mean_total += factor[k];
        }
        factor_mean[i] = mean_total / total_week;
    }
    //求base，利用最后一周的均值
    //或者最后3,5天的均值
    int last_week_total = 0;
    //最后几天的平均值
    int last_day = 7;                       
    for(int i = (7 - last_day); i < 7; i++){
        // if(week_factor[2][i] < 0.001){
        //     last_week_total += week_original_value[2][i];
        // }
        // else{
        //     last_week_total += week_original_value[2][i] / week_factor[2][i];
        // }
        last_week_total += week_original_value[total_week-1][i];
        
    }
    // float base = last_week_total / (last_day * 1.0);

    //在加上3天的平均
    int ran_last_day = 3;
    for(int i = (7 - ran_last_day); i < 7; i++){
        last_week_total += week_original_value[total_week-2][i];
    }
    float base = last_week_total / ((last_day + ran_last_day) * 1.0);
    
    //用base乘以factor的中位数，即得相应预测的天数
    //或者用factor的均值，或者两者的组合
    
    //定义需要预测的天数
    vector<int> predict_day(m_predict_day,0);
    PredictVM p_vm;
    p_vm.vm_name = flavor;
    p_vm.vm_num = 0;

    //预测的开始天可能不连续
    int separate_day = StringToDatetime(m_predict_begin_time) - m_max_day - 1;

    for(unsigned int i = 0; i < predict_day.size(); i++){
        // predict_day[i] = (int)(base * factor_median[(i+separate_day) % 7] + 0.5);
        // predict_day[i] = (int)(base * factor_mean[(i+separate_day) % 7] + 0.5);
        predict_day[i] = (int)(base * (factor_mean[(i+separate_day) % 7] * 0.5 + factor_median[(i+separate_day) % 7] * 0.5)+ 0.5);
        // predict_day[i] = 3;
        p_vm.vm_num += predict_day[i];
    }

    return p_vm;
}

//使用规则进行预测
void DevCloud::GuiZe(){
    //对输入的每一种类型的虚拟机进行预测
    PredictVM p_vm;
    for(unsigned int i = 0; i < m_vm_kind.size(); i++){
        p_vm = PredictOneKindVM_GuiZe(m_vm_kind[i].vm_name, 5);
        if(p_vm.vm_num > 0){
            //对预测结果放大1.2倍
            // p_vm.vm_num = (int)(p_vm.vm_num / 1.1 + 0.5);
            // p_vm.vm_num = (int)(p_vm.vm_num / 1.5 + 0.5);
            // p_vm.vm_num += 35;
            // srand(i+2);
            // int ddd = rand();
            // p_vm.vm_num += (ddd % p_vm.vm_num);

            if(p_vm.vm_num > 500){
                p_vm.vm_num = (int)(p_vm.vm_num * 1.25 + 0.5);
            }
            else if(p_vm.vm_num > 100 && p_vm.vm_num <= 500){
                p_vm.vm_num = (int)(p_vm.vm_num * 0.7);
            }
            else{
                p_vm.vm_num = (int)(p_vm.vm_num * 3.0 + 0.5);
            }

            m_predict_vm.push_back(p_vm);
            //将虚拟机总数累加
            m_predict_vm_num += p_vm.vm_num;
        }
    }
}

//将需要处理的虚拟机建立索引
void DevCloud::VMToMap(){
    for(unsigned int i = 0; i < m_vm_kind.size(); i++){
        m_map_vm[m_vm_kind[i].vm_name] = m_vm_kind[i];
    }
}

//将需要处理的服务器建立索引
void DevCloud::PYToMap(){
    for(unsigned int i = 0; i < m_py_server.size(); i++){
        m_map_py[m_py_server[i].name] = m_py_server[i];
    }
}

//对预测的虚拟机进行拆箱
vector<RemainderVM> DevCloud::Unpacking(){
    //需要转化的情况
    vector<RemainderVM> v_rvm;
    for(unsigned int i = 0; i < m_predict_vm.size(); i++){
        for(int j = 0; j < m_predict_vm[i].vm_num; j++){
            RemainderVM rvm;
            rvm.p_vm = m_predict_vm[i];
            //将数量置为1
            rvm.p_vm.vm_num = 1;
            rvm.core_num = m_map_vm[m_predict_vm[i].vm_name].core_num;
            rvm.mem_size = m_map_vm[m_predict_vm[i].vm_name].mem_size;
            v_rvm.push_back(rvm);
        }
    }
    return v_rvm;
}


void DevCloud::DongTai(){
    //将所有服务器的内存修改为以GB为单位
    // for(unsigned int i = 0; i < m_py_server.size(); i++){
    //     m_py_server[i].mem_size /= 1024;
    // }
    // //将所有的虚拟机的内存修改为以GB为单位
    // for(vector<VirtualMachine>::iterator it = m_vm_kind.begin(); it != m_vm_kind.end(); it++){
    //     it->mem_size /= 1024;
    // }

    //选择一种物理服务器
    PhysicalServer ps = m_py_server[2];

    //跑一千次找最优的
    int predict_py_server_num = 0;
    vector<vector<PredictVM> > predict_py_server;

    for(int i = 0; i < 50; i++){
        //第一步：拆箱：将预测的虚拟机全部拆开为单独的
        vector<RemainderVM> v_rvm = Unpacking();
        //第二步，将所有虚拟机的循序打乱
        random_shuffle(v_rvm.begin(), v_rvm.end());
        //第三步，对CPU进行优化
        vector<vector<PredictVM> > py_server = OptimizeByCPU(v_rvm, ps);
        //第四步，进行转移
        TransferServer(py_server, ps);
        //第五步，选择使用数量最小的
        if(i == 0){
            predict_py_server_num = m_predict_py_server[0].py_server_num;
            predict_py_server = m_predict_py_server[0].onekind_py_server;
        }
        if(m_predict_py_server[0].py_server_num < predict_py_server_num){

            predict_py_server_num = m_predict_py_server[0].py_server_num;
            predict_py_server = m_predict_py_server[0].onekind_py_server;
        }
        //重新跑
        m_predict_py_server[0].py_server_num = 0;
        m_predict_py_server[0].onekind_py_server.clear();
    }
    //将最小的复制回来
    m_predict_py_server[0].py_server_num = predict_py_server_num;
    m_predict_py_server[0].onekind_py_server = predict_py_server;
}


//按CPU进行优化
vector<vector<PredictVM> > DevCloud::OptimizeByCPU(vector<RemainderVM>& v_rvm, PhysicalServer ps){
    vector<vector<PredictVM> > py_server;
    while(!v_rvm.empty()){
        //构建一个行为v_rvm个数，列为物理服务器CPU数+1的数组
        vector<vector<PhysicalServer> > dp;
        for(unsigned int i = 0; i < v_rvm.size(); i++){
            vector<PhysicalServer> temp(ps.core_num+1,ps);
            dp.push_back(temp);
        }
        //开始规划
        dp[0][ps.core_num - v_rvm[0].core_num] = ps;
        dp[0][ps.core_num - v_rvm[0].core_num].core_num -= v_rvm[0].core_num;
        dp[0][ps.core_num - v_rvm[0].core_num].mem_size -= v_rvm[0].mem_size;

        unsigned int i = 1;
        int j = ps.core_num;
        for(i = 1; i < v_rvm.size(); i++){
            for(j = ps.core_num; j >= 0; j--){
                PhysicalServer ps_remainder;
                ps_remainder = dp[i-1][j];
                //尤其注意界限问题，同时还要考虑两个维度，暂时以CPU为主要优化
                if(j + v_rvm[i].core_num <= ps.core_num){
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
        for(j = 0; j <= ps.core_num; j++){
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
        if(dp[0][j].core_num == j && j != ps.core_num){
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


void DevCloud::TransferServer(vector<vector<PredictVM> > py_server, PhysicalServer ps){
    //只有一种物理服务器
    VMOfPyServer vm_py;
    vm_py.py_server_name = ps.name;

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
        vm_py.onekind_py_server.push_back(vp_vm);
    }
    //计算总数
    vm_py.py_server_num = vm_py.onekind_py_server.size();
    //将此保存
    m_predict_py_server.clear();
    m_predict_py_server.push_back(vm_py);
}


void DevCloud::CheckResult(){
    //如果在需要预测的中，但是预测为0，则添加进来
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

void DevCloud::CheckUnit(){
    double rate = ComputeUtilizationRate(m_predict_py_server);
    printf("最后的利用率为%f\n",rate);
}

void DevCloud::Greedy(){
    //将所有服务器的内存修改为以GB为单位
    // for(unsigned int i = 0; i < m_py_server.size(); i++){
    //     m_py_server[i].mem_size /= 1024;
    // }
    // //将所有的虚拟机的内存修改为以GB为单位
    // for(vector<VirtualMachine>::iterator it = m_vm_kind.begin(); it != m_vm_kind.end(); it++){
    //     it->mem_size /= 1024;
    // }

    //跑一千次找最优的
    vector<VMOfPyServer> v_py_server;
    double max_rate = 0.0;

    for(int i = 0; i < 10; i++){
        //第一步：拆箱：将预测的虚拟机全部拆开为单独的
        vector<RemainderVM> v_rvm = Unpacking();
        //第二步：将所有虚拟机的循序打乱
        random_shuffle(v_rvm.begin(), v_rvm.end());
        //第三步：选择一种方案
        v_py_server = GreedySelect(v_rvm);
        //第四步：计算该分布的资源利用率
        double rate = ComputeUtilizationRate(v_py_server);
        //第五步：选择资源利用率最大的方案
        if(rate > max_rate){
            max_rate = rate;
            m_predict_py_server = v_py_server;
        }
        printf("第%d次，此次利用率为%f.\n",i+1,max_rate);
    }
}

//一次只最优化一个服务器，使该服务器的资源利用率达到最大
vector<VMOfPyServer> DevCloud::GreedySelect(vector<RemainderVM>& v_rvm){
    vector<VMOfPyServer> v_py_server;

    while(!v_rvm.empty()){
        //每种服务器选一个，装满一箱，看看哪个服务器的利用率最大
        unsigned int val = 0;
        double max_rate = 0.0;
        for(unsigned int i = 0; i < m_py_server.size(); i++){
            //计算此种服务器的利用率
            double rate = 0.0;
            PackOneBox(v_rvm, m_py_server[i], rate);
            if(rate >= max_rate){
                max_rate = rate;
                val = i;
            }
        }

        //使用服务器利用率最大的进行装箱
        vector<PredictVM> v_pvm = PackOneBox(v_rvm, m_py_server[val], max_rate, true);

        /*将此虚拟机移动到服务器中*/
        //判断此类型的服务器是否存在
        if(!v_pvm.empty()){
            vector<VMOfPyServer>::iterator it = v_py_server.begin();
            for(it = v_py_server.begin(); it != v_py_server.end(); it++){
                if(it->py_server_name == m_py_server[val].name){
                    break;
                }
            }

            if(it == v_py_server.end()){
                //没有此类型的服务器
                VMOfPyServer vmps;
                vmps.py_server_name = m_py_server[val].name;
                //vmps.py_server_num 需要稍后计算
                vmps.onekind_py_server.push_back(v_pvm);
                v_py_server.push_back(vmps);
            }
            else{
                //原来已经有此类型的服务器
                it->onekind_py_server.push_back(v_pvm);
            }
        }
    }
    //对v_py_server进行整合
    IntegratedServer(v_py_server);
    return v_py_server;
}

void DevCloud::IntegratedServer(vector<VMOfPyServer>& v_py_server){
    for(vector<VMOfPyServer>::iterator it = v_py_server.begin(); it != v_py_server.end(); it++){
        //首先确定数量
        it->py_server_num = it->onekind_py_server.size();
        //将里面的内容进行合并
        for(unsigned int i = 0; i < it->onekind_py_server.size(); i++){
            it->onekind_py_server[i] = IntegratedOneServer(it->onekind_py_server[i]);
        }
    }
}


vector<PredictVM> DevCloud::IntegratedOneServer(vector<PredictVM>& v_pvm){
    map<string,int> mp = StatisticsVM(v_pvm);
    vector<PredictVM> result;

    for(map<string,int>::iterator it = mp.begin(); it != mp.end(); it++){
        PredictVM vm;
        vm.vm_name = it->first;
        vm.vm_num = it->second;
        result.push_back(vm);
    }

    return result;
}

map<string,int> DevCloud::StatisticsVM(vector<PredictVM>& v_pvm){
    map<string,int> mp;
    for(unsigned int i = 0; i < v_pvm.size(); i++){
        mp[v_pvm[i].vm_name] += v_pvm[i].vm_num;
    }
    return mp;
}


vector<PredictVM> DevCloud::PackOneBox(vector<RemainderVM>& v_rvm, PhysicalServer ps, double& rate, bool delete_vm){
    //构建一个行为v_rvm个数，列为物理服务器CPU数+1的数组
    vector<vector<PhysicalServer> > dp;
    for(unsigned int i = 0; i < v_rvm.size(); i++){
        vector<PhysicalServer> temp(ps.core_num+1,ps);
        dp.push_back(temp);
    }
    //开始规划
    dp[0][ps.core_num - v_rvm[0].core_num] = ps;
    dp[0][ps.core_num - v_rvm[0].core_num].core_num -= v_rvm[0].core_num;
    dp[0][ps.core_num - v_rvm[0].core_num].mem_size -= v_rvm[0].mem_size;

    unsigned int i = 1;
    int j = ps.core_num;
    for(i = 1; i < v_rvm.size(); i++){
        for(j = ps.core_num; j >= 0; j--){
            PhysicalServer ps_remainder;
            ps_remainder = dp[i-1][j];
            //尤其注意界限问题，同时还要考虑两个维度，暂时以CPU为主要优化
            if(j + v_rvm[i].core_num <= ps.core_num){
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
    for(j = 0; j <= ps.core_num; j++){
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
            if(true == delete_vm){
                vector<RemainderVM>::iterator iter = v_rvm.begin() + i;
                v_rvm.erase(iter);
            }
            i--;
        }
    }
    //需要判断i == 0的虚拟机时候用上，
    if(dp[0][j].core_num == j && j != ps.core_num){
        //将此虚拟机压入一台服务器中
        PredictVM pvm;
        pvm.vm_name = v_rvm[i].p_vm.vm_name;
        pvm.vm_num = 1;
        vpvm.push_back(pvm);
        //将此虚拟机删除
        if(true == delete_vm){
            vector<RemainderVM>::iterator iter = v_rvm.begin() + i;
            v_rvm.erase(iter);
        }
    }

    /*计算此利用率*/
    //根据vpvm计算利用率
    int all_cpu = 0;
    int all_mem = 0;
    for(unsigned int i = 0; i < vpvm.size(); i++){
        all_cpu += m_map_vm[vpvm[i].vm_name].core_num * vpvm[i].vm_num;
        all_mem += m_map_vm[vpvm[i].vm_name].mem_size * vpvm[i].vm_num;
    }
    
    rate = (all_cpu / (ps.core_num * 1.0) + all_mem / (ps.mem_size * 1.0)) / 2.0;

    return vpvm;
}


double DevCloud::ComputeUtilizationRate(vector<VMOfPyServer>& v_py_server){
    //计算预测的虚拟机所占用的资源
    int all_cpu_of_vm = 0;
    int all_mem_of_vm = 0;

    for(unsigned int i = 0; i < m_predict_vm.size(); i++){
        //根据名称，查找对应的CPU 和 MEM 的大小
        int cpu = m_map_vm[m_predict_vm[i].vm_name].core_num;
        int mem = m_map_vm[m_predict_vm[i].vm_name].mem_size;

        //计算总计的用量
        all_cpu_of_vm += cpu * m_predict_vm[i].vm_num;
        all_mem_of_vm += mem * m_predict_vm[i].vm_num;
    }

    //计算所用的服务器总计的资源
    int all_cpu_of_py = 0;
    int all_mem_of_py = 0;

    for(unsigned int i = 0; i < v_py_server.size(); i++){
        //根据名称，查找对应的服务器的CPU和MEM的大小
        int cpu = m_map_py[v_py_server[i].py_server_name].core_num;
        int mem = m_map_py[v_py_server[i].py_server_name].mem_size;

        //计算总计的量
        all_cpu_of_py += cpu * v_py_server[i].py_server_num;
        all_mem_of_py += mem * v_py_server[i].py_server_num;
    }

    //如果除数为0则，返回0、
    if(all_cpu_of_py == 0 || all_mem_of_py == 0){
        return 0.0;
    }

    //计算最终的比例
    double result = 0.0;
    result = all_cpu_of_vm / (all_cpu_of_py * 1.0) + all_mem_of_vm / (all_mem_of_py * 1.0);
    result /= 2.0;

    return result;
}

//提高利用率
void DevCloud::ImproveUtilization(){
    for(unsigned int i = 0; i < m_predict_py_server.size(); i++){
        //对此类型的服务器进行调整
        bool flag = ProcessOnePY(m_predict_py_server[i],i);
        if(flag == false){
            i--;
        }
    }
}

bool DevCloud::ProcessOnePY(VMOfPyServer& one_kind_py, int val){
    //需要检查每一个服务器的利用率，进行提升
    double utilization_core = 0.0;
    double utilization_mem = 0.0;

    for(unsigned int i = 0; i < one_kind_py.onekind_py_server.size(); i++){
        int core_num = 0;
        int mem_num = 0;

        for(unsigned int j = 0; j < one_kind_py.onekind_py_server[i].size(); j++){
            core_num += one_kind_py.onekind_py_server[i][j].vm_num * m_map_vm[one_kind_py.onekind_py_server[i][j].vm_name].core_num;
            mem_num += one_kind_py.onekind_py_server[i][j].vm_num * m_map_vm[one_kind_py.onekind_py_server[i][j].vm_name].mem_size;
        }

        //计算利用率
        utilization_core = core_num / (m_map_py[one_kind_py.py_server_name].core_num * 1.0);
        utilization_mem = mem_num / (m_map_py[one_kind_py.py_server_name].mem_size * 1.0);

        //如果有一种利用率小于60%，直接删除，否则填满
        if(utilization_core < 0.6 || utilization_mem < 0.6){
            bool flag = DeleteVM(i, one_kind_py.onekind_py_server[i], val); 
            if(flag == false){
                return false;
            }
            i--;
        }
        else{
            FillVM(i, one_kind_py.onekind_py_server[i], one_kind_py.py_server_name);
        }
    }
    return true;
}


void DevCloud::FillVM(int num, vector<PredictVM>& v_pvm, string pyname){
    //计算剩余空间
    int rem_core = m_map_py[pyname].core_num;
    int rem_mem = m_map_py[pyname].mem_size;
    for(unsigned int i = 0; i < v_pvm.size(); i++){
        rem_core -= v_pvm[i].vm_num * m_map_vm[v_pvm[i].vm_name].core_num;
        rem_mem -= v_pvm[i].vm_num * m_map_vm[v_pvm[i].vm_name].mem_size;
    }
    //开始填充
    for(unsigned int i = 0; i < v_pvm.size(); i++){
        //计算可以填充此类型的虚拟机几个
        int core_num = rem_core / m_map_vm[v_pvm[i].vm_name].core_num;
        int mem_num = rem_mem / m_map_vm[v_pvm[i].vm_name].mem_size;
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
        rem_core -= max_num * m_map_vm[v_pvm[i].vm_name].core_num;
        rem_mem -= max_num * m_map_vm[v_pvm[i].vm_name].mem_size;
    }
}

//返回false，表示删除了该类型服务器；否则，没有删除该类型的服务器
bool DevCloud::DeleteVM(int num, vector<PredictVM>& v_pvm, int val){
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
    vector<vector<PredictVM> >::iterator it = m_predict_py_server[val].onekind_py_server.begin() + num;
    m_predict_py_server[val].onekind_py_server.erase(it);
    m_predict_py_server[val].py_server_num--;

    //如果该类型服务器数量为0,则将该服务器删除
    if(0 == m_predict_py_server[val].py_server_num){
        vector<VMOfPyServer>::iterator iter = m_predict_py_server.begin() + val;
        m_predict_py_server.erase(iter);
        return false;
    }
    return true;
}


void DevCloud::LastPredictDay(){
    //对输入的每一种类型的虚拟机进行预测
    PredictVM p_vm;
    for(unsigned int i = 0; i < m_vm_kind.size(); i++){
        p_vm = PredictOneKindVM_LastPredictDay(m_vm_kind[i].vm_name);
        if(p_vm.vm_num > 0){
            //对预测结果放大1.2倍
            p_vm.vm_num = (int)(p_vm.vm_num * 2 + 0.5);
            // p_vm.vm_num += 50;
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
    int begin_day = m_max_day - m_predict_day + 1;
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


void DevCloud::ExponentialSmooth(){
    //对输入的每一种类型的虚拟机进行预测
    PredictVM p_vm;
    for(unsigned int i = 0; i < m_vm_kind.size(); i++){
        p_vm = PredictOneKindVM_ES(m_vm_kind[i].vm_name);
        if(p_vm.vm_num > 0){
            //对预测结果放大1.2倍
            // p_vm.vm_num = (int)(p_vm.vm_num * 1.2 + 0.5);
            // p_vm.vm_num = (int)(p_vm.vm_num / 1.3 + 0.5);
            // p_vm.vm_num += 40;
            m_predict_vm.push_back(p_vm);
            //将虚拟机总数累加
            m_predict_vm_num += p_vm.vm_num;
        }
    }
}



// PredictVM DevCloud::PredictOneKindVM_ES(string flavor){
//     //统计此种类型的虚拟机
//     map<int,int> vm = StatisticsOneKindVM(flavor);
//     //计算可以划分出来几个数据
//     int total_num = (m_max_day - m_min_day + 1) / m_predict_day;
//     //总的天数
//     int total_day = total_num * m_predict_day;
//     //开始的天数
//     int begin_day = m_max_day - total_day + 1;

//     //将每一天提取出来
//     vector<int> one_day(total_day, 0);

//     for(map<int,int>::iterator it = vm.begin(); it != vm.end(); it++){
//         if(it->first >= begin_day){
//             one_day[it->first - begin_day] = it->second;
//         }
//     }

//     //将one_day逆序
//     reverse(one_day.begin(), one_day.end());

//     //对total_num个数据进行统计
//     vector<int> n_total_num(total_num, 0);

//     for(int i = 0; i < total_num; i++){
//         for(int j = 0; j < m_predict_day; j++){
//             n_total_num[i] += one_day[i * m_predict_day + j];
//         }
//     }

//     //方法一：
//     //对n_total_num的值进行处理
//     //求n_total_num的平均值
//     double mean = accumulate(n_total_num.begin(), n_total_num.end(), 0) / (n_total_num.size() * 1.0);
//     PredictVM pvm;
//     pvm.vm_name = flavor;
//     pvm.vm_num = (int)(0.8 * mean + 0.6 * n_total_num[0] + 0.5);

//     return pvm;

//     //方法二：
//     // reverse(n_total_num.begin(), n_total_num.end());
//     // //开始滑窗
//     // double alpha = 0.6;
//     // double init = 0.0;
//     // if(n_total_num.size() >= 3){
//     //     init = (n_total_num[0] + n_total_num[1] + n_total_num[2]) /3.0;
//     // }
//     // else{
//     //     init = n_total_num[0];
//     // }

//     // vector<double> esmo(n_total_num.size(), init);
//     // for(unsigned int i = 1; i < n_total_num.size(); i++){
//     //     esmo[i] = alpha * n_total_num[i] + (1 - alpha) * esmo[i -1];
//     // }

//     // //预测新的
//     // double predict_num = alpha * n_total_num[n_total_num.size() -1] + (1 - alpha) * esmo[esmo.size() - 1] + 0.5;
//     // PredictVM pvm;
//     // pvm.vm_name = flavor;
//     // pvm.vm_num = (int)(predict_num);
//     // return pvm;
// }

PredictVM DevCloud::PredictOneKindVM_ES(string flavor){
    //将信息补充完全
    //将此种类型的虚拟机统计出来
    map<int,int> vm = StatisticsOneKindVM(flavor);
    vector<int> one_day(m_max_day - m_min_day + 1, 0);
    // vector<int> one_day(StringToDatetime(m_predict_begin_time) - m_min_day, 0);

    for(map<int,int>::iterator it = vm.begin(); it != vm.end(); it++){
        one_day[it->first - m_min_day] = it->second;
    }

    //计算总的个数
    int total_val = m_max_day - m_min_day + 1;
    total_val -= m_predict_day;
    total_val += 1;

    vector<int> sum_day(total_val, 0);

    for(int i = 0; i < total_val; i++){
        for(int j = 0; j < m_predict_day; j++){
            sum_day[i] += one_day[i + j];
        }
    }

    /****************************************
     * 一次滑窗，a从[0,1:0.1]，取使MSE最小的a
    ****************************************/
    //使用最优的opt_a进行一次滑窗
    //计算初值
    double initial = (sum_day[0] + sum_day[1] + sum_day[2]) / 3.0;
    //  double initial = (sum_day[0] + sum_day[1]) / 3.0;
    //初始化a为一组向量，选择MSE最小的
    vector<double> a;
    for(int i = 1; i < 10; i++){
        a.push_back(i / 10.0);
    }
    double opt_a = 0.0;
    // double opt_mse = 0.0;
    //开始一次滑动
    // for(unsigned int i = 0; i < a.size(); i++){
    //     vector<double> OneTimeSmooth(sum_day.size(),0.0);
    //     double mse = 0.0;
    //     //计算mse
    //     for(unsigned int j = 0; j < sum_day.size(); j++){
    //         if(j == 0){
    //             OneTimeSmooth[j] = initial;
    //         }
    //         else{
    //             OneTimeSmooth[j] = opt_a * sum_day[j-1] + (1 - opt_a) * OneTimeSmooth[j-1];
    //         }
    //         mse += (sum_day[j] - OneTimeSmooth[j]) * (sum_day[j] - OneTimeSmooth[j]);
    //     }

    //     if(i == 0){
    //         opt_mse = mse;
    //         opt_a = a[i];
    //     }
    //     else{
    //         if(mse < opt_mse){
    //             opt_mse = mse;
    //             opt_a = a[i];
    //         }
    //     }
    // }
    //使用最优的opt_a进行一次滑窗
    opt_a = 0.6;
    vector<double> OneTimeSmooth(sum_day.size(),0.0);
    for(unsigned int j = 0; j < sum_day.size(); j++){
        if(j == 0){
            OneTimeSmooth[j] = initial;
        }
        else{
            // OneTimeSmooth[j] = opt_a * sum_day[j-1] + (1 - opt_a) * OneTimeSmooth[j-1];
            OneTimeSmooth[j] = opt_a *sum_day[j-1] + (1 - opt_a) * OneTimeSmooth[j-1];
        }
    }


    //对最后的值进行预测
    double total_day = 0.0;
    // total_day = opt_a * sum_day[sum_day.size()-1] + (1 - opt_a) * OneTimeSmooth[sum_day.size()-1];
    // total_day = sum_day[sum_day.size()-1] + (1 - opt_a) * OneTimeSmooth[sum_day.size()-1];
    // double mean = accumulate(sum_day.begin(), sum_day.end(), 0) / (sum_day.size() * 1.0);
    // total_day = 0.2 * mean + 0.8 * sum_day[sum_day.size() - 1];

    // PredictVM pvm;
    // pvm.vm_name = flavor;
    // pvm.vm_num = (int)(total_day + 0.5);

    for(int i = 0; i < m_predict_day; i++){
        double mean = accumulate(sum_day.begin(), sum_day.end(), 0) / (sum_day.size() * 1.0);
        int day = (int)(0.8 * mean + 0.6 * sum_day[sum_day.size() -1] + 0.5);
        sum_day.push_back(day);        
    }


    PredictVM pvm;
    pvm.vm_name = flavor;
    // pvm.vm_num = sum_day[sum_day.size()-1];
    pvm.vm_num = (int)accumulate(sum_day.begin() + total_val, sum_day.end(), 0) / (m_predict_day * 1.0);
    // pvm.vm_num = sum_day[sum_day.size()-1];

    return pvm;
}

void DevCloud::LinearRegression(){
    //将所有虚拟机放在一起训练出来一组参数
    vector<double> w = TrainOnline();
    // vector<double> w(3,0);

    //对输入的每一种类型的虚拟机进行预测
    PredictVM p_vm;
    for(unsigned int i = 0; i < m_vm_kind.size(); i++){
        p_vm = PredictOneKindVM_LR(m_vm_kind[i].vm_name, w);
        if(p_vm.vm_num > 0){
            //对预测结果放大1.2倍
            // p_vm.vm_num = (int)(p_vm.vm_num * 1.2 + 0.5);
            // p_vm.vm_num = (int)(p_vm.vm_num / 1.3 + 0.5);
            // if(i == 0){
            //     p_vm.vm_num = (int)(p_vm.vm_num * 1.2 + 0.5);
            // }
            // else{
            //     // p_vm.vm_num += 30;
            //     p_vm.vm_num = (int)(p_vm.vm_num * 2 + 0.5);
            // }
            
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
    vector<int> one_day(m_max_day - m_min_day + 1, 0);
    // vector<int> one_day(StringToDatetime(m_predict_begin_time) - m_min_day, 0);

    for(map<int,int>::iterator it = vm.begin(); it != vm.end(); it++){
        one_day[it->first - m_min_day] = it->second;
    }
    //根据需要预测的天数进行滑窗
    int last_day = m_max_day - m_min_day + 1 - m_predict_day;
    // int last_day = StringToDatetime(m_predict_begin_time) - m_min_day - m_predict_day;
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

    // double lambda = 1000;

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

vector<double> DevCloud::TrainOnline(){
    //划分df
    vector<vector<int> > df;
    for(unsigned int i = 0; i < m_vm_kind.size(); i++){
        map<int,int> vm = StatisticsOneKindVM(m_vm_kind[i].vm_name);
        // vector<int> one_day(StringToDatetime(m_predict_begin_time) - m_min_day, 0);
        vector<int> one_day(m_max_day + 1 - m_min_day, 0);
        for(map<int,int>::iterator it = vm.begin(); it != vm.end(); it++){
            one_day[it->first - m_min_day] = it->second;
        }
        // int last_day = StringToDatetime(m_predict_begin_time) - m_min_day - m_predict_day;
        int last_day = m_max_day + 1 - m_min_day - m_predict_day;
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
    int m = 20;
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
