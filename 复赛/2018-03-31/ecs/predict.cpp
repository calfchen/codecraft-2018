#include "predict.h"
#include "devcloud.h"
#include <stdio.h>

//��Ҫ��ɵĹ��������
void predict_server(char * info[MAX_INFO_NUM], char * data[MAX_DATA_NUM], int data_num, char * filename)
{
	DevCloud devCloud;
	devCloud.ReadInput(info);
	devCloud.ReadTrain(data,data_num);

	/*Ԥ�ⲿ��*/
	// devCloud.Transform();
	// devCloud.PredictAllVM();
	// devCloud.LastPredictDay();
	// devCloud.GuiZe();
	// devCloud.MaxResourcePrior();
	// devCloud.ClassificationPredict();
	// devCloud.Merge();
	devCloud.SVRPredict();

	/*װ�䲿��*/
	// devCloud.FirstFit();
	devCloud.BestFit();
	// devCloud.OneServerToVM();

	/*���װ��������*/
	// devCloud.CheckUtilization();

	/*���һ��������*/
	// devCloud.ImproveUtilization();

	// ��Ҫ���������
	string result = devCloud.OutPut();
	char * result_file = (char *)result.c_str();

	// ֱ�ӵ�������ļ��ķ��������ָ���ļ���(ps��ע���ʽ����ȷ�ԣ�����н⣬��һ��ֻ��һ�����ݣ��ڶ���Ϊ�գ������п�ʼ���Ǿ�������ݣ�����֮����һ���ո�ָ���)
	write_result(result_file, filename);
}
