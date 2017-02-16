/*******************************************************************************
 * am_amf_queue.h
 *
 * History:
 *   2014-7-22 - [ypchang] created file
 *
 * Copyright (c) 2016 Ambarella, Inc.
 *
 * This file and its contents ("Software") are protected by intellectual
 * property rights including, without limitation, U.S. and/or foreign
 * copyrights. This Software is also the confidential and proprietary
 * information of Ambarella, Inc. and its licensors. You may not use, reproduce,
 * disclose, distribute, modify, or otherwise prepare derivative works of this
 * Software or any portion thereof except pursuant to a signed license agreement
 * or nondisclosure agreement with Ambarella, Inc. or its authorized affiliates.
 * In the absence of such an agreement, you agree to promptly notify and return
 * this Software to Ambarella, Inc.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF NON-INFRINGEMENT,
 * MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL AMBARELLA, INC. OR ITS AFFILIATES BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; COMPUTER FAILURE OR MALFUNCTION; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/
#ifndef AM_AMF_QUEUE_H_
#define AM_AMF_QUEUE_H_

class AMMutex;
class AMCondition;

class AMQueue
{
  public:
    enum QTYPE
    {
      AM_Q_MSG,
      AM_Q_DATA
    };
    struct WaitResult
    {
        AMQueue *dataQ;
        void    *owner;
        uint32_t block_size;
    };
  public:
    static AMQueue* create(AMQueue *mainQ,
                           void    *owner,
                           uint32_t blockSize,
                           uint32_t reservedSlots);
    void destroy();

  public:
    AM_STATE post_msg(const void *msg, uint32_t msgSize);
    AM_STATE send_msg(const void *msg, uint32_t msgSize);

    void get_msg(void *msg, uint32_t msgSize);
    bool get_msg_non_block(void *msg, uint32_t msgSize);
    bool peek_msg(void *msg, uint32_t msgSize);
    void reply(AM_STATE result);

    void enable(bool enabled = true);
    bool is_enable();

    AM_STATE put_data(const void *buffer, uint32_t size);

    QTYPE wait_data_msg(void *msg, uint32_t msgSize, WaitResult *result);
    bool peek_data(void *buffer, uint32_t size);
    uint32_t get_available_data_number();

  public:
    bool is_main();
    bool is_sub();

  private:
    AMQueue(AMQueue *mainQ, void *owner);
    virtual ~AMQueue();
    AM_STATE init(uint32_t blockSize, uint32_t reservedSlots);

  private:
    struct List
    {
      List *next;
      bool allocated;
      void destroy();
    };

  private:
    void copy(void *to, const void *from, uint32_t bytes);
    List* alloc_node();
    void write_data(List *node, const void *buffer, uint32_t size);
    void read_data(void *buffer, uint32_t size);
    void wakeup_any_reader();

  private:
    uint8_t     *m_reserved_mem;
    AM_STATE    *m_msg_result;
    void        *m_owner;
    List        *m_head;
    List        *m_tail;
    List        *m_free_list;
    List        *m_send_buffer;
    AMQueue     *m_main_q;
    AMQueue     *m_prev_q;
    AMQueue     *m_next_q;
    AMMutex     *m_mutex;
    AMCondition *m_cond_reply;
    AMCondition *m_cond_get;
    AMCondition *m_cond_send_msg;
    uint32_t     m_num_get;
    uint32_t     m_num_send_msg;
    uint32_t     m_num_data;
    uint32_t     m_block_size;
    bool         m_is_disabled;
};

#endif /* AM_AMF_QUEUE_H_ */
