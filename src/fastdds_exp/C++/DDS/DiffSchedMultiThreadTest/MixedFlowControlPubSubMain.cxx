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

    std::string data_path = "/home/pi/code/source/FastDDSWithTSDSIR/src/fastdds_exp/C++/DDS/DiffSchedMultiThreadTest/data/";
    switch(type)
    {
        case 1:
            {
                MixedFlowControlPublisher mypub(data_path);
                if (mypub.init())
                {
                    /*
                        TODO:
                        开辟四个子线程分别为四个优先级DW的发送线程，来发送四种优先级的数据
                    */
                    mypub.run();
                    std::cout << "======test end.======" << std::endl;
                }
                break;
            }
        case 2:
            {
                MixedFlowControlSubscriber mysub(data_path);
                if (mysub.init())
                {
                    mysub.run();
                }
                break;
            }
    }

    return 0;
}
