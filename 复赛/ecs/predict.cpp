#include "predict.h"
#include "devcloud.h"
#include <stdio.h>

//��Ҫ��ɵĹ��������
void predict_server(char * info[MAX_INFO_NUM], char * data[MAX_DATA_NUM], int data_num, char * filename)
{
	//����һ������
	DevCloud devCloud;
	//�������룬ѵ���ļ�
	devCloud.ReadInput(info);
	devCloud.ReadTrain(data,data_num);

	//���������������
	// devCloud.Transform();

	//��ʼ������
	devCloud.VMToMap();
	devCloud.PYToMap();

	//Ԥ��
	// devCloud.GuiZe();
	// devCloud.LastPredictDay();
	devCloud.ExponentialSmooth();
	// devCloud.LinearRegression();

	//�ڷ�
	// devCloud.DongTai();
	devCloud.Greedy();


	/*���һ��������*/
	devCloud.ImproveUtilization();

	//���������
	devCloud.CheckUnit();

	//�����
	devCloud.CheckResult();

	// ��Ҫ���������
	string result = devCloud.OutPut();
	char * result_file = (char *)result.c_str();

	// ֱ�ӵ�������ļ��ķ��������ָ���ļ���(ps��ע���ʽ����ȷ�ԣ�����н⣬��һ��ֻ��һ�����ݣ��ڶ���Ϊ�գ������п�ʼ���Ǿ�������ݣ�����֮����һ���ո�ָ���)
	write_result(result_file, filename);
}
