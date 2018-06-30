#ifndef __DEVCLOUD_H__  
#define __DEVCLOUD_H__  

#include<stdio.h>
#include<iostream>
#include<string>
#include<vector>
#include<ctime>
#include<map>
#include<algorithm> 
using namespace std;

//物理服务器
struct PhysicalServer
{  
    int core_num;       //物理服务器的CPU核心数
    int mem_size;       //物理服务器的内存大小
    int hd_size;        //物理服务器的硬盘大小
}; 

//虚拟机
struct VirtualMachine
{
    string vm_name;     //虚拟机规格名称
    int core_num;       //虚拟机的CPU核心数
    int mem_size;       //虚拟机的内存大小
};

//训练集的一条数据
struct TrainDf
{
    string vm_name;     //训练集一条数据的虚拟机规格名称
    string r_time;      //训练集一条数据的虚拟机请求的时间
    int i_time;         //将数据转化为对应的整数，以天为单位
};

//预测的虚拟机的情况
struct PredictVM
{
    PredictVM(){
        vm_name = "";
        vm_num = 0;
    }
    string vm_name;     //预测的虚拟机规格名称
    int vm_num;         //对应的虚拟机个数
};

//一种规格的虚拟机，请求的时间，以及次数
struct VMRequest
{
    int day;            //虚拟机请求的时间，距离1900年的天数
    int count;          //该天，此种类型的虚拟机总共请求的次数
};

//预测需要摆放的虚拟机，剩余的量
struct RemainderVM{
    PredictVM p_vm;
    int core_num;       //虚拟机的CPU核心数
    int mem_size;       //虚拟机的内存大小
};


time_t StringToDatetime(string str);
bool SortVMByCPU( const RemainderVM &v1, const RemainderVM &v2);
bool SortVMByMEM( const RemainderVM &v1, const RemainderVM &v2);
vector<double> Train(vector<vector<int> > X, vector<int> y);
double ComputeLoss(vector<double> &w, vector<vector<int> > &X, vector<int> &y);
void GradientDescent(vector<double> &w,vector<vector<int> > &X, vector<int> &y, double learn_rate = 0.01);

//华为云服务器
class DevCloud
{
public:
    //构造函数
    DevCloud(){
        m_vm_kind_num = 0;
        m_vm_kind.clear();
        m_optimize_resource = "";

        m_train_df_num = 0;

        m_predict_vm_num = 0;
        m_predict_vm.clear();
        m_predict_py_server_num = 0;
        m_predict_py_server.size();
    }                                           

    void ReadInput(char *info[]);                         //读入输入文件
    void ReadTrain(char *data[],int data_num);            //读入训练数据
    void Transform();                                     //一个临时的转化文件，便于数据跑通
    string OutPut();                                      //将预测结果转为char *格式输出
    void PredictAllVM();                                  //将预测需要的虚拟机情况保存起来
    void MaxResourcePrior();                              //按需要优化的资源优先
    void OneServerToVM();                                 //一个服务器对应一个虚拟机
    void FirstFit();                                      //firstFit方法
    vector<RemainderVM> PredictToRemainder();             //将预测出来的虚拟机转化为剩余量

    /*开始尝试新的装箱方法*/
    void BestFit();
    vector<RemainderVM> Unpacking();
    void TransferServer(vector<vector<PredictVM> > py_server);

    /*开始构造新的算法进行预测*/
    void GuiZe();                                         //使用时间序列的规则方法进行预测
    // vector<int> FindTest(map<int,int> vm, int num = 7);   //划分出测试集,默认7天，也只定义7天
    vector<int> FindTrain(map<int,int> vm);               //划分出训练集
    PredictVM PredictOneKindVM_GuiZe(string flavor);
    vector<int> PrecessOutlier(vector<int> train);

    /*尝试先分类别，再进行预测的方法*/
    void ClassificationPredict();
    int StatisticsOneKindLastThreeWeekVM(string flavor);

    /*开始尝试融合的方法*/
    void Merge();

    /*检查一下装箱利用率*/
    void CheckUtilization();

    /*新的方法*/
    void LastPredictDay();                                //以最后几天的和为预测值
    PredictVM PredictOneKindVM_LastPredictDay(string flavor); 

    /*后操作，如果装箱的利用率不超过50%，则提高此箱子的总的比例*/
    void ImproveUtilization();
    void FillVM(int num, vector<PredictVM>& v_pvm);        //填充新的虚拟机，到最大
    void DeleteVM(int num, vector<PredictVM>& v_pvm);      //利用率太低的直接删除

    //使用动态规划来求解问题
    //检查一下虚拟机的内存是否为1024的整数倍，如果是则返回真
    bool CheckVMMEMSize(); 
    void DongTai();                                       //使用动态规划来求解   
    //优先考虑CPU进行优化
    vector<vector<PredictVM> > OptimizeByCPU(vector<RemainderVM>& v_rvm);
    vector<vector<PredictVM> > OptimizeByMEM(vector<RemainderVM>& v_rvm);  

    //使用线性回归进行预测
    void LinearRegression();
    PredictVM PredictOneKindVM_LR(string flavor, vector<double> w);
    vector<double> TrainOnline();


    //检查结果，如果预测的为0，则添加进来
    void CheckResult();

    void BIU();

private:
    PredictVM PredictOneKindVM(string flavor);            //对其中一种规格的虚拟机进行预测
    map<int,int> StatisticsOneKindVM(string flavor);      //统计一种虚拟机训练集的情况
    double Mean(map<int,int> vm);                         //计算出均值
    map<string,VirtualMachine> VMToMap();                 //将需要处理的虚拟机建立索引
    

private:
    //输入数据相关的定义
    PhysicalServer m_py_server;                           //物理服务器的属性
    int m_vm_kind_num;                                    //虚拟机规格数量
    vector<VirtualMachine> m_vm_kind;                     //虚拟机规格
    string m_optimize_resource;                           //需要优化的资源
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
    int m_predict_py_server_num;                          //预测需要的物理服务器总数
    vector<vector<PredictVM> > m_predict_py_server;       //预测需要的物理服务器的分配情况
};



#endif 