#ifndef _MIXEDFLOWCONTROL_PUBLISHER_H_
#define _MIXEDFLOWCONTROL_PUBLISHER_H_

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <sstream>
#include <random>
#include <algorithm>
#include <fstream>
#include <cmath>
#include <vector>
#include "MixedMsgPubSubTypes.h"
#include "FlowControlExamplePubSubTypes.h"


class MixedFlowControlPublisher
{
public:

    MixedFlowControlPublisher();

    virtual ~MixedFlowControlPublisher();

    bool init();

    // void run();
    void run(std::string data_path, int discrete_num, int total_num);

    /**
     * 生成测试的待发送数据序列
     *  索引：数据包序号（1~10000），数据：数据包优先级（6 4 2 0）
     *  离散数据包（pro=5）占比：0%~50%
     *  保存数据到文件，供不同的调度算法测试
    */
    void make_DataSeq(std::string data_path, int total_num, int discrete_min, int discrete_max, int gap);
    
    int read_data(std::string msg_sequence_file);

private:

    eprosima::fastdds::dds::DomainParticipant* participant_;

    eprosima::fastdds::dds::Publisher* a_publisher_;
    eprosima::fastdds::dds::Publisher* b_publisher_;
    eprosima::fastdds::dds::Publisher* c_publisher_;
    eprosima::fastdds::dds::Publisher* d_publisher_;

    eprosima::fastdds::dds::Topic* topic_;
    eprosima::fastdds::dds::Topic* topic_2;

    eprosima::fastdds::dds::DataWriter* a_writer_;
    eprosima::fastdds::dds::DataWriter* b_writer_;
    eprosima::fastdds::dds::DataWriter* c_writer_;
    eprosima::fastdds::dds::DataWriter* d_writer_;

    class PubListener : public eprosima::fastdds::dds::DataWriterListener
    {
public:

        PubListener()
            : n_matched(0)
        {
        }

        ~PubListener() override
        {
        }

        void on_publication_matched(
                eprosima::fastdds::dds::DataWriter* writer,
                const eprosima::fastdds::dds::PublicationMatchedStatus& info) override;

        int n_matched;

    } m_listener;

    eprosima::fastdds::dds::TypeSupport myType;
    eprosima::fastdds::dds::TypeSupport myType_2;

    std::vector<int> msg_sequence; // 消息发送序列

    int msg_sequence_size = 0; // 消息总数量
    int discrete_msg_size = 0; // 离散数据数量，一般为最高优先级

};

#endif // _MIXEDFLOWCONTROL_PUBLISHER_H_
