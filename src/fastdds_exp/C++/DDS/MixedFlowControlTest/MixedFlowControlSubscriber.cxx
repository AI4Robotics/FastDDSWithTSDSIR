#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>

#include "MixedFlowControlSubscriber.h"

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps::rtps;

MixedFlowControlSubscriber::MixedFlowControlSubscriber()
    : participant_(nullptr)
    , subscriber_(nullptr)
    , topic_(nullptr)
    , reader_(nullptr)
    , myType(new MixedMsgPubSubType())
{
}

MixedFlowControlSubscriber::~MixedFlowControlSubscriber()
{
    if (reader_ != nullptr)
    {
        subscriber_->delete_datareader(reader_);
    }
    if (topic_ != nullptr)
    {
        participant_->delete_topic(topic_);
    }
    if (subscriber_ != nullptr)
    {
        participant_->delete_subscriber(subscriber_);
    }
    DomainParticipantFactory::get_instance()->delete_participant(participant_);
}

bool MixedFlowControlSubscriber::init()
{
    // Create Participant
    DomainParticipantQos pqos;
    pqos.wire_protocol().builtin.discovery_config.leaseDuration = eprosima::fastrtps::c_TimeInfinite;
    pqos.name("Participant_subscriber"); //You can put the name you want

    participant_ = DomainParticipantFactory::get_instance()->create_participant(0, pqos);

    if (participant_ == nullptr)
    {
        return false;
    }

    //Register the type
    myType.register_type(participant_);

    // Create Subscriber
    subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT);

    if (subscriber_ == nullptr)
    {
        return false;
    }

    // Create Topic
    topic_ = participant_->create_topic("MixedMsgPubSubTopic", myType.get_type_name(), TOPIC_QOS_DEFAULT);

    if (topic_ == nullptr)
    {
        return false;
    }

    DataReaderQos rqos;
    rqos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;
    rqos.history().depth = 10;
    // rqos.data_sharing().off();
    // Create DataReader
    // reader_ = subscriber_->create_datareader(topic_, DATAREADER_QOS_DEFAULT, &m_listener);
    reader_ = subscriber_->create_datareader(topic_, rqos, &m_listener);

    if (reader_ == nullptr)
    {
        return false;
    }

    return true;
}

void MixedFlowControlSubscriber::SubListener::on_subscription_matched(
        DataReader*,
        const SubscriptionMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        n_matched = info.total_count;
        std::cout << "Subscriber matched." << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        n_matched = info.total_count;
        std::cout << "Subscriber unmatched." << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for SubscriptionMatchedStatus current count change" << std::endl;
    }
}

void MixedFlowControlSubscriber::SubListener::on_data_available(
        DataReader* reader)
{
    SampleInfo info;
    MixedMsg st;
    Time_t now_time;
    Time_t start_time;
    Time_t during_time;
    if (reader->take_next_sample(&st, &info) == ReturnCode_t::RETCODE_OK)
    {
        if (info.valid_data)
        {
            ++received_num;
            // 设置数据接收时间
            Time_t::now(now_time);
            st.msg_end_seconds(now_time.seconds());
            st.msg_end_nanosec(now_time.nanosec());
            start_time.seconds(st.msg_start_seconds());
            start_time.nanosec(st.msg_start_nanosec());
            std::cout << "$$$$$$$$ time before sending : " << start_time << "$$$$$$$$" << std::endl;
            std::cout << "$$$$$$$$ time after sending : " << now_time << "$$$$$$$$" << std::endl;
            during_time = now_time - start_time;

            // Print your structure data here.
            statisticsOut << received_num << ", " << st.msg_sequence_num() << "," << st.msg_type() 
                << ", " << st.msg_priority() << ", " << start_time << ", " << now_time << "," << during_time << std::endl;
            std::cout << "***Sample received msg_type: " << st.msg_type() << "\t msg_sequence_num: " << st.msg_sequence_num() 
                << "\t msg_priority: " << st.msg_priority() << "\t msg_trans_time: " << during_time <<
                "***" << std::endl;

        }
    }
}

void MixedFlowControlSubscriber::run()
{
    std::cout << "Waiting for Data, press Enter to stop the Subscriber. " << std::endl;
    std::cin.ignore();
    std::cout << "Shutting down the Subscriber." << std::endl;
}
