#ifndef _MIXEDFLOWCONTROL_SUBSCRIBER_H_
#define _MIXEDFLOWCONTROL_SUBSCRIBER_H_

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <sstream>
#include <random>
#include <algorithm>
#include <fstream>
#include <cmath>
#include <vector>
#include "MixedMsgPubSubTypes.h"

class MixedFlowControlSubscriber
{
public:

    MixedFlowControlSubscriber();

    virtual ~MixedFlowControlSubscriber();

    bool init();

    void run();

private:

    eprosima::fastdds::dds::DomainParticipant* participant_;

    eprosima::fastdds::dds::Subscriber* subscriber_;

    eprosima::fastdds::dds::Topic* topic_;

    eprosima::fastdds::dds::DataReader* reader_;

    class SubListener : public eprosima::fastdds::dds::DataReaderListener
    {
public:

        SubListener()
            : n_matched(0)
            , n_msg(0)
        {
            statisticsOut.open(data_path + "result/receive_statistics.csv", std::ios::trunc);
            // 数据接收的序列号，数据发送的序列号，数据类型，数据优先级，发送时间，接收时间，传输时间
		    statisticsOut << "receive_msg_num, send_msg_num, msg_type, msg_priority, msg_start_time, msg_end_time, msg_trans_time" << std::endl;
        }

        ~SubListener() override
        {
            statisticsOut.close();
        }

        void on_data_available(
                eprosima::fastdds::dds::DataReader* reader) override;

        void on_subscription_matched(
                eprosima::fastdds::dds::DataReader* reader,
                const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;

        int n_matched;

        int n_msg;
        int received_num = 0;
        std::ofstream statisticsOut;
        // /home/jich/code/source/FastDDS_v2_6_6/src/fastdds_exp/C++/DDS/MixedFlowControlTest/data/result
        std::string data_path = "/home/jich/code/source/FastDDS_v2_6_6/src/fastdds_exp/C++/DDS/MixedFlowControlTest/data/";
    } m_listener;

    eprosima::fastdds::dds::TypeSupport myType;
    
};

#endif // _MIXEDFLOWCONTROL_SUBSCRIBER_H_
