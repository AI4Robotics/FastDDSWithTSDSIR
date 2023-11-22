#ifndef _RTPS_FLOWCONTROL_FLOWCONTROLLERIMPL_HPP_
#define _RTPS_FLOWCONTROL_FLOWCONTROLLERIMPL_HPP_

#include "FlowController.hpp"
#include <fastdds/rtps/common/Guid.h>
#include <fastdds/rtps/writer/RTPSWriter.h>
#include <fastdds/rtps/common/Time_t.h>

#include <atomic>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <map>
#include <mutex>
#include <thread>
#include <unordered_map>

namespace eprosima {
namespace fastdds {
namespace rtps {

/** Auxiliary classes **/

struct FlowQueue
{
    FlowQueue() noexcept = default;

    ~FlowQueue() noexcept
    {
        assert(new_interested_.is_empty());
        assert(old_interested_.is_empty());
    }

    FlowQueue(
            FlowQueue&& old) noexcept
    {
        swap(std::move(old));
    }

    FlowQueue& operator =(
            FlowQueue&& old) noexcept
    {
        swap(std::move(old));
        return *this;
    }

    void swap(
            FlowQueue&& old) noexcept
    {
        new_interested_.swap(old.new_interested_);
        old_interested_.swap(old.old_interested_);

        new_ones_.swap(old.new_ones_);
        old_ones_.swap(old.old_ones_);
    }

    bool is_empty() const noexcept
    {
        return new_ones_.is_empty() && old_ones_.is_empty();
    }

    ///////////////////////////////////////////////
    /**
     * Return whether the time-base queue is empty
    */
    bool is_time_queue_empty() const noexcept
    {
        return new_ones_.is_time_empty() && old_ones_.is_time_empty();
    }

    /**
     * The new_ones of this FlowQueue are added to time_queue in ascending order of 
     * remaining transmission time
    */
    void add_into_time_queue(FlowQueue& time_queue)
    {
        new_ones_.add_into_time_list(time_queue.new_ones_);
    }

    ///////////////////////////////////////////////

    /**
     * TODO: Introduction
    */
    void add_new_sample(
            fastrtps::rtps::CacheChange_t* change) noexcept
    {
        new_interested_.add_change(change);
        //std::cout << "==FlowQueue::add_new_sample(): size of added new_interested_: "<< 
            //new_interested_.list_size() << "==" << std::endl;
    }

    void add_old_sample(
            fastrtps::rtps::CacheChange_t* change) noexcept
    {
        old_interested_.add_change(change);
    }

    fastrtps::rtps::CacheChange_t* get_next_change() noexcept
    {
        if (!is_empty())
        {
            return !new_ones_.is_empty() ?
                   new_ones_.head.writer_info.next : old_ones_.head.writer_info.next;
        }

        return nullptr;
    }

    ///////////////////////////////////////////////
    /**
     * Get the next change of the time-based queue
    */
    fastrtps::rtps::CacheChange_t* get_down_change() noexcept
    {
        if (!is_time_queue_empty())
        {
            return !new_ones_.is_time_empty() ?
                new_ones_.earlier.writer_info.later : old_ones_.earlier.writer_info.later;
        }

        return nullptr;
    }
    ///////////////////////////////////////////////

    void add_interested_changes_to_queue() noexcept
    {
        // std::cout << "==SubThread-FlowQueue::add_interested_changes_to_queue(): before into queue, new_interested_大小为: "<< 
            //new_interested_.list_size() << "==" << std::endl;
        // This function should be called with mutex_  and interested_lock locked, because the queue is changed.
        new_ones_.add_list(new_interested_);
        old_ones_.add_list(old_interested_);
        // std::cout << "==SubThread-FlowQueue::add_interested_changes_to_queue(): interested into queue, new_ones_大小为: "<< 
            //new_ones_.list_size() << "==" << std::endl;
        // std::cout << "==SubThread-FlowQueue::add_interested_changes_to_queue(): interested into queue, old_ones_大小为: "<< 
        //     old_ones_.list_size() << "==" << std::endl;
    }

    ///////////////////////////////////////////////
    /**
     * Get size of the time-based queue
    */
    int get_time_size()
    {
        return new_ones_.list_time_size();
    }

    /**
     * Get size of the priority-based queue
    */
    int get_size()
    {
        return new_ones_.list_size();
    }

    void print_time_size()
    {
        new_ones_.print_time_list();
    }

    long double calc_ave_trans_time()
    {
        return new_ones_.calc_list_ave_trans_time();
    }

    void clear_time()
    {
        new_ones_.clear();
    }
    ///////////////////////////////////////////////

private:

    struct ListInfo
    {
        ListInfo() noexcept
        {
            clear();
        }

        void swap(
                ListInfo& other) noexcept
        {
            if (other.is_empty())
            {
                clear();
            }
            else
            {
                head.writer_info.next = other.head.writer_info.next;
                tail.writer_info.previous = other.tail.writer_info.previous;
                other.clear();
                head.writer_info.next->writer_info.previous = &head;
                tail.writer_info.previous->writer_info.next = &tail;
            }
        }

        void clear() noexcept
        {
            // clear priority-based list
            head.writer_info.next = &tail;
            tail.writer_info.previous = &head;
            ////////////////////////////////////
            // clear time-based list
            earlier.writer_info.later = &later;
            later.writer_info.earlier = &earlier;
            ////////////////////////////////////
        }

        bool is_empty() const noexcept
        {
            assert((&tail == head.writer_info.next && &head == tail.writer_info.previous) ||
                    (&tail != head.writer_info.next && &head != tail.writer_info.previous));
            return &tail == head.writer_info.next;
        }

        ////////////////////////////////////
        /**
         * Return whether the time-base list is empty
        */
        bool is_time_empty() const noexcept
        {
            assert((&later == earlier.writer_info.later && &earlier == later.writer_info.earlier) ||
                (&later != earlier.writer_info.later && &earlier != later.writer_info.earlier));
            return &later == earlier.writer_info.later;
        }
        ////////////////////////////////////

        /**
         * 
        */
        void add_change(
                fastrtps::rtps::CacheChange_t* change) noexcept
        {
            change->writer_info.previous = tail.writer_info.previous;
            change->writer_info.previous->writer_info.next = change;
            tail.writer_info.previous = change;
            change->writer_info.next = &tail;
        }

        void add_list(
                ListInfo& list) noexcept
        {
            if (!list.is_empty())
            {
                fastrtps::rtps::CacheChange_t* first = list.head.writer_info.next;
                fastrtps::rtps::CacheChange_t* last = list.tail.writer_info.previous;

                first->writer_info.previous = tail.writer_info.previous;
                first->writer_info.previous->writer_info.next = first;
                last->writer_info.next = &tail;
                tail.writer_info.previous = last;

                list.clear();
            }
        }

        ////////////////////////////////////
        /**
         * Inserts the elements of the current list into the passed argument list 
         * in ascending order of time remaining
        */
        void add_into_time_list(ListInfo& list)
        {
            // 从head到tail遍历，插入排序，添加到list中
            fastrtps::rtps::CacheChange_t* temp = head.writer_info.next;
            while (temp != &tail)
            {
                // std::cout << "... adding msg no." << temp->sequenceNumber << "\t ts: " << temp->sourceTimestamp 
				//     << "\t remain+now: " << temp->sourceTimestamp + temp->maxTransportTime << std::endl;
                // list.earlier.writer_info.later
                if (list.earlier.writer_info.later == &list.later && list.later.writer_info.earlier == &list.earlier) // 时间优先队列空队列
                {
                    temp->writer_info.earlier = list.later.writer_info.earlier;
                    temp->writer_info.earlier->writer_info.later = temp;
                    list.later.writer_info.earlier = temp;
                    temp->writer_info.later = &list.later;
                }
                else
                {
                    fastrtps::rtps::CacheChange_t* time_temp = list.earlier.writer_info.later;
                    // change找比它Treamin小的，并且temp不是最后一结点
                    while((temp->maxTransportTime + temp->sourceTimestamp) >=
                            (time_temp->maxTransportTime + time_temp->sourceTimestamp) && time_temp->writer_info.later != &list.later)
                    {
                        time_temp = time_temp->writer_info.later;
                    }
                    if ((temp->maxTransportTime + temp->sourceTimestamp) < 
                            (time_temp->maxTransportTime + time_temp->sourceTimestamp))
                    {
                        temp->writer_info.earlier = time_temp->writer_info.earlier;
                        temp->writer_info.earlier->writer_info.later = temp;
                        time_temp->writer_info.earlier = temp;
                        temp->writer_info.later = time_temp;
                    }
                    else if (time_temp->writer_info.later == &list.later)
                    {
                        temp->writer_info.earlier = list.later.writer_info.earlier;
                        temp->writer_info.earlier->writer_info.later = temp;
                        list.later.writer_info.earlier = temp;
                        temp->writer_info.later = &list.later;
                    }
                    else
                    {
                        std::cout << "**** ERROR add_change_by_remain_time ****" << std::endl;
                    }
                }
                // list.print_time_list();
                // std::cout << "..........................." << std::endl;
                temp = temp->writer_info.next;
            }
        }

        /**
         * Get length of the priority-based list
        */
        int list_size()
        {
            int size = 0;
            fastrtps::rtps::CacheChange_t* temp = &head;
            while (temp->writer_info.next != &tail)
            {
                temp = temp->writer_info.next;
                size++;
            }
            return size;
        }

        /**
         * Get length of the time-based list
        */
        int list_time_size()
        {
            int size = 0;
            fastrtps::rtps::CacheChange_t* temp = &earlier;
            while (temp->writer_info.later != &later)
            {
                temp = temp->writer_info.later;
                size++;
            }
            return size;
        }

        /**
         * Print change of time-based list
        */
        void print_time_list()
        {
            int size = 0;
            fastrtps::rtps::CacheChange_t* temp = &earlier;
            while (temp->writer_info.later != &later)
            {
                temp = temp->writer_info.later;
                std::cout << "msg no." << temp->sequenceNumber << "\t ts: " << temp->sourceTimestamp 
				    << "\t remain+now: " << temp->sourceTimestamp + temp->maxTransportTime << std::endl;
                size++;
            }
        }

        /**
         * Returns the average waiting time for the current queue
        */
        long double calc_list_ave_trans_time()
        {
            int len = list_time_size();
            if (len == 0)
            {
                return 0;
            }
            else
            {
                long double trans_time_sum = 0.0;
                fastrtps::rtps::CacheChange_t* temp = &earlier;
                // long double now = 10020; // 获取当前时间
                eprosima::fastrtps::rtps::Time_t now; 
                eprosima::fastrtps::rtps::Time_t::now(now);
                while (temp->writer_info.later != &later)
                {
                    temp = temp->writer_info.later;
                    trans_time_sum += ((temp->priority + 10) / 21.0 + 1) * (now - temp->sourceTimestamp).to_ns(); 
                }
                return trans_time_sum / len;
            }
        }
        ////////////////////////////////////

        fastrtps::rtps::CacheChange_t head;
        fastrtps::rtps::CacheChange_t tail;
        fastrtps::rtps::CacheChange_t earlier;
        fastrtps::rtps::CacheChange_t later;
    };

    //! List of interested new changes to be included.
    //! Should be protected with changes_interested_mutex.
    ListInfo new_interested_;

    //! List of interested old changes to be included.
    //! Should be protected with changes_interested_mutex.
    ListInfo old_interested_;

    //! List of new changes
    //! Should be protected with mutex_.
    ListInfo new_ones_;

    //! List of old changes
    //! Should be protected with mutex_.
    ListInfo old_ones_;
};

/** Classes used to specify FlowController's publication model **/

//! Only sends new samples synchronously. There is no mechanism to send old ones.
struct FlowControllerPureSyncPublishMode
{

    FlowControllerPureSyncPublishMode(
            fastrtps::rtps::RTPSParticipantImpl*,
            const FlowControllerDescriptor*)
    {
    }

};

//! Sends new samples asynchronously. Old samples are sent also asynchronously */
struct FlowControllerAsyncPublishMode
{
    FlowControllerAsyncPublishMode(
            fastrtps::rtps::RTPSParticipantImpl* participant,
            const FlowControllerDescriptor*)
        : group(participant, true)
    {
    }

    virtual ~FlowControllerAsyncPublishMode()
    {
        if (running)
        {
            {
                std::unique_lock<std::mutex> lock(changes_interested_mutex);
                running = false;
                cv.notify_one();
            }
            thread.join();
        }
    }

    bool fast_check_is_there_slot_for_change(
            fastrtps::rtps::CacheChange_t*) const
    {
        return true;
    }

    bool wait(
            std::unique_lock<std::mutex>& lock)
    {
        cv.wait(lock);
        return false;
    }

    bool force_wait() const
    {
        return false;
    }

    void process_deliver_retcode(
            const fastrtps::rtps::DeliveryRetCode&)
    {
    }

    std::thread thread;

    std::atomic_bool running {false};

    std::condition_variable cv;

    fastrtps::rtps::RTPSMessageGroup group;

    //! Mutex for interested samples to be added.
    std::mutex changes_interested_mutex;

    //! Used to warning async thread a writer wants to remove a sample.
    std::atomic<uint32_t> writers_interested_in_remove = {0};
};

//! Sends new samples synchronously. Old samples are sent asynchronously */
struct FlowControllerSyncPublishMode : public FlowControllerPureSyncPublishMode, public FlowControllerAsyncPublishMode
{

    FlowControllerSyncPublishMode(
            fastrtps::rtps::RTPSParticipantImpl* participant,
            const FlowControllerDescriptor* descriptor)
        : FlowControllerPureSyncPublishMode(participant, descriptor)
        , FlowControllerAsyncPublishMode(participant, descriptor)
    {
    }

};

//! Sends all samples asynchronously but with bandwidth limitation.
struct FlowControllerLimitedAsyncPublishMode : public FlowControllerAsyncPublishMode
{
    FlowControllerLimitedAsyncPublishMode(
            fastrtps::rtps::RTPSParticipantImpl* participant,
            const FlowControllerDescriptor* descriptor)
        : FlowControllerAsyncPublishMode(participant, descriptor)
    {
        assert(nullptr != descriptor);
        assert(0 < descriptor->max_bytes_per_period);

        max_bytes_per_period = descriptor->max_bytes_per_period;
        period_ms = std::chrono::milliseconds(descriptor->period_ms);
        group.set_sent_bytes_limitation(static_cast<uint32_t>(max_bytes_per_period));
    }

    bool fast_check_is_there_slot_for_change(
            fastrtps::rtps::CacheChange_t* change)
    {
        // Not fragmented sample, the fast check is if the serialized payload fit.
        uint32_t size_to_check = change->serializedPayload.length;

        if (0 != change->getFragmentCount())
        {
            // For fragmented sample, the fast check is the minor fragments fit.
            size_to_check = change->serializedPayload.length % change->getFragmentSize();

            if (0 == size_to_check)
            {
                size_to_check = change->getFragmentSize();
            }


        }

        bool ret = (max_bytes_per_period - group.get_current_bytes_processed()) > size_to_check;

        if (!ret)
        {
            force_wait_ = true;
        }

        return ret;
    }

    /*!
     * Wait until there is a new change added (notified by other thread) or there is a timeout (period was excedded and
     * the bandwidth limitation has to be reset.
     *
     * @return false if the condition_variable was awaken because a new change was added. true if the condition_variable was awaken because the bandwidth limitation has to be reset.
     */
    bool wait(
            std::unique_lock<std::mutex>& lock)
    {
        auto lapse = std::chrono::steady_clock::now() - last_period_;
        bool reset_limit = true;

        if (lapse < period_ms)
        {
            /*
                指定阻塞到一定的时间段并对mutex完成 unlock() 操作。
                在线程收到唤醒通知或者时间超时之前，对线程进行阻塞并且对mutex完成 unlock() 操作。
                唤醒线程后则自动完成 lock() 操作。如果无法 lock() 则继续阻塞直到获取到 mutex。
            */
            if (std::cv_status::no_timeout == cv.wait_for(lock, period_ms - lapse))
            {
                reset_limit = false;
            }
        }

        if (reset_limit)
        {
            last_period_ = std::chrono::steady_clock::now();
            force_wait_ = false;
            group.reset_current_bytes_processed();
        }

        return reset_limit;
    }

    bool force_wait() const
    {
        return force_wait_;
    }

    void process_deliver_retcode(
            const fastrtps::rtps::DeliveryRetCode& ret_value)
    {
        if (fastrtps::rtps::DeliveryRetCode::EXCEEDED_LIMIT == ret_value)
        {
            force_wait_ = true;
        }
    }

    int32_t max_bytes_per_period = 0;

    std::chrono::milliseconds period_ms;

private:

    bool force_wait_ = false;

    std::chrono::steady_clock::time_point last_period_ = std::chrono::steady_clock::now();
};


/** Classes used to specify FlowController's sample scheduling **/

//! Fifo scheduling
struct FlowControllerFifoSchedule
{
    void register_writer(
            fastrtps::rtps::RTPSWriter*) const
    {
    }

    void unregister_writer(
            fastrtps::rtps::RTPSWriter*) const
    {
    }

    void work_done() const
    {
        // Do nothing
    }

    void add_new_sample(
            fastrtps::rtps::RTPSWriter*,
            fastrtps::rtps::CacheChange_t* change)
    {
        std::cout << "==FlowControllerFifoSchedule::add_new_sample(): 向FlowQueue中添加change==" << std::endl;
        queue_.add_new_sample(change);
    }

    void add_old_sample(
            fastrtps::rtps::RTPSWriter*,
            fastrtps::rtps::CacheChange_t* change)
    {
        queue_.add_old_sample(change);
    }

    /*!
     * Returns the first sample in the queue.
     * Default behaviour.
     * Expects the queue is ordered.
     *
     * @return Pointer to next change to be sent. nullptr implies there is no sample to be sent or is forbidden due to
     * bandwidth exceeded.
     */
    fastrtps::rtps::CacheChange_t* get_next_change_nts()
    {
        return queue_.get_next_change();
    }

    /*!
     * Store the sample at the end of the list.
     *
     * @return true if there is added changes.
     */
    void add_interested_changes_to_queue_nts()
    {
        // This function should be called with mutex_  and interested_lock locked, because the queue is changed.
        queue_.add_interested_changes_to_queue();
    }

    void set_bandwith_limitation(
            uint32_t) const
    {
    }

    void trigger_bandwidth_limit_reset() const
    {
    }

private:

    //! Scheduler queue. FIFO scheduler only has one queue.
    FlowQueue queue_;
};

//! Round Robin scheduling
struct FlowControllerRoundRobinSchedule
{
    using element = std::tuple<fastrtps::rtps::RTPSWriter*, FlowQueue>;
    using container = std::vector<element>;
    using iterator = container::iterator;

    FlowControllerRoundRobinSchedule()
    {
        next_writer_ = writers_queue_.begin();
    }

    void register_writer(
            fastrtps::rtps::RTPSWriter* writer)
    {
        fastrtps::rtps::RTPSWriter* current_writer = nullptr;

        if (writers_queue_.end() != next_writer_)
        {
            current_writer = std::get<0>(*next_writer_);
        }

        assert(writers_queue_.end() == find(writer));
        writers_queue_.emplace_back(writer, FlowQueue());

        if (nullptr == current_writer)
        {
            next_writer_ = writers_queue_.begin();
        }
        else
        {
            next_writer_ = find(current_writer);
        }
    }

    void unregister_writer(
            fastrtps::rtps::RTPSWriter* writer)
    {
        // Queue cannot be empty, as writer should be present
        assert(writers_queue_.end() != next_writer_);
        fastrtps::rtps::RTPSWriter* current_writer = std::get<0>(*next_writer_);
        assert(nullptr != current_writer);

        auto it = find(writer);
        assert(it != writers_queue_.end());
        assert(std::get<1>(*it).is_empty());

        // Go to the next writer when unregistering the current one
        if (it == next_writer_)
        {
            set_next_writer();
            current_writer = std::get<0>(*next_writer_);
        }

        writers_queue_.erase(it);
        if (writer == current_writer)
        {
            next_writer_ = writers_queue_.begin();
        }
        else
        {
            next_writer_ = find(current_writer);
        }
    }

    void work_done()
    {
        assert(0 < writers_queue_.size());
        assert(writers_queue_.end() != next_writer_);
        set_next_writer();
    }

    iterator set_next_writer()
    {
        iterator next = std::next(next_writer_);
        next_writer_ = writers_queue_.end() == next ? writers_queue_.begin() : next;
        return next_writer_;
    }

    void add_new_sample(
            fastrtps::rtps::RTPSWriter* writer,
            fastrtps::rtps::CacheChange_t* change)
    {
        auto it = find(writer);
        assert(it != writers_queue_.end());
        std::get<1>(*it).add_new_sample(change);
    }

    void add_old_sample(
            fastrtps::rtps::RTPSWriter* writer,
            fastrtps::rtps::CacheChange_t* change)
    {
        auto it = find(writer);
        assert(it != writers_queue_.end());
        std::get<1>(*it).add_old_sample(change);
    }

    fastrtps::rtps::CacheChange_t* get_next_change_nts()
    {
        fastrtps::rtps::CacheChange_t* ret_change = nullptr;

        if (0 < writers_queue_.size())
        {
            auto starting_it = next_writer_;     // For avoid loops.

            do
            {
                ret_change = std::get<1>(*next_writer_).get_next_change();
            } while (nullptr == ret_change && starting_it != set_next_writer());
        }

        return ret_change;
    }

    void add_interested_changes_to_queue_nts()
    {
        // This function should be called with mutex_  and interested_lock locked, because the queue is changed.
        for (auto& queue : writers_queue_)
        {
            std::get<1>(queue).add_interested_changes_to_queue();
        }
    }

    void set_bandwith_limitation(
            uint32_t) const
    {
    }

    void trigger_bandwidth_limit_reset() const
    {
    }

private:

    iterator find(
            const fastrtps::rtps::RTPSWriter* writer)
    {
        return std::find_if(writers_queue_.begin(), writers_queue_.end(),
                       [writer](const element& current_writer) -> bool
                       {
                           return writer == std::get<0>(current_writer);
                       });
    }

    container writers_queue_;
    iterator next_writer_;

};

//! High priority scheduling
struct FlowControllerHighPrioritySchedule
{
    void register_writer(
            fastrtps::rtps::RTPSWriter* writer)
    {
        assert(nullptr != writer);
        int32_t priority = 10;
        auto property = fastrtps::rtps::PropertyPolicyHelper::find_property(
            writer->getAttributes().properties, "fastdds.sfc.priority");

        if (nullptr != property)
        {
            char* ptr = nullptr;
            priority = strtol(property->c_str(), &ptr, 10);

            if (property->c_str() != ptr)     // A valid integer was read.
            {
                if (-10 > priority || 10 < priority)
                {
                    priority = 10;
                    logError(RTPS_WRITER,
                            "Wrong value for fastdds.sfc.priority property. Range is [-10, 10]. Priority set to lowest (10)");
                }
            }
            else
            {
                priority = 10;
                logError(RTPS_WRITER,
                        "Not numerical value for fastdds.sfc.priority property. Priority set to lowest (10)");
            }
        }

        auto ret = priorities_.insert({writer, priority});
        (void)ret;
        assert(ret.second);

        // Ensure the priority is created.
        FlowQueue& queue = writers_queue_[priority];
        (void)queue;
    }

    void unregister_writer(
            fastrtps::rtps::RTPSWriter* writer)
    {
        auto it = priorities_.find(writer);
        assert(it != priorities_.end());
        priorities_.erase(it);
    }

    void work_done() const
    {
        // Do nothing
    }

    void add_new_sample(
            fastrtps::rtps::RTPSWriter* writer,
            fastrtps::rtps::CacheChange_t* change)
    {
        find_queue(writer).add_new_sample(change);
    }

    void add_old_sample(
            fastrtps::rtps::RTPSWriter* writer,
            fastrtps::rtps::CacheChange_t* change)
    {
        find_queue(writer).add_old_sample(change);
    }

    fastrtps::rtps::CacheChange_t* get_next_change_nts()
    {
        fastrtps::rtps::CacheChange_t* ret_change = nullptr;

        if (0 < writers_queue_.size())
        {
            for (auto it = writers_queue_.begin(); nullptr == ret_change && it != writers_queue_.end(); ++it)
            {
                ret_change = it->second.get_next_change();
            }
        }

        return ret_change;
    }

    void add_interested_changes_to_queue_nts()
    {
        // This function should be called with mutex_  and interested_lock locked, because the queue is changed.
        for (auto& queue : writers_queue_)
        {
            queue.second.add_interested_changes_to_queue();
        }
    }

    void set_bandwith_limitation(
            uint32_t) const
    {
    }

    void trigger_bandwidth_limit_reset() const
    {
    }

private:

    FlowQueue& find_queue(
            fastrtps::rtps::RTPSWriter* writer)
    {
        // Find priority.
        auto priority_it = priorities_.find(writer);
        assert(priority_it != priorities_.end());
        auto queue_it = writers_queue_.find(priority_it->second);
        assert(queue_it != writers_queue_.end());
        return queue_it->second;
    }

    std::map<int32_t, FlowQueue> writers_queue_;

    std::unordered_map<fastrtps::rtps::RTPSWriter*, int32_t> priorities_;
};

/////////////////////////////////////////////////////////////////////////////////////
//! TSDSIR scheduling
/**
 * writer/msg 优先级取址范围[-10, 10], -10为最大优先级，10为最小优先级
 *
*/
struct FlowControllerTSDSIRSchedule
{
public:
    FlowControllerTSDSIRSchedule()
    {

    }

    void register_writer(
        fastrtps::rtps::RTPSWriter* writer)
    {
        assert(nullptr != writer);
        int32_t priority = 10;
        auto property = fastrtps::rtps::PropertyPolicyHelper::find_property(
            writer->getAttributes().properties, "fastdds.sfc.priority");

        if (nullptr != property)
        {
            char* ptr = nullptr;
            priority = strtol(property->c_str(), &ptr, 10);

            if (property->c_str() != ptr)     // A valid integer was read.
            {
                if (-10 > priority || 10 < priority)
                {
                    priority = 10;
                    logError(RTPS_WRITER,
                            "Wrong value for fastdds.sfc.priority property. Range is [-10, 10]. Priority set to lowest (10)");
                }
            }
            else
            {
                priority = 10;
                logError(RTPS_WRITER,
                        "Not numerical value for fastdds.sfc.priority property. Priority set to lowest (10)");
            }
        }

        auto ret = priorities_.insert({writer, priority});
        (void)ret;
        assert(ret.second);

        // Ensure the priority is created.
        FlowQueue& queue = writers_queue_[priority];
        (void)queue;
    }


    void unregister_writer(fastrtps::rtps::RTPSWriter* writer)
    {
        auto it = priorities_.find(writer);
        assert(it != priorities_.end());
        priorities_.erase(it);
    }

    void work_done() const
    {
        // Do nothing
    }

    // TODO*******************执行顺序①
    void  add_new_sample(fastrtps::rtps::RTPSWriter* writer, fastrtps::rtps::CacheChange_t* change)
    {
        // change->sourceTimestamp; --> Tstart
        // 不同writer发送的不同类型的消息具有不同最大传输延时 Tmax（具体根据该消息相关功能应用实时性需求来确定）
        assert(nullptr != writer);
        int32_t priority = 10;
        auto property = fastrtps::rtps::PropertyPolicyHelper::find_property(
            writer->getAttributes().properties, "fastdds.sfc.priority");
        if (nullptr != property)
        {
            char* ptr = nullptr;
            priority = strtol(property->c_str(), &ptr, 10);
            if (property->c_str() != ptr)     // A valid integer was read.
            {
                if (-10 > priority || 10 < priority)
                {
                    priority = 10;
                    logError(RTPS_WRITER,
                            "Wrong value for fastdds.sfc.priority property. Range is [-10, 10]. Priority set to lowest (10)");
                }
            }
            else
            {
                priority = 10;
                logError(RTPS_WRITER,
                        "Not numerical value for fastdds.sfc.priority property. Priority set to lowest (10)");
            }
        }
        // int priority = writer->get_priority();
        std::cout << "+++add_new_sample: " << priority
            << "优先级的writer向其FlowQueue中添加change seq_num: " << change->sequenceNumber << std::endl;
        highest_priority = 0;   // 记录所传消息中最高的优先级
        switch (priority)
        {
        case 6:
            // TODO: Time_t::Time_t(long double sec)
            change->maxTransportTime = eprosima::fastrtps::rtps::Time_t(0.040); // 36ms
            change->priority = 6;
            break;
        case 4:
            change->maxTransportTime = eprosima::fastrtps::rtps::Time_t(0.038);
            change->priority = 4;
            break;
        case 2:
            change->maxTransportTime = eprosima::fastrtps::rtps::Time_t(0.032);
            change->priority = 2;
            break;
        case 0:
            change->maxTransportTime = eprosima::fastrtps::rtps::Time_t(0.030);
            change->priority = 0;
            break;
        default:
            break;
        }
        find_queue(writer).add_new_sample(change);
    }

    void add_old_sample(
            fastrtps::rtps::RTPSWriter* writer,
            fastrtps::rtps::CacheChange_t* change)
    {
        find_queue(writer).add_old_sample(change);
    }

    // TODO*******************执行顺序③
    fastrtps::rtps::CacheChange_t* get_next_change_nts()
    {
        /*
            TODO：
            优先级优先队列已经通过writers_queue_排序好；
            仅需对当前所有的change按剩余时间排序即可，放入queue_中（仅保存排好序队列的top和bottom指针）
        */
        queue_.clear_time();    // 清空queue队列
        if (0 < writers_queue_.size())
        {
            // 优先获取并发送优先级较大的writer的消息，int越小对应writer优先级越大
            for (auto it = writers_queue_.begin(); it != writers_queue_.end(); ++it)
            {
                it->second.add_into_time_queue(queue_); // 遍历second（flowqueue）中元素，添加到时间优先队列
            }
        }
        // std::cout << "当前时间优先队列长度: " << queue_.get_time_size() << std::endl;
        // queue_.print_time_size();
        
        judge_switch(); // 判断是否切换子系统
        /*
            TODO: 根据队列元素判断是否切换子系统/切换优先队列
            T, Tmax, Tmin通过计算所得，Tmax和Tmin 与当前的RTT有关，即当时网络延迟
            时间优先->优先级优先：false
                (存在离散数据包/存在最高优先级的数据 && 平均已传输时间T<Tmax) || (T<Tmin)
            优先级优先->时间优先：true
                存在即将超时的消息，平均已传输时间T>Tmax
        */
        fastrtps::rtps::CacheChange_t* ret_change = nullptr;
        if (is_switch)  // 从时间优先队列中输出队首元素
        {
            std::cout << "时间优先队列输出" << std::endl;
            ret_change = queue_.get_down_change();
        }
        else // 从优先级优先队列中输出队首元素
        {
            std::cout << "优先级优先队列输出" << std::endl;
            if (0 < writers_queue_.size())
            {
                // 优先获取并发送优先级较大的writer的消息，int越小对应writer优先级越大
                for (auto it = writers_queue_.begin(); nullptr == ret_change && it != writers_queue_.end(); ++it)
                {
                    ret_change = it->second.get_next_change();
                }
            }
        }
        return ret_change;
    }

    // TODO*******************执行顺序②
    void add_interested_changes_to_queue_nts()
    {
        // This function should be called with mutex_  and interested_lock locked, because the queue is changed.
        for (auto& queue : writers_queue_)
        {
            queue.second.add_interested_changes_to_queue();
        }
    }

    void switchQueue()
    {
        is_switch = !is_switch;
    }

    void calc_t_max_min()
    {
        fastrtps::rtps::CacheChange_t* ret_change = nullptr;
        long double rtt = 0, tmp = 0;
        t_max = 0, t_min = 0;
        if (0 < writers_queue_.size())
        {
            for (auto it = writers_queue_.begin(); it != writers_queue_.end(); ++it)
            {
                ret_change = it->second.get_next_change();
                if (ret_change != nullptr)
                {
                    tmp = (1.0 * ret_change->maxTransportTime.to_ns() - rtt / 2) 
                            * (1.0 * it->second.get_size() / queue_.get_time_size());
                    t_max += r_max * tmp;
                    t_min += r_min * tmp;
                }
            }
        }
    }

    void judge_switch()
    {
        ave_trans_time = queue_.calc_ave_trans_time();
        calc_t_max_min();
        std::cout << "当前待发送队列的平均已传输时间为: " << ave_trans_time << std::endl;
        std::cout << "当前待发送队列的t_max: " << t_max << ", t_min: " << t_min << std::endl;
        if (is_switch) 
        {
            // 当前是时间优先子系统，判断是否切换为优先级优先子系统
            auto discrete_ = writers_queue_.find(highest_priority);
            if (discrete_ != writers_queue_.end())
            {
                // is_switch = discrete_->second.is_empty() ? is_switch : !is_switch;
                if (!discrete_->second.is_empty())
                {
                    is_switch = !is_switch;
                    std::cout << "### 时间优先子系统 ---> 优先级优先子系统 ###" << std::endl;
                }
            }
        }
        else
        {
            // 当前是优先级优先子系统，判断是否切换为时间优先子系统
	    if (ave_trans_time > t_max)
            {
                is_switch = !is_switch;
                std::cout << "*** 优先级优先子系统 ---> 时间优先子系统 ***" << std::endl;
		return;
            }

            auto discrete_ = writers_queue_.find(highest_priority);
            if (discrete_ != writers_queue_.end())
            {
                // is_switch = discrete_->second.is_empty() ? is_switch : !is_switch;
                if (discrete_->second.is_empty())
                {
                    is_switch = !is_switch;
                    std::cout << "*** 优先级优先子系统 ---> 时间优先子系统 ***" << std::endl;
                }
            }
            else
            {
                is_switch = !is_switch;
                std::cout << "*** 优先级优先子系统 ---> 时间优先子系统 ***" << std::endl;
            }
            
        }
        // return false;
    }

    void set_bandwith_limitation(
            uint32_t) const
    {
    }

    void trigger_bandwidth_limit_reset() const
    {
    }

private:

    FlowQueue& find_queue(fastrtps::rtps::RTPSWriter* writer)
    {
        // Find priority.
        auto priority_it = priorities_.find(writer);
        assert(priority_it != priorities_.end());
        auto queue_it = writers_queue_.find(priority_it->second);
        assert(queue_it != writers_queue_.end());
        return queue_it->second;
    }

    // 消息优先级优先队列（不同优先级的writer与其发送的不同优先级的消息相关联），自动按Key升序排序，从较小值（较大优先级）获取
    std::map<int, FlowQueue> writers_queue_;
    std::unordered_map<fastrtps::rtps::RTPSWriter*, int> priorities_;

    // 消息时间优先队列（消息的等待时间Twait=Tnow-Tstart，剩余等待时间Tremain=Tmax-Twait），按照消息的剩余等待时间(急迫程度)升序
    FlowQueue queue_;

    bool is_switch = false;
    long double ave_trans_time = 0.0;
    long double t_max = 0.0;
    long double t_min = 0.0;
    int highest_priority = -10;
    const double r_max = 0.8;
    const double r_min = 0.2;
};

/////////////////////////////////////////////////////////////////////////////////////


//! Priority with reservation scheduling
struct FlowControllerPriorityWithReservationSchedule
{
    void register_writer(
            fastrtps::rtps::RTPSWriter* writer)
    {
        assert(nullptr != writer);
        int32_t priority = 10;
        auto property = fastrtps::rtps::PropertyPolicyHelper::find_property(
            writer->getAttributes().properties, "fastdds.sfc.priority");

        if (nullptr != property)
        {
            char* ptr = nullptr;
            priority = strtol(property->c_str(), &ptr, 10);

            if (property->c_str() != ptr)     // A valid integer was read.
            {
                if (-10 > priority || 10 < priority)
                {
                    priority = 10;
                    logError(RTPS_WRITER,
                            "Wrong value for fastdds.sfc.priority property. Range is [-10, 10]. Priority set to lowest (10)");
                }
            }
            else
            {
                priority = 10;
                logError(RTPS_WRITER,
                        "Not numerical value for fastdds.sfc.priority property. Priority set to lowest (10)");
            }
        }

        uint32_t reservation = 0;
        property = fastrtps::rtps::PropertyPolicyHelper::find_property(
            writer->getAttributes().properties, "fastdds.sfc.bandwidth_reservation");

        if (nullptr != property)
        {
            char* ptr = nullptr;
            reservation = strtoul(property->c_str(), &ptr, 10);

            if (property->c_str() != ptr)     // A valid integer was read.
            {
                if (100 < reservation)
                {
                    reservation = 0;
                    logError(RTPS_WRITER,
                            "Wrong value for fastdds.sfc.bandwidth_reservation property. Range is [0, 100]. Reservation set to lowest (0)");
                }
            }
            else
            {
                reservation = 0;
                logError(RTPS_WRITER,
                        "Not numerical value for fastdds.sfc.bandwidth_reservation property. Reservation set to lowest (0)");
            }
        }

        // Calculate reservation in bytes.
        uint32_t reservation_bytes = (0 == bandwidth_limit_? 0 :
                ((bandwidth_limit_ * reservation) / 100));

        auto ret = writers_queue_.emplace(writer, std::make_tuple(FlowQueue(), priority, reservation_bytes, 0u));
        (void)ret;
        assert(ret.second);

        priorities_[priority].push_back(writer);
    }

    void unregister_writer(
            fastrtps::rtps::RTPSWriter* writer)
    {
        auto it = writers_queue_.find(writer);
        assert(it != writers_queue_.end());
        int32_t priority = std::get<1>(it->second);
        writers_queue_.erase(it);
        auto priority_it = priorities_.find(priority);
        assert(priority_it != priorities_.end());
        auto writer_it = std::find(priority_it->second.begin(), priority_it->second.end(), writer);
        assert(writer_it != priority_it->second.end());
        priority_it->second.erase(writer_it);
    }

    void work_done()
    {
        if (nullptr != writer_being_processed_)
        {
            assert(0 != size_being_processed_);
            auto writer = writers_queue_.find(writer_being_processed_);
            std::get<3>(writer->second) += size_being_processed_;
            writer_being_processed_ = nullptr;
            size_being_processed_ = 0;
        }
    }

    void add_new_sample(
            fastrtps::rtps::RTPSWriter* writer,
            fastrtps::rtps::CacheChange_t* change)
    {
        // Find writer queue..
        auto it = writers_queue_.find(writer);
        assert(it != writers_queue_.end());
        std::get<0>(it->second).add_new_sample(change);
    }

    void add_old_sample(
            fastrtps::rtps::RTPSWriter* writer,
            fastrtps::rtps::CacheChange_t* change)
    {
        // Find writer queue..
        auto it = writers_queue_.find(writer);
        assert(it != writers_queue_.end());
        std::get<0>(it->second).add_old_sample(change);
    }

    fastrtps::rtps::CacheChange_t* get_next_change_nts()
    {
        fastrtps::rtps::CacheChange_t* highest_priority = nullptr;
        fastrtps::rtps::CacheChange_t* ret_change = nullptr;

        if (0 < writers_queue_.size())
        {
            for (auto& priority : priorities_)
            {
                for (auto writer_it : priority.second)
                {
                    auto writer = writers_queue_.find(writer_it);
                    fastrtps::rtps::CacheChange_t* change = std::get<0>(writer->second).get_next_change();

                    if (nullptr == highest_priority)
                    {
                        highest_priority = change;
                    }

                    if (nullptr != change)
                    {
                        // Check if writer's next change can be processed because the writer's bandwidth reservation is
                        // enough.
                        uint32_t size_to_check = change->serializedPayload.length;
                        if (0 != change->getFragmentCount())
                        {
                            size_to_check = change->getFragmentSize();
                        }

                        if (std::get<2>(writer->second) > (std::get<3>(writer->second) + size_to_check))
                        {
                            ret_change = change;
                            writer_being_processed_ = writer_it;
                            size_being_processed_ = size_to_check;
                            break;
                        }
                    }
                }

                if (nullptr != ret_change)
                {
                    break;
                }
            }
        }

        return (nullptr != ret_change ? ret_change : highest_priority);
    }

    void add_interested_changes_to_queue_nts()
    {
        // This function should be called with mutex_  and interested_lock locked, because the queue is changed.
        for (auto& queue : writers_queue_)
        {
            std::get<0>(queue.second).add_interested_changes_to_queue();
        }
    }

    void set_bandwith_limitation(
            uint32_t limit)
    {
        bandwidth_limit_ = limit;
    }

    void trigger_bandwidth_limit_reset()
    {
        for (auto& writer : writers_queue_)
        {
            std::get<3>(writer.second) = 0;
        }
    }

private:

    using map_writers = std::unordered_map<fastrtps::rtps::RTPSWriter*, std::tuple<FlowQueue, int32_t, uint32_t,
                    uint32_t>>;

    using map_priorities = std::map<int32_t, std::vector<fastrtps::rtps::RTPSWriter*>>;

    map_writers writers_queue_;

    map_priorities priorities_;

    uint32_t bandwidth_limit_ = 0;

    fastrtps::rtps::RTPSWriter* writer_being_processed_ = nullptr;

    uint32_t size_being_processed_ = 0;
};

template<typename PublishMode, typename SampleScheduling>
class FlowControllerImpl : public FlowController
{
    using publish_mode = PublishMode;
    using scheduler = SampleScheduling;

public:

    FlowControllerImpl(
            fastrtps::rtps::RTPSParticipantImpl* participant,
            const FlowControllerDescriptor* descriptor
            )
        : participant_(participant)
        , async_mode(participant, descriptor)
    {
        uint32_t limitation = get_max_payload();

        if (std::numeric_limits<uint32_t>::max() != limitation)
        {
            sched.set_bandwith_limitation(limitation);
        }
    }

    virtual ~FlowControllerImpl() noexcept
    {
    }

    /*!
     * Initializes the flow controller.
     */
    void init() override
    {
        initialize_async_thread();
    }

    /*!
     * Registers a writer.
     * This object is only be able to manage a CacheChante_t if its writer was registered previously with this function.
     *
     * @param writer Pointer to the writer to be registered. Cannot be nullptr.
     */
    void register_writer(
            fastrtps::rtps::RTPSWriter* writer) override
    {
        std::unique_lock<std::mutex> lock(mutex_);
        auto ret = writers_.insert({ writer->getGuid(), writer});
        (void)ret;
        assert(ret.second);
        register_writer_impl(writer);
    }

    /*!
     * Unregister a writer.
     *
     * @param writer Pointer to the writer to be unregistered. Cannot be nullptr.
     */
    void unregister_writer(
            fastrtps::rtps::RTPSWriter* writer) override
    {
        std::unique_lock<std::mutex> lock(mutex_);
        writers_.erase(writer->getGuid());
        unregister_writer_impl(writer);
    }

    /*
     * Adds the CacheChange_t to be managed by this object.
     * The CacheChange_t has to be a new one, that is, it has to be added to the writer's history before this call.
     * This function should be called by RTPSWriter::unsent_change_added_to_history().
     * This function has two specializations depending on template parameter PublishMode.
     *
     * @param Pointer to the writer which the added CacheChante_t is responsable. Cannot be nullptr.
     * @param change Pointer to the new CacheChange_t to be managed by this object. Cannot be nullptr.
     * @return true if sample could be added. false in other case.
     */
    bool add_new_sample(
            fastrtps::rtps::RTPSWriter* writer,
            fastrtps::rtps::CacheChange_t* change,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time) override
    {
        return add_new_sample_impl(writer, change, max_blocking_time);
    }

    /*!
     * Adds the CacheChante_t to be managed by this object.
     * The CacheChange_t has to be an old one, that is, it is already in the writer's history and for some reason has to
     * be sent again.
     *
     * @param Pointer to the writer which the added change is responsable. Cannot be nullptr.
     * @param change Pointer to the old change to be managed by this object. Cannot be nullptr.
     * @return true if sample could be added. false in other case.
     */
    bool add_old_sample(
            fastrtps::rtps::RTPSWriter* writer,
            fastrtps::rtps::CacheChange_t* change) override
    {
        return add_old_sample_impl(writer, change,
                       std::chrono::steady_clock::now() + std::chrono::hours(24));
    }

    /*!
     * If currently the CacheChange_t is managed by this object, remove it.
     * This funcion should be called when a CacheChange_t is removed from the writer's history.
     *
     * @param Pointer to the change which should be removed if it is currently managed by this object.
     */
    void remove_change(
            fastrtps::rtps::CacheChange_t* change) override
    {
        assert(nullptr != change);
        remove_change_impl(change);
    }

    uint32_t get_max_payload() override
    {
        return get_max_payload_impl();
    }

private:

    /*!
     * Initialize asynchronous thread.
     */
    template<typename PubMode = PublishMode>
    typename std::enable_if<!std::is_same<FlowControllerPureSyncPublishMode, PubMode>::value, void>::type
    initialize_async_thread()
    {
        bool expected = false;
        if (async_mode.running.compare_exchange_strong(expected, true))
        {
            // Code for initializing the asynchronous thread.
            async_mode.thread = std::thread(&FlowControllerImpl::run, this);
        }
    }

    /*! This function is used when PublishMode = FlowControllerPureSyncPublishMode.
     *  In this case the async thread doesn't need to be initialized.
     */
    template<typename PubMode = PublishMode>
    typename std::enable_if<std::is_same<FlowControllerPureSyncPublishMode, PubMode>::value, void>::type
    initialize_async_thread()
    {
        // Do nothing.
    }

    template<typename PubMode = PublishMode>
    typename std::enable_if<!std::is_same<FlowControllerPureSyncPublishMode, PubMode>::value, void>::type
    register_writer_impl(
            fastrtps::rtps::RTPSWriter* writer)
    {
        std::unique_lock<std::mutex> in_lock(async_mode.changes_interested_mutex);
        sched.register_writer(writer);
    }

    template<typename PubMode = PublishMode>
    typename std::enable_if<std::is_same<FlowControllerPureSyncPublishMode, PubMode>::value, void>::type
    register_writer_impl(
            fastrtps::rtps::RTPSWriter*)
    {
        // Do nothing.
    }

    template<typename PubMode = PublishMode>
    typename std::enable_if<!std::is_same<FlowControllerPureSyncPublishMode, PubMode>::value, void>::type
    unregister_writer_impl(
            fastrtps::rtps::RTPSWriter* writer)
    {
        std::unique_lock<std::mutex> in_lock(async_mode.changes_interested_mutex);
        sched.unregister_writer(writer);
    }

    template<typename PubMode = PublishMode>
    typename std::enable_if<std::is_same<FlowControllerPureSyncPublishMode, PubMode>::value, void>::type
    unregister_writer_impl(
            fastrtps::rtps::RTPSWriter*)
    {
        // Do nothing.
    }

    /*!
     * This function store internally the sample and wake up the async thread.
     *
     * @note Before calling this function, the change's writer mutex have to be locked.
     */
    template<typename PubMode = PublishMode>
    typename std::enable_if<!std::is_same<FlowControllerPureSyncPublishMode, PubMode>::value, bool>::type
    enqueue_new_sample_impl(
            fastrtps::rtps::RTPSWriter* writer,
            fastrtps::rtps::CacheChange_t* change,
            const std::chrono::time_point<std::chrono::steady_clock>& /* TODO max_blocking_time*/)
    {
        assert(nullptr == change->writer_info.previous &&
                nullptr == change->writer_info.next);
        // Sync delivery failed. Try to store for asynchronous delivery.
        std::unique_lock<std::mutex> lock(async_mode.changes_interested_mutex);
        sched.add_new_sample(writer, change);
        std::cout << "==FlowControllerImpl<...>::enqueue_new_sample_impl(): 向FlowQueue(new_interested_)中添加change==" << std::endl;
        // ++call_num_;
        // if (call_num_ >= 10){
        //     call_num_ = 0;
        //     async_mode.cv.notify_one();
        //     std::cout << "==FlowControllerImpl<...>::enqueue_new_sample_impl(): 通知异步线程已经向FlowQueue中添加change" << std::endl;
        // }
        async_mode.cv.notify_one();
        std::cout << "==FlowControllerImpl<...>::enqueue_new_sample_impl(): 通知异步线程已经向FlowQueue中添加change" << std::endl;

        return true;
    }

    /*! This function is used when PublishMode = FlowControllerPureSyncPublishMode.
     *  In this case there is no async mechanism.
     */
    template<typename PubMode = PublishMode>
    typename std::enable_if<std::is_same<FlowControllerPureSyncPublishMode, PubMode>::value, bool>::type
    constexpr enqueue_new_sample_impl(
            fastrtps::rtps::RTPSWriter*,
            fastrtps::rtps::CacheChange_t*,
            const std::chrono::time_point<std::chrono::steady_clock>&) const
    {
        // Do nothing. Return false.
        return false;
    }

    /*!
     * This function tries to send the sample synchronously.
     * That is, it uses the user's thread, which is the one calling this function, to send the sample.
     * It calls new function `RTPSWriter::deliver_sample_nts()` for sending the sample.
     * If this function fails (for example because non-blocking socket is full), this function stores internally the sample to
     * try sending it again asynchronously.
     */
    template<typename PubMode = PublishMode>
    typename std::enable_if<std::is_base_of<FlowControllerPureSyncPublishMode, PubMode>::value, bool>::type
    add_new_sample_impl(
            fastrtps::rtps::RTPSWriter* writer,
            fastrtps::rtps::CacheChange_t* change,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time)
    {
        // This call should be made with writer's mutex locked.
        fastrtps::rtps::LocatorSelectorSender& locator_selector = writer->get_general_locator_selector();
        std::lock_guard<fastrtps::rtps::LocatorSelectorSender> lock(locator_selector);
        fastrtps::rtps::RTPSMessageGroup group(participant_, writer, &locator_selector);
        if (fastrtps::rtps::DeliveryRetCode::DELIVERED !=
                writer->deliver_sample_nts(change, group, locator_selector, max_blocking_time))
        {
            return enqueue_new_sample_impl(writer, change, max_blocking_time);
        }

        return true;
    }

    /*!
     * This function stores internally the sample to send it asynchronously.
     */
    template<typename PubMode = PublishMode>
    typename std::enable_if<!std::is_base_of<FlowControllerPureSyncPublishMode, PubMode>::value, bool>::type
    add_new_sample_impl(
            fastrtps::rtps::RTPSWriter* writer,
            fastrtps::rtps::CacheChange_t* change,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time)
    {
        return enqueue_new_sample_impl(writer, change, max_blocking_time);
    }

    /*!
     * This function store internally the sample and wake up the async thread.
     *
     * @note Before calling this function, the change's writer mutex have to be locked.
     */
    template<typename PubMode = PublishMode>
    typename std::enable_if<!std::is_same<FlowControllerPureSyncPublishMode, PubMode>::value, bool>::type
    add_old_sample_impl(
            fastrtps::rtps::RTPSWriter* writer,
            fastrtps::rtps::CacheChange_t* change,
            const std::chrono::time_point<std::chrono::steady_clock>& /* TODO max_blocking_time*/)
    {
        // This comparison is thread-safe, because we ensure the change to a problematic state is always protected for
        // its writer's mutex.
        // Problematic states:
        // - Being added: change both pointers from nullptr to a pointer values.
        // - Being removed: change both pointer from pointer values to nullptr.
        if (nullptr == change->writer_info.previous &&
                nullptr == change->writer_info.next)
        {
            std::unique_lock<std::mutex> lock(async_mode.changes_interested_mutex);
            sched.add_old_sample(writer, change);
            async_mode.cv.notify_one();

            return true;
        }

        return false;
    }

    /*! This function is used when PublishMode = FlowControllerPureSyncPublishMode.
     *  In this case there is no async mechanism.
     */
    template<typename PubMode = PublishMode>
    typename std::enable_if<std::is_same<FlowControllerPureSyncPublishMode, PubMode>::value, bool>::type
    constexpr add_old_sample_impl(
            fastrtps::rtps::RTPSWriter*,
            fastrtps::rtps::CacheChange_t*,
            const std::chrono::time_point<std::chrono::steady_clock>&) const
    {
        return false;
    }

    /*!
     * This function store internally the sample and wake up the async thread.
     *
     * @note Before calling this function, the change's writer mutex have to be locked.
     */
    template<typename PubMode = PublishMode>
    typename std::enable_if<!std::is_same<FlowControllerPureSyncPublishMode, PubMode>::value, void>::type
    remove_change_impl(
            fastrtps::rtps::CacheChange_t* change)
    {
        // This comparison is thread-safe, because we ensure the change to a problematic state is always protected for
        // its writer's mutex.
        // Problematic states:
        // - Being added: change both pointers from nullptr to a pointer values.
        // - Being removed: change both pointer from pointer values to nullptr.
        if (nullptr != change->writer_info.previous ||
                nullptr != change->writer_info.next)
        {
            ++async_mode.writers_interested_in_remove;
            std::unique_lock<std::mutex> lock(mutex_);
            std::unique_lock<std::mutex> interested_lock(async_mode.changes_interested_mutex);

            // When blocked, both pointer are different than nullptr or equal.
            assert((nullptr != change->writer_info.previous &&
                    nullptr != change->writer_info.next) ||
                    (nullptr == change->writer_info.previous &&
                    nullptr == change->writer_info.next));
            if (nullptr != change->writer_info.previous &&
                    nullptr != change->writer_info.next)
            {

                // Try to join previous node and next node.
                change->writer_info.previous->writer_info.next = change->writer_info.next;
                change->writer_info.next->writer_info.previous = change->writer_info.previous;
                change->writer_info.previous = nullptr;
                change->writer_info.next = nullptr;
            }
            --async_mode.writers_interested_in_remove;
        }
    }

    /*! This function is used when PublishMode = FlowControllerPureSyncPublishMode.
     *  In this case there is no async mechanism.
     */
    template<typename PubMode = PublishMode>
    typename std::enable_if<std::is_same<FlowControllerPureSyncPublishMode, PubMode>::value, void>::type
    remove_change_impl(
            fastrtps::rtps::CacheChange_t*) const
    {
        // Do nothing.
    }

    /*!
     * Function run by the asynchronous thread.
     */
    void run()
    {
        std::cout << "==SubThread-FlowControllerImpl<PublishMode, SampleScheduling>::run(): 异步发送线程启动==" << std::endl;
        while (async_mode.running)
        {
            // There are writers interested in removing a sample.
            if (0 != async_mode.writers_interested_in_remove)
            {
                continue;
            }

            std::unique_lock<std::mutex> lock(mutex_);
            fastrtps::rtps::CacheChange_t* change_to_process = nullptr;

            //Check if we have to sleep.
            {
                std::unique_lock<std::mutex> in_lock(async_mode.changes_interested_mutex);
                // Add interested changes into the queue.
                sched.add_interested_changes_to_queue_nts();
                std::cout << "==SubThread-Add interested changes into the queue.将interested_添加到ones中==" << std::endl;

                while (async_mode.running &&
                        (async_mode.force_wait() || nullptr == (change_to_process = sched.get_next_change_nts())))
                {
                    // Release main mutex to allow registering/unregistering writers while this thread is waiting.
                    lock.unlock();
                    bool ret = async_mode.wait(in_lock);

                    in_lock.unlock();
                    lock.lock();
                    in_lock.lock();

                    if (ret)
                    {
                        sched.trigger_bandwidth_limit_reset();
                    }
                    sched.add_interested_changes_to_queue_nts();
                }
            }

            fastrtps::rtps::RTPSWriter* current_writer = nullptr;
            while (nullptr != change_to_process)
            {
                std::cout << "==SubThread-FlowControllerImpl<, >::run(): 取出的数据的序列号：" << change_to_process->sequenceNumber << "==" << std::endl;
                // Fast check if next change will enter.
                if (!async_mode.fast_check_is_there_slot_for_change(change_to_process)) // 当前传输超过max_bytes_per_period,则丢弃period内的change
                {
                    std::cout << "==SubThread***当前发送带宽超过max_bytes_per_period, 移除数据的序号为: " << change_to_process->sequenceNumber << std::endl; 
                    break;
                }

                if (nullptr == current_writer || current_writer->getGuid() != change_to_process->writerGUID)
                {
                    auto writer_it = writers_.find(change_to_process->writerGUID);
                    assert(writers_.end() != writer_it);

                    current_writer = writer_it->second;
                }

                if (!current_writer->getMutex().try_lock())
                {
                    break;
                }

                fastrtps::rtps::LocatorSelectorSender& locator_selector =
                        current_writer->get_async_locator_selector();
                async_mode.group.sender(current_writer, &locator_selector);
                locator_selector.lock();

                // Remove previously from queue, because deliver_sample_nts could call FlowController::remove_sample()
                // provoking a deadlock.
                fastrtps::rtps::CacheChange_t* previous = change_to_process->writer_info.previous;
                fastrtps::rtps::CacheChange_t* next = change_to_process->writer_info.next;
                previous->writer_info.next = next;
                next->writer_info.previous = previous;
                change_to_process->writer_info.previous = nullptr;
                change_to_process->writer_info.next = nullptr;
                //////////////////////////////////////////////////////////////////
                // todo：移除上指针和下指针
                fastrtps::rtps::CacheChange_t* top = nullptr;
                fastrtps::rtps::CacheChange_t* bottom = nullptr;
                if (change_to_process->writer_info.earlier != nullptr && change_to_process->writer_info.later != nullptr)
                {
                    top = change_to_process->writer_info.earlier;
                    bottom = change_to_process->writer_info.later;
                    top->writer_info.later = bottom;
                    bottom->writer_info.earlier = top;
                    change_to_process->writer_info.earlier = nullptr;
                    change_to_process->writer_info.later = nullptr;
                }
                //////////////////////////////////////////////////////////////////

                fastrtps::rtps::DeliveryRetCode ret_delivery = current_writer->deliver_sample_nts(
                    change_to_process, async_mode.group, locator_selector,
                    std::chrono::steady_clock::now() + std::chrono::hours(24));
                // 大数据分段发送
                if (fastrtps::rtps::DeliveryRetCode::DELIVERED != ret_delivery)
                {
                    // If delivery fails, put the change again in the queue.
                    previous->writer_info.next = change_to_process;
                    next->writer_info.previous = change_to_process;
                    change_to_process->writer_info.previous = previous;
                    change_to_process->writer_info.next = next;
                    //////////////////////////////////////////////////////////////////
                    // todo：恢复上指针和下指针
                    if (top != nullptr && bottom != nullptr)
                    {
                        top->writer_info.later = change_to_process;
                        bottom->writer_info.earlier = change_to_process;
                        change_to_process->writer_info.earlier = top;
                        change_to_process->writer_info.later = bottom;
                    }
                    //////////////////////////////////////////////////////////////////
                    
                    async_mode.process_deliver_retcode(ret_delivery);
                    locator_selector.unlock();
                    current_writer->getMutex().unlock();
                    // Unlock mutex_ and try again.
                    break;
                }

                locator_selector.unlock();
                current_writer->getMutex().unlock();

                sched.work_done();
                std::cout << "==SubThread-流调度完毕==" << std::endl;

                if (0 != async_mode.writers_interested_in_remove)
                {
                    // There are writers that want to remove samples.
                    break;
                }

                // Add interested changes into the queue.
                {
                    std::unique_lock<std::mutex> in_lock(async_mode.changes_interested_mutex);
                    sched.add_interested_changes_to_queue_nts();
                }

                change_to_process = sched.get_next_change_nts();
                std::cout << "==SubThread-获取FlowQueue的下一个数据==" << std::endl;
                if (change_to_process == nullptr)
                {
                    std::cout << "==SubThread-获取失败，当前FlowQueue为空==" << std::endl;
                }
            }

            async_mode.group.sender(nullptr, nullptr);
        }
        std::cout << "==SubThread-FlowControllerImpl<PublishMode, SampleScheduling>::run(): 异步发送线程结束==" << std::endl;
    }

    template<typename PubMode = PublishMode>
    typename std::enable_if<std::is_base_of<FlowControllerLimitedAsyncPublishMode, PubMode>::value, uint32_t>::type
    get_max_payload_impl()
    {
        return static_cast<uint32_t>(async_mode.max_bytes_per_period);
    }

    template<typename PubMode = PublishMode>
    typename std::enable_if<!std::is_base_of<FlowControllerLimitedAsyncPublishMode, PubMode>::value, uint32_t>::type
    constexpr get_max_payload_impl() const
    {
        return std::numeric_limits<uint32_t>::max();
    }

    std::mutex mutex_;

    fastrtps::rtps::RTPSParticipantImpl* participant_ = nullptr;

    std::map<fastrtps::rtps::GUID_t, fastrtps::rtps::RTPSWriter*> writers_;

    scheduler sched;

    // async_mode must be destroyed before sched.
    publish_mode async_mode;

    int call_num_ = 0;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _RTPS_FLOWCONTROL_FLOWCONTROLLERIMPL_HPP_
