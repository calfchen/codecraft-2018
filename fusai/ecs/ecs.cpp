#include "predict.h"
#include "lib_io.h"
#include "lib_time.h"
#include "stdio.h"

// void predict_server(char * info[MAX_INFO_NUM], char * data[MAX_DATA_NUM], int data_num, char * filename);
// void print_time(const char * const head);
// int read_file(char ** const buff, const unsigned int spec, const char * const filename);
// void write_result(const char * const buff,const char * const filename);
// void release_buff(char ** const buff, const int valid_item_num);


int main(int argc, char *argv[])
{
    print_time("Begin");
	char *data[MAX_DATA_NUM];
    char *info[MAX_INFO_NUM];
	int data_line_num;
    int info_line_num;


    // char *data_file = argv[1];
    char *data_file = (char *)"train.txt";

    data_line_num = read_file(data, MAX_DATA_NUM, data_file);

    printf("data file line num is :%d \n", data_line_num);
    if (data_line_num == 0)
    {
        printf("Please input valid data file.\n");
        return -1;
    }
	
    // char *input_file = argv[2];
    char *input_file = (char *)"input.txt";

    info_line_num = read_file(info, MAX_INFO_NUM, input_file);

    printf("input file line num is :%d \n", info_line_num);
    if (info_line_num == 0)
    {
        printf("Please input valid info file.\n");
        return -1;
    }

    // char *output_file = argv[3];
    char *output_file = (char *)"output.txt";

    predict_server(info, data, data_line_num, output_file);

    release_buff(info, info_line_num);
	release_buff(data, data_line_num);

    print_time("End");

	return 0;
}

