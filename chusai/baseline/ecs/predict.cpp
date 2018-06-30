#include "predict.h"
#include "devcloud.h"
#include <stdio.h>

//你要完成的功能总入口
void predict_server(char * info[MAX_INFO_NUM], char * data[MAX_DATA_NUM], int data_num, char * filename)
{
	DevCloud devCloud;
	devCloud.ReadInput(info);
	devCloud.ReadTrain(data,data_num);

	/*预测部分*/
	// devCloud.Transform();
	// devCloud.PredictAllVM();
	// devCloud.LastPredictDay();
	devCloud.GuiZe();
	// devCloud.ClassificationPredict();
	// devCloud.Merge();
	// devCloud.LinearRegression();
	// devCloud.ExponentialSmooth();

	/*装箱部分*/
	// devCloud.OneServerToVM();
	// devCloud.MaxResourcePrior();
	// devCloud.FirstFit();
	// devCloud.BestFit();
	devCloud.DongTai();

	/*检查装箱利用率*/
	// devCloud.CheckUtilization();

	/*提高一下利用率*/
	devCloud.ImproveUtilization();

	//检查结果
	devCloud.CheckResult();

	// 需要输出的内容
	string result = devCloud.OutPut();
	char * result_file = (char *)result.c_str();

	// 直接调用输出文件的方法输出到指定文件中(ps请注意格式的正确性，如果有解，第一行只有一个数据；第二行为空；第三行开始才是具体的数据，数据之间用一个空格分隔开)
	write_result(result_file, filename);
}
