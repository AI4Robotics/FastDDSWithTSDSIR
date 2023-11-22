#include <fastdds/dds/domain/DomainParticipantFactory.hpp>

#include "MixedFlowControlPublisher.h"

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps::rtps;

MixedFlowControlPublisher::MixedFlowControlPublisher(std::string data_path)
    : participant_(nullptr)
    , a_publisher_(nullptr)
    , b_publisher_(nullptr)
    , c_publisher_(nullptr)
    , d_publisher_(nullptr)
    , topic_(nullptr)
    , topic_2(nullptr)
    , a_writer_(nullptr)
    , b_writer_(nullptr)
    , c_writer_(nullptr)
    , d_writer_(nullptr)
    , myType(new MsgRGBImagePubSubType())
    // , myType_2(new FlowControlExamplePubSubType())
    , data_path_(data_path)
{
}

MixedFlowControlPublisher::~MixedFlowControlPublisher()
{
    if (a_writer_ != nullptr)
    {
        a_publisher_->delete_datawriter(a_writer_);
    }
    if (b_writer_ != nullptr)
    {
        b_publisher_->delete_datawriter(b_writer_);
    }
    if (c_writer_ != nullptr)
    {
        c_publisher_->delete_datawriter(c_writer_);
    }
    if (d_writer_ != nullptr)
    {
        d_publisher_->delete_datawriter(d_writer_);
    }
    if (a_publisher_ != nullptr)
    {
        participant_->delete_publisher(a_publisher_);
    }
    if (b_publisher_ != nullptr)
    {
        participant_->delete_publisher(b_publisher_);
    }
    if (c_publisher_ != nullptr)
    {
        participant_->delete_publisher(c_publisher_);
    }
    if (d_publisher_ != nullptr)
    {
        participant_->delete_publisher(d_publisher_);
    }
    if (topic_ != nullptr)
    {
        participant_->delete_topic(topic_);
    }
    if (topic_2 != nullptr)
    {
        participant_->delete_topic(topic_2);
    }
    DomainParticipantFactory::get_instance()->delete_participant(participant_);
}

bool MixedFlowControlPublisher::init()
{
    ///////////////////////////////////////////////////////////////////////////
    // Limit to 300kb per second.
    static const char* flow_controller_name = "example_flow_controller";
    auto flow_control_300k_per_sec = std::make_shared<eprosima::fastdds::rtps::FlowControllerDescriptor>();
    flow_control_300k_per_sec->name = flow_controller_name;
    // flow_control_300k_per_sec->scheduler = eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::HIGH_PRIORITY;
    // flow_control_300k_per_sec->scheduler = eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::FIFO;
    // flow_control_300k_per_sec->scheduler = eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::ROUND_ROBIN;
    flow_control_300k_per_sec->scheduler = eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::TSDSIR;
    // flow_control_300k_per_sec->max_bytes_per_period = 1024 * 300; // 300KB/s
    // flow_control_300k_per_sec->max_bytes_per_period = 1024 * 400; // 400KB/s
    // flow_control_300k_per_sec->max_bytes_per_period = 1024 * 500; // 400KB/s
    // flow_control_300k_per_sec->max_bytes_per_period = 1024 * 600; // 600KB/s
    // flow_control_300k_per_sec->max_bytes_per_period = 1024 * 900; // 900KB/s
    flow_control_300k_per_sec->max_bytes_per_period = 50 * 1024 * 1024; // 1MB/s
    flow_control_300k_per_sec->period_ms = 1000; // 1s
    ///////////////////////////////////////////////////////////////////////////

    // Create Participant
    DomainParticipantQos pqos;
    pqos.wire_protocol().builtin.discovery_config.leaseDuration = eprosima::fastrtps::c_TimeInfinite;
    pqos.flow_controllers().push_back(flow_control_300k_per_sec);   // Register flow controller on participant
    pqos.name("Participant_publisher");  //You can put here the name you want

    participant_ = DomainParticipantFactory::get_instance()->create_participant(0, pqos);
    if (participant_ == nullptr)
    {
        return false;
    }
    //Register the type
    myType.register_type(participant_);
    // myType_2.register_type(participant_);

    a_publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT);
    if (a_publisher_ == nullptr)
    {
        return false;
    }

    // Create Topic
    topic_ = participant_->create_topic("MsgRGBImagePubSubTopic", myType.get_type_name(), TOPIC_QOS_DEFAULT);
    // topic_2 = participant_->create_topic("FlowControlExampleTopic", myType_2.get_type_name(), TOPIC_QOS_DEFAULT);
    
    if (topic_ == nullptr)
    {
        return false;
    }

    DataWriterQos waqos;
    waqos.publish_mode().kind = ASYNCHRONOUS_PUBLISH_MODE;
    waqos.publish_mode().flow_controller_name = flow_controller_name; // Link writer to the registered flow controller.
    waqos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;
    // waqos.history().depth = 10;
    Property aproperty;
    aproperty.name("fastdds.sfc.priority");
    aproperty.value("6");
    waqos.properties().properties().emplace_back(aproperty);
    a_writer_ = a_publisher_->create_datawriter(topic_, waqos, &m_listener);
    if (a_writer_ == nullptr)
    {
        return false;
    }
    std::cout << "a# publisher created, waiting for subscribers..." << std::endl;


    b_publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT);
    if (b_publisher_ == nullptr)
    {
        return false;
    }

    DataWriterQos wbqos;
    wbqos.publish_mode().kind = ASYNCHRONOUS_PUBLISH_MODE;
    wbqos.publish_mode().flow_controller_name = flow_controller_name; // Link writer to the registered flow controller.
    wbqos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;
    // wbqos.history().depth = 10;
    Property bproperty;
    bproperty.name("fastdds.sfc.priority");
    bproperty.value("4");
    wbqos.properties().properties().emplace_back(bproperty);
    b_writer_ = b_publisher_->create_datawriter(topic_, wbqos, &m_listener);
    if (b_writer_ == nullptr)
    {
        return false;
    }
    std::cout << "b# publisher created, waiting for subscribers..." << std::endl;


    c_publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT);
    if (c_publisher_ == nullptr)
    {
        return false;
    }

    DataWriterQos wcqos;
    wcqos.publish_mode().kind = ASYNCHRONOUS_PUBLISH_MODE;
    wcqos.publish_mode().flow_controller_name = flow_controller_name;
    wcqos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;
    // wcqos.history().depth = 10;
    Property cproperty;
    cproperty.name("fastdds.sfc.priority");
    cproperty.value("2");
    wcqos.properties().properties().emplace_back(cproperty);
    c_writer_ = c_publisher_->create_datawriter(topic_, wcqos, &m_listener);
    if (c_writer_ == nullptr)
    {
        return false;
    }
    std::cout << "c# publisher created, waiting for subscribers..." << std::endl;


    d_publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT);
    if (d_publisher_ == nullptr)
    {
        return false;
    }

    DataWriterQos wdqos;
    wdqos.publish_mode().kind = ASYNCHRONOUS_PUBLISH_MODE;
    wdqos.publish_mode().flow_controller_name = flow_controller_name;
    wdqos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;
    // wdqos.history().depth = 10;
    Property dproperty;
    dproperty.name("fastdds.sfc.priority");
    dproperty.value("0");
    wdqos.properties().properties().emplace_back(dproperty);
    d_writer_ = d_publisher_->create_datawriter(topic_, wdqos, &m_listener);
    if (d_writer_ == nullptr)
    {
        return false;
    }
    std::cout << "d# publisher created, waiting for subscribers..." << std::endl;

    return true;
}

void MixedFlowControlPublisher::PubListener::on_publication_matched(
        eprosima::fastdds::dds::DataWriter*,
        const eprosima::fastdds::dds::PublicationMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        n_matched = info.total_count;
        std::cout << "Publisher matched." << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        n_matched = info.total_count;
        std::cout << "Publisher unmatched." << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
    }
}

void MixedFlowControlPublisher::run()
{
    while (m_listener.n_matched == 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    std::cout << "############ Mixed Flow Control Test, 传输消息的大小：" << sizeof(MsgRGBImage) << "###############" << std::endl;
    /*
        TODO:
            开辟四个子线程，每个线程对应一个DataWriter（不同优先级DW发送不同优先级/类型的消息）；
            a_writer_ -- 6 -- 600 -- 1ms/msg = 600ms
            b_writer_ -- 4 -- 300 -- 2ms/msg = 600ms
            c_writer_ -- 2 -- 200 -- 3ms/msg = 600ms
            d_writer_ -- 0 -- 100 -- 6ms/msg = 600ms
    */
   	short a_priority = 6, b_priority = 4, c_priority = 2, d_priority = 0;
	int a_send_sum = 1000, b_send_sum = 300, c_send_sum = 200, d_send_sum = 100;
	std::string a_type_name = "a", b_type_name = "b", c_type_name = "c", d_type_name = "d";
	int a_ms_per_msg = 34, b_ms_per_msg = 2, c_ms_per_msg = 3, d_ms_per_msg = 6;
	int a_thread_end = 0, b_thread_end = 0, c_thread_end = 0, d_thread_end = 0; 
    Time_t start_time;
    Time_t::now(start_time);
	std::thread th1(&MixedFlowControlPublisher::sub_thread_write, this, &a_writer_, 
					a_priority, a_send_sum, a_type_name, a_ms_per_msg, &a_thread_end);
	th1.detach();
    std::thread th2(&MixedFlowControlPublisher::sub_thread_write, this, &b_writer_, 
					b_priority, a_send_sum, b_type_name, a_ms_per_msg, &b_thread_end);
	th2.detach();
    std::thread th3(&MixedFlowControlPublisher::sub_thread_write, this, &c_writer_, 
					c_priority, a_send_sum, c_type_name, a_ms_per_msg, &c_thread_end);
	th3.detach();
    std::thread th4(&MixedFlowControlPublisher::sub_thread_write, this, &d_writer_, 
					d_priority, a_send_sum, d_type_name, a_ms_per_msg, &d_thread_end);
	th4.detach();
	while (a_thread_end == 0 || b_thread_end == 0 || c_thread_end == 0 || d_thread_end == 0)
	{
		// 等待子线程退出
	}
    Time_t end_time;
    Time_t::now(end_time);

	std::cout << "---------- send end, speed " << end_time.to_ns() - start_time.to_ns() << "ns -------------" << std::endl;
}

/**
 * 发送子线程
 *  a_writer_以一定的频率发送优先级/类型为a的消息，总共发送
*/
void MixedFlowControlPublisher::sub_thread_write(eprosima::fastdds::dds::DataWriter** writer, 
        short priority, int send_sum, std::string type_name, int ms_per_msg, int* thread_end)
{
    int index = 0;
    std::ofstream statisticsOut;
    statisticsOut.open(data_path_ + "send/msg_" + type_name + "_send.csv", std::ios::trunc);
    // 待发送消息的 序号、优先级、类型、生成时间、是否成功发出
	statisticsOut << "msg_index, msg_priority, msg_type, msg_start_time(ts), msg_success_send" << std::endl;
    MsgRGBImage st;
    Time_t now_time;

    while (index < send_sum)
    {
        // 初始化消息
        st.msg_sequence_num(++index);
        st.msg_priority(priority);
        st.msg_type(type_name);
        Time_t::now(now_time);  // 设置数据生成时间
        st.msg_start_seconds(now_time.seconds());
        st.msg_start_nanosec(now_time.nanosec());
        // 发送消息
        if ((*writer)->write(&st))
        {
            st.is_sent(true);
        }
        else
        {
            st.is_sent(false);
        }
        statisticsOut << st.msg_sequence_num() << ", " << st.msg_priority() << ", " 
        << st.msg_type() << ", " << now_time << ", " << st.is_sent() << std::endl;
        std::cout << "@@@ Sending sample of type: " << st.msg_type() 
            << ", msg_sequence_num =" << st.msg_sequence_num() 
            << ", msg_priority =" << st.msg_priority() << " @@@" << std::endl;
        // 睡眠，控制发送频率
        std::this_thread::sleep_for(std::chrono::milliseconds(ms_per_msg));
    }
    statisticsOut.close();
    std::cout << "...... sub_thread_write " << type_name << "end ......" << std::endl;
    *thread_end = 1;
}

