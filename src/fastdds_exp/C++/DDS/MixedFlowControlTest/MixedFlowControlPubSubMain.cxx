#include "MixedFlowControlPublisher.h"
#include "MixedFlowControlSubscriber.h"


#include <fastrtps/Domain.h>
#include <fastrtps/log/Log.h>

using namespace eprosima;
using namespace eprosima::fastrtps;

int main(int argc, char** argv)
{
    std::cout << "Starting " << std::endl;
    int type = 1;
    if (argc > 1)
    {
        if (strcmp(argv[1], "publisher") == 0)
        {
            type = 1;
        }
        else if (strcmp(argv[1], "subscriber") == 0)
        {
            type = 2;
        }
    }
    else
    {
        std::cout << "publisher OR subscriber argument needed" << std::endl;
        return 0;
    }

    // Register the type being used


    switch(type)
    {
        case 1:
            {
                MixedFlowControlPublisher mypub;
                if (mypub.init())
                {
                    std::string data_path = "/home/jich/code/source/FastDDS_v2_6_6/src/fastdds_exp/C++/DDS/MixedFlowControlTest/data/";
                    int total_num = 1000, discrete_min = 0, discrete_max = 500, gap = 50;

                    char generate_msg_sequence = 'N';
                    std::cout << "Whether to generate messages? Y/N: ";
                    std::cin >> generate_msg_sequence;
                    if (generate_msg_sequence == 'Y')
                    {
                        // 生成一组待发送数据的优先级序列，供多种调度算法测试
                        mypub.make_DataSeq(data_path, total_num, discrete_min, discrete_max, gap);   // 0, 500, 1000, 1500, ... 
                        // 0 50 100 150 200 250 300
                    }
                    /*
                        TODO:
                        已生成各离散占比不同的数据序列数据；
                        针对不同离散占比的数据集进行分组测试，mypub.run(data_file_40%);
                        mypub.run()：打开文件，依次遍历数据集，即按该顺序发送，不同的优先级数据使用不同优先级的DW来发送；
                        在mypub.run()遍历发送中，用csv记录发送的数据集信息（数据序列号，发送时间，优先级，消息类型等）;
                        订阅者接收消息，同时用csv记录依次接收到的数据集消息的信息（数据序列号，发送时间，接收时间，优先级，消息类型，传输时间等）
                    */
                    // mypub.run();
                    // for (int i = discrete_min; i <= discrete_max; i += gap)
                    // {
                    //     std::string str = std::to_string(int(i * 1.0 / total_num * 100));
                    //     std::string msg_sequence_file = data_path + "input_data_" + str + "%.txt";
                    //     mypub.run(msg_sequence_file); // 对不同离散占比的数据进行发送测试
                    //     std::cout << "======discrete num: " << i << " test end.======" << std::endl;
                    // }
                    int discrete_num = discrete_min + gap * 6;
                    mypub.run(data_path, discrete_num, total_num); // 对不同离散占比的数据进行发送测试 300 / 1000 = 30%
                    std::cout << "======discrete num: " << discrete_num << " test end.======" << std::endl;
                }
                break;
            }
        case 2:
            {
                MixedFlowControlSubscriber mysub;
                if (mysub.init())
                {
                    mysub.run();
                }
                break;
            }
    }

    return 0;
}
