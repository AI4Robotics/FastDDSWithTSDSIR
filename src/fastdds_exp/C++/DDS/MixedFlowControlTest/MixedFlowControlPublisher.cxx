#include <fastdds/dds/domain/DomainParticipantFactory.hpp>

#include "MixedFlowControlPublisher.h"

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps::rtps;

MixedFlowControlPublisher::MixedFlowControlPublisher()
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
    , myType(new MixedMsgPubSubType())
    , myType_2(new FlowControlExamplePubSubType())
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
    flow_control_300k_per_sec->scheduler = eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::ROUND_ROBIN;
    // flow_control_300k_per_sec->max_bytes_per_period = 300 * 1000;
    // flow_control_300k_per_sec->max_bytes_per_period = 1000;
    flow_control_300k_per_sec->max_bytes_per_period = 20 * 1000;
    flow_control_300k_per_sec->period_ms = 1000;
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
    myType_2.register_type(participant_);

    a_publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT);
    if (a_publisher_ == nullptr)
    {
        return false;
    }

    // Create Topic
    topic_ = participant_->create_topic("MixedMsgPubSubTopic", myType.get_type_name(), TOPIC_QOS_DEFAULT);
    topic_2 = participant_->create_topic("FlowControlExampleTopic", myType_2.get_type_name(), TOPIC_QOS_DEFAULT);
    
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

void MixedFlowControlPublisher::run(std::string data_path, int discrete_num, int total_num)
{
    while (m_listener.n_matched == 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    std::cout << "############ Mixed Flow Control Test ############" << std::endl;

    MixedMsg st;
    int64_t sequence_num = 0; // 消息序列号
    short priority = 10; // 消息优先级
    std::string type = "test_a"; // 消息类型
    int32_t start_seconds = 0; // 消息创建时间
    uint32_t start_nanosec = 0;
    int32_t end_seconds = 0; // 消息创建时间
    uint32_t end_nanosec = 0;
    int32_t trans_time = 0; // 已传输时间
    bool is_overtime = false; // 是否已超时 / 或在此表示是否发送成功
    std::cout << "传输消息的大小：" << sizeof(MixedMsg) << std::endl;

    Time_t now_time;
    std::string str = std::to_string(int(discrete_num * 1.0 / total_num * 100));
    std::string msg_sequence_file = data_path + "input_data_" + str + "%.txt";

    if (read_data(msg_sequence_file))
    {
        if (discrete_num != discrete_msg_size || total_num != msg_sequence_size)
        {
            std::cout << "数据读入不完整，退出发送测试" << std::endl;
            return;
        }
        std::cout << "数据读入成功" << std::endl;
		// std::string str = std::to_string(int(discrete_msg_size * 1.0 / msg_sequence_size * 100));
        std::cout << "Percentage of discrete packets: " << str + "%" << std::endl;
		std::ofstream statisticsOut;
		statisticsOut.open(data_path + "result/send_statistics_" + str + "%.csv", std::ios::trunc);
		statisticsOut << "msg_number, msg_priority, msg_type, msg_start_time, msg_success_send" << std::endl;
        // 遍历待发送的数据序列依次发送
        int priority = 0;
        for (int i = 1; i <= total_num; ++i)
        {
            priority = msg_sequence[i];
            switch (priority)
            {
            case 6:
                st.msg_sequence_num(i);
                st.msg_priority(6);
                st.msg_type("test_a");
                // 设置数据生成时间
                Time_t::now(now_time);
                st.msg_start_seconds(now_time.seconds());
                st.msg_start_nanosec(now_time.nanosec());
                // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                if (a_writer_->write(&st))
                {
                    st.is_overtime(true);
                }
                else
                {
                    st.is_overtime(false);
                }
                statisticsOut << st.msg_sequence_num() << ", " << st.msg_priority() << ", " 
                    << st.msg_type() << ", " << now_time << ", " << st.is_overtime() << std::endl;
                std::cout << "Sending sample, msg_sequence_num=" << i << " through the a_writer. " << std::endl;
                break;
            case 4:
                st.msg_sequence_num(i);
                st.msg_priority(4);
                st.msg_type("test_b");
                Time_t::now(now_time);
                st.msg_start_seconds(now_time.seconds());
                st.msg_start_nanosec(now_time.nanosec());
                if (b_writer_->write(&st))
                {
                    st.is_overtime(true);
                }
                else
                {
                    st.is_overtime(false);
                }
                statisticsOut << st.msg_sequence_num() << ", " << st.msg_priority() << ", " 
                    << st.msg_type() << ", " << now_time << ", " << st.is_overtime() << std::endl;
                std::cout << "Sending sample, msg_sequence_num=" << i << " through the b_writer. " << std::endl;
                break;
            case 2:
                st.msg_sequence_num(i);
                st.msg_priority(2);
                st.msg_type("test_c");
                Time_t::now(now_time);
                st.msg_start_seconds(now_time.seconds());
                st.msg_start_nanosec(now_time.nanosec());
                if (c_writer_->write(&st))
                {
                    st.is_overtime(true);
                }
                else
                {
                    st.is_overtime(false);
                }
                statisticsOut << st.msg_sequence_num() << ", " << st.msg_priority() << ", " 
                    << st.msg_type() << ", " << now_time << ", " << st.is_overtime() << std::endl; 
                std::cout << "Sending sample, msg_sequence_num=" << i << " through the c_writer. " << std::endl;
                break;
            case 0:
                st.msg_sequence_num(i);
                st.msg_priority(0);
                st.msg_type("test_d");
                Time_t::now(now_time);
                st.msg_start_seconds(now_time.seconds());
                st.msg_start_nanosec(now_time.nanosec());
                if (d_writer_->write(&st))
                {
                    st.is_overtime(true);
                }
                else
                {
                    st.is_overtime(false);
                }
                statisticsOut << st.msg_sequence_num() << ", " << st.msg_priority() << ", " 
                    << st.msg_type() << ", " << now_time << ", " << st.is_overtime() << std::endl;
                std::cout << "Sending sample, msg_sequence_num=" << i << " through the d_writer. " << std::endl;
                break;
            default:
                break;
            }

            std::cout << "############ 等待100ms ############" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        statisticsOut.close();
    }
}

/**
 * 生成测试的待发送数据序列
 *  索引：数据包序号（1~10000），数据：数据包优先级（0，1，4，5）
 *  离散数据包（pro=5）占比：0%~50%
 *  保存数据到文件，供不同的调度算法测试
*/
void MixedFlowControlPublisher::make_DataSeq(std::string data_path, int total_num, int discrete_min, int discrete_max, int gap)
{
    std::ofstream make_data_seq;
    // int total_num = 10000, discrete_min = 0, discrete_max = 5000, gap = 500;
    for (int i = discrete_min; i <= discrete_max; i += gap) {
        std::string str = std::to_string(int(i * 1.0 / total_num * 100));
        std::cout << "Percentage of discrete packets: " << str + "%.txt" << std::endl;
		// ~/source/ai4robotics_lab/Fast-DDS/Fast-DDS/examples/cpp/dds/TSDSIRSimTest/data
        // /home/jich/code/source/FastDDS_v2_6_6/src/fastdds_exp/C++/DDS/MixedFlowControlTest/data
        make_data_seq.open(data_path + "input_data_" + str + "%.txt", std::ios::trunc);

        std::vector<int> total_seq(total_num, 10); // 默认生成的数据优先级为10，即最低
        std::vector<int> discrete_seq;
        std::default_random_engine e(static_cast<unsigned int>(time(nullptr)));
        // std::default_random_engine e;
        std::uniform_int_distribution<unsigned int> u(0, total_num-1);
        int n = 0;
        while (n < i)
        {
            int temp = u(e);
            if (!std::count(discrete_seq.begin(), discrete_seq.end(), temp))
            {
                discrete_seq.push_back(temp);
                n++;
            }
        }
        std::sort(discrete_seq.begin(), discrete_seq.end());
        for (std::vector<int>::iterator it = discrete_seq.begin(); it != discrete_seq.end(); it++)
        {
            std::cout << (*it) << "\t";
            total_seq[(*it)] = 0;
        }
        std::cout << std::endl << "============================================================" << std::endl;
        for (std::vector<int>::iterator it = total_seq.begin(); it != total_seq.end(); it++)
        {
            if (*it == 10)
            {
                int a = rand() % 3;
                switch (a)
                {
                    case 0:
                    {
                        *it = 6;
                        break;
                    }
                    case 1:
                    {
                        *it = 4;
                        break;
                    }
                    case 2:
                    {
                        *it = 2;
                        break;
                    }
                }
            } 
            std::cout << (*it) << "\t";
            make_data_seq << (*it) << std::endl;
        }
        std::cout << std::endl;
        make_data_seq.close();
    }
}

/**
 * 根据文件路径读取待测试的消息发送序列
*/
int MixedFlowControlPublisher::read_data(std::string msg_sequence_file)
{
	int discrete_data = 0, all_data = 0;
    msg_sequence.clear();
	std::ifstream r;
	std::cout << msg_sequence_file << std::endl;
	r.open(msg_sequence_file, std::ios::in);
	if (r.is_open())
	{
		std::cout << "======文件打开成功======" << std::endl;
		int temp;
		while (r >> temp)
		{
			// std::cout << temp << std::endl;
			if (temp == 0)
			{
				discrete_data++;
			}
			all_data++;
			msg_sequence.push_back(temp);
		}
        msg_sequence_size = all_data;
        discrete_msg_size = discrete_data;
		std::cout << "all_data: " << all_data 
			<< ", discrete_data: " << discrete_data << std::endl;
		std::cout << "len of vector: " << msg_sequence.size() << std::endl;
		return 1;		
	}
	else
	{
		std::cout << "======文件打开失败======" << std::endl;
		return 0;
	}
}
