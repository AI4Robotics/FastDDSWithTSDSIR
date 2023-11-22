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
#include <thread>
#include "msg/MsgRGBImagePubSubTypes.h"


class MixedFlowControlPublisher
{
public:

    MixedFlowControlPublisher(std::string data_path);

    virtual ~MixedFlowControlPublisher();

    bool init();

    void run();

    void sub_thread_write(eprosima::fastdds::dds::DataWriter** writer, 
        short priority, int send_sum, std::string type_name, int ms_per_msg, int* thread_end);

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

        PubListener() : n_matched(0)
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

    std::string data_path_;
};

#endif // _MIXEDFLOWCONTROL_PUBLISHER_H_
