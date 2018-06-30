#ifndef __DEVCLOUD_H__  
#define __DEVCLOUD_H__  

#include<stdio.h>
#include<iostream>
#include<string>
#include<vector>
#include<ctime>
#include<map>
#include<algorithm> 
#include<sstream>
#include<numeric> 
using namespace std;

//物理服务器
struct PhysicalServer
{
    PhysicalServer(){name = ""; core_num = 0; mem_size = 0; hd_size =0;}
    string name;        //物理服务器类型名称  
    int core_num;       //物理服务器的CPU核心数
    int mem_size;       //物理服务器的内存大小
    int hd_size;        //物理服务器的硬盘大小
}; 

//虚拟机
struct VirtualMachine
{
    VirtualMachine(){vm_name = ""; core_num = 0; mem_size = 0;}
    string vm_name;     //虚拟机规格名称
    int core_num;       //虚拟机的CPU核心数
    int mem_size;       //虚拟机的内存大小
};

//训练集的一条数据
struct TrainDf
{
    TrainDf(){vm_name = ""; r_time = ""; i_time = 0;}
    string vm_name;     //训练集一条数据的虚拟机规格名称
    string r_time;      //训练集一条数据的虚拟机请求的时间
    int i_time;         //将数据转化为对应的整数，以天为单位
};

//预测的虚拟机的情况
struct PredictVM
{
    PredictVM(){vm_name = ""; vm_num = 0;}
    string vm_name;     //预测的虚拟机规格名称
    int vm_num;         //对应的虚拟机个数
};

//预测需要摆放的虚拟机，剩余的量
struct RemainderVM{
    RemainderVM(){core_num = 0; mem_size = 0;}
    PredictVM p_vm;
    int core_num;       //虚拟机的CPU核心数
    int mem_size;       //虚拟机的内存大小
};

//输出文件的定义
struct VMOfPyServer{
    VMOfPyServer(){py_server_name = ""; py_server_num = 0; onekind_py_server.clear();}
    string py_server_name;
    int py_server_num;
    vector<vector<PredictVM> > onekind_py_server;       //预测需要的物理服务器的分配情况
};



time_t StringToDatetime(string str);
vector<double> Train(vector<vector<int> > X, vector<int> y);
double ComputeLoss(vector<double> &w, vector<vector<int> > &X, vector<int> &y);
void GradientDescent(vector<double> &w,vector<vector<int> > &X, vector<int> &y, double learn_rate = 0.01);

//华为云服务器
class DevCloud
{
public:
    //构造函数
    DevCloud(){
        //输入数据相关初始化
        m_py_server_num = 0;
        m_py_server.clear();
        m_vm_kind_num = 0;
        m_vm_kind.clear();
        m_predict_begin_time = "";
        m_predict_end_time = "";
        m_predict_day = 0;

        //训练数据集
        m_train_df_num = 0;
        m_train_df.clear();
        m_min_day = 0;
        m_max_day = 0;

        //输出数据相关定义
        m_predict_vm_num = 0;
        m_predict_vm.clear();
        m_predict_py_server.clear();

        //索引初始化
        m_map_vm.clear();
        m_map_py.clear();
    }                                           

    //输入输出函数
    void ReadInput(char *info[]);                         //读入输入文件
    void ReadTrain(char *data[],int data_num);            //读入训练数据
    void Transform();                                     //一个临时的转化文件，便于数据跑通
    string OutPut();                                      //将预测结果转为char *格式输出

    //预测函数
    void GuiZe();                                         //使用时间序列的规则方法进行预测
    vector<int> FindTrain(map<int,int> vm, int total_week);               //划分出训练集
    PredictVM PredictOneKindVM_GuiZe(string flavor, int total_week);
    vector<int> PrecessOutlier(vector<int> train);

    //使用最后等量的天数去预测
    void LastPredictDay();                                //以最后几天的和为预测值
    PredictVM PredictOneKindVM_LastPredictDay(string flavor); 

    //使用指数平滑去预测
    void ExponentialSmooth();
    PredictVM PredictOneKindVM_ES(string flavor);

    //使用线性回归进行预测
    void LinearRegression();
    PredictVM PredictOneKindVM_LR(string flavor, vector<double> w);
    vector<double> TrainOnline();

    //摆放函数
    /*第一种方案，将以前的动态规划照搬过来，只使用其中的一种进行摆放*/
    bool CheckVMMEMSize();
	void DongTai();
    //优先考虑CPU进行优化
    vector<vector<PredictVM> > OptimizeByCPU(vector<RemainderVM>& v_rvm, PhysicalServer ps);
    void TransferServer(vector<vector<PredictVM> > py_server, PhysicalServer ps);
    /*第二种方案，采用贪心算法，一次只最优化一个服务器*/
    void Greedy();
    vector<VMOfPyServer> GreedySelect(vector<RemainderVM>& v_rvm);
    void IntegratedServer(vector<VMOfPyServer>& v_py_server);
    vector<PredictVM> IntegratedOneServer(vector<PredictVM>& v_pvm);
    map<string,int> StatisticsVM(vector<PredictVM>& v_pvm);

    //检查结果，如果预测的为0，则添加进来
    void CheckResult();

    //建立虚拟机索引
    void VMToMap();                                       //将需要处理的虚拟机建立索引
    void PYToMap();                                       //将需要处理的服务器建立索引

    //提高装箱利用率
    void ImproveUtilization();
    bool ProcessOnePY(VMOfPyServer& one_kind_py, int val);
    void FillVM(int num, vector<PredictVM>& v_pvm, string pyname);       //填充新的虚拟机，到最大
    bool DeleteVM(int num, vector<PredictVM>& v_pvm, int val);           //利用率太低的直接删除

    //检查一下最后的利用率
    void CheckUnit();

private:
    map<int,int> StatisticsOneKindVM(string flavor);      //统计一种虚拟机训练集的情况
    vector<RemainderVM> Unpacking();                      //对预测的虚拟机进行拆箱
    //计算资源利用率
    double ComputeUtilizationRate(vector<VMOfPyServer>& v_py_server);
    vector<PredictVM> PackOneBox(vector<RemainderVM>& v_rvm, PhysicalServer ps, double& rate, bool delete_vm = false);

private:
    //输入数据相关的定义
    int m_py_server_num;                                  //物理服务器类型数量
    vector<PhysicalServer> m_py_server;                   //物理服务器的属性
    int m_vm_kind_num;                                    //虚拟机规格数量
    vector<VirtualMachine> m_vm_kind;                     //虚拟机规格
    string m_predict_begin_time;                          //预测的开始时间
    string m_predict_end_time;                            //预测的结束时间 
    int m_predict_day;                                    //需要预测的天数，以天为单位


    //训练数据集
    int m_train_df_num;                                   //训练数据集的条数
    vector<TrainDf> m_train_df;                           //训练数据集
    int m_min_day;                                        //训练集中最小的天数
    int m_max_day;                                        //训练集中最大的天数

    //输出数据相关定义
    int m_predict_vm_num;                                 //预测虚拟机总数
    vector<PredictVM> m_predict_vm;                       //预测虚拟机的情况
    vector<VMOfPyServer> m_predict_py_server;             //预测需要的物理服务器的分配情况

    //建立一份索引
    map<string,VirtualMachine> m_map_vm;
    map<string,PhysicalServer> m_map_py;
};



#endif 