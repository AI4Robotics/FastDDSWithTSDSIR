// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/*!
 * @file MsgRGBImage.cpp
 * This source file contains the definition of the described types in the IDL file.
 *
 * This file was generated by the tool gen.
 */

#ifdef _WIN32
// Remove linker warning LNK4221 on Visual Studio
namespace {
char dummy;
}  // namespace
#endif  // _WIN32

#include "MsgRGBImage.h"
#include <fastcdr/Cdr.h>

#include <fastcdr/exceptions/BadParamException.h>
using namespace eprosima::fastcdr::exception;

#include <utility>

MsgRGBImage::MsgRGBImage()
{
    // m_msg_priority com.eprosima.idl.parser.typecode.PrimitiveTypeCode@7276c8cd
    m_msg_priority = 0;
    // m_msg_sequence_num com.eprosima.idl.parser.typecode.PrimitiveTypeCode@544a2ea6
    m_msg_sequence_num = 0;
    // m_msg_type com.eprosima.idl.parser.typecode.StringTypeCode@2e3fc542
    m_msg_type ="";
    // m_msg_start_seconds com.eprosima.idl.parser.typecode.PrimitiveTypeCode@150c158
    m_msg_start_seconds = 0;
    // m_msg_start_nanosec com.eprosima.idl.parser.typecode.PrimitiveTypeCode@10dba097
    m_msg_start_nanosec = 0;
    // m_msg_end_seconds com.eprosima.idl.parser.typecode.PrimitiveTypeCode@1786f9d5
    m_msg_end_seconds = 0;
    // m_msg_end_nanosec com.eprosima.idl.parser.typecode.PrimitiveTypeCode@704d6e83
    m_msg_end_nanosec = 0;
    // m_msg_trans_time com.eprosima.idl.parser.typecode.PrimitiveTypeCode@2eda0940
    m_msg_trans_time = 0;
    // m_max_trans_time com.eprosima.idl.parser.typecode.PrimitiveTypeCode@3578436e
    m_max_trans_time = 0;
    // m_is_sent com.eprosima.idl.parser.typecode.PrimitiveTypeCode@706a04ae
    m_is_sent = false;
    // m_is_overtime com.eprosima.idl.parser.typecode.PrimitiveTypeCode@6eceb130
    m_is_overtime = false;
    // m_message com.eprosima.idl.parser.typecode.ArrayTypeCode@10a035a0
    memset(&m_message, 0, (489600) * 1);

}

MsgRGBImage::~MsgRGBImage()
{












}

MsgRGBImage::MsgRGBImage(
        const MsgRGBImage& x)
{
    m_msg_priority = x.m_msg_priority;
    m_msg_sequence_num = x.m_msg_sequence_num;
    m_msg_type = x.m_msg_type;
    m_msg_start_seconds = x.m_msg_start_seconds;
    m_msg_start_nanosec = x.m_msg_start_nanosec;
    m_msg_end_seconds = x.m_msg_end_seconds;
    m_msg_end_nanosec = x.m_msg_end_nanosec;
    m_msg_trans_time = x.m_msg_trans_time;
    m_max_trans_time = x.m_max_trans_time;
    m_is_sent = x.m_is_sent;
    m_is_overtime = x.m_is_overtime;
    m_message = x.m_message;
}

MsgRGBImage::MsgRGBImage(
        MsgRGBImage&& x)
{
    m_msg_priority = x.m_msg_priority;
    m_msg_sequence_num = x.m_msg_sequence_num;
    m_msg_type = std::move(x.m_msg_type);
    m_msg_start_seconds = x.m_msg_start_seconds;
    m_msg_start_nanosec = x.m_msg_start_nanosec;
    m_msg_end_seconds = x.m_msg_end_seconds;
    m_msg_end_nanosec = x.m_msg_end_nanosec;
    m_msg_trans_time = x.m_msg_trans_time;
    m_max_trans_time = x.m_max_trans_time;
    m_is_sent = x.m_is_sent;
    m_is_overtime = x.m_is_overtime;
    m_message = std::move(x.m_message);
}

MsgRGBImage& MsgRGBImage::operator =(
        const MsgRGBImage& x)
{

    m_msg_priority = x.m_msg_priority;
    m_msg_sequence_num = x.m_msg_sequence_num;
    m_msg_type = x.m_msg_type;
    m_msg_start_seconds = x.m_msg_start_seconds;
    m_msg_start_nanosec = x.m_msg_start_nanosec;
    m_msg_end_seconds = x.m_msg_end_seconds;
    m_msg_end_nanosec = x.m_msg_end_nanosec;
    m_msg_trans_time = x.m_msg_trans_time;
    m_max_trans_time = x.m_max_trans_time;
    m_is_sent = x.m_is_sent;
    m_is_overtime = x.m_is_overtime;
    m_message = x.m_message;

    return *this;
}

MsgRGBImage& MsgRGBImage::operator =(
        MsgRGBImage&& x)
{

    m_msg_priority = x.m_msg_priority;
    m_msg_sequence_num = x.m_msg_sequence_num;
    m_msg_type = std::move(x.m_msg_type);
    m_msg_start_seconds = x.m_msg_start_seconds;
    m_msg_start_nanosec = x.m_msg_start_nanosec;
    m_msg_end_seconds = x.m_msg_end_seconds;
    m_msg_end_nanosec = x.m_msg_end_nanosec;
    m_msg_trans_time = x.m_msg_trans_time;
    m_max_trans_time = x.m_max_trans_time;
    m_is_sent = x.m_is_sent;
    m_is_overtime = x.m_is_overtime;
    m_message = std::move(x.m_message);

    return *this;
}

bool MsgRGBImage::operator ==(
        const MsgRGBImage& x) const
{

    return (m_msg_priority == x.m_msg_priority && m_msg_sequence_num == x.m_msg_sequence_num && m_msg_type == x.m_msg_type && m_msg_start_seconds == x.m_msg_start_seconds && m_msg_start_nanosec == x.m_msg_start_nanosec && m_msg_end_seconds == x.m_msg_end_seconds && m_msg_end_nanosec == x.m_msg_end_nanosec && m_msg_trans_time == x.m_msg_trans_time && m_max_trans_time == x.m_max_trans_time && m_is_sent == x.m_is_sent && m_is_overtime == x.m_is_overtime && m_message == x.m_message);
}

bool MsgRGBImage::operator !=(
        const MsgRGBImage& x) const
{
    return !(*this == x);
}

size_t MsgRGBImage::getMaxCdrSerializedSize(
        size_t current_alignment)
{
    size_t initial_alignment = current_alignment;


    current_alignment += 2 + eprosima::fastcdr::Cdr::alignment(current_alignment, 2);


    current_alignment += 8 + eprosima::fastcdr::Cdr::alignment(current_alignment, 8);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + 255 + 1;

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 1 + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);


    current_alignment += 1 + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);


    current_alignment += ((489600) * 1) + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);



    return current_alignment - initial_alignment;
}

size_t MsgRGBImage::getCdrSerializedSize(
        const MsgRGBImage& data,
        size_t current_alignment)
{
    (void)data;
    size_t initial_alignment = current_alignment;


    current_alignment += 2 + eprosima::fastcdr::Cdr::alignment(current_alignment, 2);


    current_alignment += 8 + eprosima::fastcdr::Cdr::alignment(current_alignment, 8);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + data.msg_type().size() + 1;

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 1 + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);


    current_alignment += 1 + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);


    if ((489600) > 0)
    {
        current_alignment += ((489600) * 1) + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);
    }


    return current_alignment - initial_alignment;
}

void MsgRGBImage::serialize(
        eprosima::fastcdr::Cdr& scdr) const
{

    scdr << m_msg_priority;
    scdr << m_msg_sequence_num;
    scdr << m_msg_type;
    scdr << m_msg_start_seconds;
    scdr << m_msg_start_nanosec;
    scdr << m_msg_end_seconds;
    scdr << m_msg_end_nanosec;
    scdr << m_msg_trans_time;
    scdr << m_max_trans_time;
    scdr << m_is_sent;
    scdr << m_is_overtime;
    scdr << m_message;


}

void MsgRGBImage::deserialize(
        eprosima::fastcdr::Cdr& dcdr)
{

    dcdr >> m_msg_priority;
    dcdr >> m_msg_sequence_num;
    dcdr >> m_msg_type;
    dcdr >> m_msg_start_seconds;
    dcdr >> m_msg_start_nanosec;
    dcdr >> m_msg_end_seconds;
    dcdr >> m_msg_end_nanosec;
    dcdr >> m_msg_trans_time;
    dcdr >> m_max_trans_time;
    dcdr >> m_is_sent;
    dcdr >> m_is_overtime;
    dcdr >> m_message;

}

/*!
 * @brief This function sets a value in member msg_priority
 * @param _msg_priority New value for member msg_priority
 */
void MsgRGBImage::msg_priority(
        int16_t _msg_priority)
{
    m_msg_priority = _msg_priority;
}

/*!
 * @brief This function returns the value of member msg_priority
 * @return Value of member msg_priority
 */
int16_t MsgRGBImage::msg_priority() const
{
    return m_msg_priority;
}

/*!
 * @brief This function returns a reference to member msg_priority
 * @return Reference to member msg_priority
 */
int16_t& MsgRGBImage::msg_priority()
{
    return m_msg_priority;
}

/*!
 * @brief This function sets a value in member msg_sequence_num
 * @param _msg_sequence_num New value for member msg_sequence_num
 */
void MsgRGBImage::msg_sequence_num(
        uint64_t _msg_sequence_num)
{
    m_msg_sequence_num = _msg_sequence_num;
}

/*!
 * @brief This function returns the value of member msg_sequence_num
 * @return Value of member msg_sequence_num
 */
uint64_t MsgRGBImage::msg_sequence_num() const
{
    return m_msg_sequence_num;
}

/*!
 * @brief This function returns a reference to member msg_sequence_num
 * @return Reference to member msg_sequence_num
 */
uint64_t& MsgRGBImage::msg_sequence_num()
{
    return m_msg_sequence_num;
}

/*!
 * @brief This function copies the value in member msg_type
 * @param _msg_type New value to be copied in member msg_type
 */
void MsgRGBImage::msg_type(
        const std::string& _msg_type)
{
    m_msg_type = _msg_type;
}

/*!
 * @brief This function moves the value in member msg_type
 * @param _msg_type New value to be moved in member msg_type
 */
void MsgRGBImage::msg_type(
        std::string&& _msg_type)
{
    m_msg_type = std::move(_msg_type);
}

/*!
 * @brief This function returns a constant reference to member msg_type
 * @return Constant reference to member msg_type
 */
const std::string& MsgRGBImage::msg_type() const
{
    return m_msg_type;
}

/*!
 * @brief This function returns a reference to member msg_type
 * @return Reference to member msg_type
 */
std::string& MsgRGBImage::msg_type()
{
    return m_msg_type;
}
/*!
 * @brief This function sets a value in member msg_start_seconds
 * @param _msg_start_seconds New value for member msg_start_seconds
 */
void MsgRGBImage::msg_start_seconds(
        int32_t _msg_start_seconds)
{
    m_msg_start_seconds = _msg_start_seconds;
}

/*!
 * @brief This function returns the value of member msg_start_seconds
 * @return Value of member msg_start_seconds
 */
int32_t MsgRGBImage::msg_start_seconds() const
{
    return m_msg_start_seconds;
}

/*!
 * @brief This function returns a reference to member msg_start_seconds
 * @return Reference to member msg_start_seconds
 */
int32_t& MsgRGBImage::msg_start_seconds()
{
    return m_msg_start_seconds;
}

/*!
 * @brief This function sets a value in member msg_start_nanosec
 * @param _msg_start_nanosec New value for member msg_start_nanosec
 */
void MsgRGBImage::msg_start_nanosec(
        uint32_t _msg_start_nanosec)
{
    m_msg_start_nanosec = _msg_start_nanosec;
}

/*!
 * @brief This function returns the value of member msg_start_nanosec
 * @return Value of member msg_start_nanosec
 */
uint32_t MsgRGBImage::msg_start_nanosec() const
{
    return m_msg_start_nanosec;
}

/*!
 * @brief This function returns a reference to member msg_start_nanosec
 * @return Reference to member msg_start_nanosec
 */
uint32_t& MsgRGBImage::msg_start_nanosec()
{
    return m_msg_start_nanosec;
}

/*!
 * @brief This function sets a value in member msg_end_seconds
 * @param _msg_end_seconds New value for member msg_end_seconds
 */
void MsgRGBImage::msg_end_seconds(
        int32_t _msg_end_seconds)
{
    m_msg_end_seconds = _msg_end_seconds;
}

/*!
 * @brief This function returns the value of member msg_end_seconds
 * @return Value of member msg_end_seconds
 */
int32_t MsgRGBImage::msg_end_seconds() const
{
    return m_msg_end_seconds;
}

/*!
 * @brief This function returns a reference to member msg_end_seconds
 * @return Reference to member msg_end_seconds
 */
int32_t& MsgRGBImage::msg_end_seconds()
{
    return m_msg_end_seconds;
}

/*!
 * @brief This function sets a value in member msg_end_nanosec
 * @param _msg_end_nanosec New value for member msg_end_nanosec
 */
void MsgRGBImage::msg_end_nanosec(
        uint32_t _msg_end_nanosec)
{
    m_msg_end_nanosec = _msg_end_nanosec;
}

/*!
 * @brief This function returns the value of member msg_end_nanosec
 * @return Value of member msg_end_nanosec
 */
uint32_t MsgRGBImage::msg_end_nanosec() const
{
    return m_msg_end_nanosec;
}

/*!
 * @brief This function returns a reference to member msg_end_nanosec
 * @return Reference to member msg_end_nanosec
 */
uint32_t& MsgRGBImage::msg_end_nanosec()
{
    return m_msg_end_nanosec;
}

/*!
 * @brief This function sets a value in member msg_trans_time
 * @param _msg_trans_time New value for member msg_trans_time
 */
void MsgRGBImage::msg_trans_time(
        int32_t _msg_trans_time)
{
    m_msg_trans_time = _msg_trans_time;
}

/*!
 * @brief This function returns the value of member msg_trans_time
 * @return Value of member msg_trans_time
 */
int32_t MsgRGBImage::msg_trans_time() const
{
    return m_msg_trans_time;
}

/*!
 * @brief This function returns a reference to member msg_trans_time
 * @return Reference to member msg_trans_time
 */
int32_t& MsgRGBImage::msg_trans_time()
{
    return m_msg_trans_time;
}

/*!
 * @brief This function sets a value in member max_trans_time
 * @param _max_trans_time New value for member max_trans_time
 */
void MsgRGBImage::max_trans_time(
        int32_t _max_trans_time)
{
    m_max_trans_time = _max_trans_time;
}

/*!
 * @brief This function returns the value of member max_trans_time
 * @return Value of member max_trans_time
 */
int32_t MsgRGBImage::max_trans_time() const
{
    return m_max_trans_time;
}

/*!
 * @brief This function returns a reference to member max_trans_time
 * @return Reference to member max_trans_time
 */
int32_t& MsgRGBImage::max_trans_time()
{
    return m_max_trans_time;
}

/*!
 * @brief This function sets a value in member is_sent
 * @param _is_sent New value for member is_sent
 */
void MsgRGBImage::is_sent(
        bool _is_sent)
{
    m_is_sent = _is_sent;
}

/*!
 * @brief This function returns the value of member is_sent
 * @return Value of member is_sent
 */
bool MsgRGBImage::is_sent() const
{
    return m_is_sent;
}

/*!
 * @brief This function returns a reference to member is_sent
 * @return Reference to member is_sent
 */
bool& MsgRGBImage::is_sent()
{
    return m_is_sent;
}

/*!
 * @brief This function sets a value in member is_overtime
 * @param _is_overtime New value for member is_overtime
 */
void MsgRGBImage::is_overtime(
        bool _is_overtime)
{
    m_is_overtime = _is_overtime;
}

/*!
 * @brief This function returns the value of member is_overtime
 * @return Value of member is_overtime
 */
bool MsgRGBImage::is_overtime() const
{
    return m_is_overtime;
}

/*!
 * @brief This function returns a reference to member is_overtime
 * @return Reference to member is_overtime
 */
bool& MsgRGBImage::is_overtime()
{
    return m_is_overtime;
}

/*!
 * @brief This function copies the value in member message
 * @param _message New value to be copied in member message
 */
void MsgRGBImage::message(
        const std::array<char, 489600>& _message)
{
    m_message = _message;
}

/*!
 * @brief This function moves the value in member message
 * @param _message New value to be moved in member message
 */
void MsgRGBImage::message(
        std::array<char, 489600>&& _message)
{
    m_message = std::move(_message);
}

/*!
 * @brief This function returns a constant reference to member message
 * @return Constant reference to member message
 */
const std::array<char, 489600>& MsgRGBImage::message() const
{
    return m_message;
}

/*!
 * @brief This function returns a reference to member message
 * @return Reference to member message
 */
std::array<char, 489600>& MsgRGBImage::message()
{
    return m_message;
}

size_t MsgRGBImage::getKeyMaxCdrSerializedSize(
        size_t current_alignment)
{
    size_t current_align = current_alignment;















    return current_align;
}

bool MsgRGBImage::isKeyDefined()
{
    return false;
}

void MsgRGBImage::serializeKey(
        eprosima::fastcdr::Cdr& scdr) const
{
    (void) scdr;
                
}
