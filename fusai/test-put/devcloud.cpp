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
vector<int> DevCloud::FindTrain(map<int,int> vm){
    int begin_num = 0, end_num = 21;
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
    //最后几天的平均值
    int last_day = 7;                       
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

    //预测的开始天可能不连续
    int separate_day = StringToDatetime(m_predict_begin_time) - m_max_day - 1;

    for(unsigned int i = 0; i < predict_day.size(); i++){
        // predict_day[i] = (int)(base * factor_median[(i+separate_day) % 7] + 0.5);
        // predict_day[i] = (int)(base * factor_mean[(i+separate_day) % 7] + 0.5);
        predict_day[i] = (int)(base * (factor_mean[(i+separate_day) % 7] * 0.4 + factor_median[(i+separate_day) % 7] * 0.6)+ 0.5);
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

    for(int i = 0; i < 500; i++){
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

    for(int i = 0; i < 50; i++){
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

// flavor4 255
// flavor6 213
// flavor9 248
// flavor12 232
// flavor13 76
// flavor14 281
// flavor15 210
// flavor17 235
// flavor18 93

void DevCloud::LeiXing(){
    PredictVM vm;
    vm.vm_name = "flavor4";
    vm.vm_num = 255;
    m_predict_vm_num += vm.vm_num;
    m_predict_vm.push_back(vm);

    vm.vm_name = "flavor6";
    vm.vm_num = 213;
    m_predict_vm_num += vm.vm_num;
    m_predict_vm.push_back(vm);

    vm.vm_name = "flavor9";
    vm.vm_num = 248;
    m_predict_vm_num += vm.vm_num;
    m_predict_vm.push_back(vm);

    vm.vm_name = "flavor12";
    vm.vm_num = 232;
    m_predict_vm_num += vm.vm_num;
    m_predict_vm.push_back(vm);

    vm.vm_name = "flavor13";
    vm.vm_num = 76;
    m_predict_vm_num += vm.vm_num;
    m_predict_vm.push_back(vm);

    vm.vm_name = "flavor14";
    vm.vm_num = 281;
    m_predict_vm_num += vm.vm_num;
    m_predict_vm.push_back(vm);

    vm.vm_name = "flavor15";
    vm.vm_num = 210;
    m_predict_vm_num += vm.vm_num;
    m_predict_vm.push_back(vm);

    vm.vm_name = "flavor17";
    vm.vm_num = 235;
    m_predict_vm_num += vm.vm_num;
    m_predict_vm.push_back(vm);

    vm.vm_name = "flavor18";
    vm.vm_num = 93;
    m_predict_vm_num += vm.vm_num;
    m_predict_vm.push_back(vm);
}