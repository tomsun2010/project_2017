/*******************************************************************************
 * am_sip_ua_server.h
 *
 * History:
 *   2015-1-26 - [Shiming Dong] created file
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

#ifndef AM_SIP_UA_SERVER_H_
#define AM_SIP_UA_SERVER_H_

#include "am_sip_ua_server_if.h"

#include "eXosip2/eXosip.h"
#include "osip2/osip_mt.h"
#include "osipparser2/osip_message.h"
#include "osipparser2/osip_parser.h"
#include "am_media_service_msg.h"

#include <atomic>
#include <mutex>
#include <map>

#include <string>
#include <queue>

const char* AM_MEDIA_UNSUPPORTED = "unsupported";

class AMEvent;
class AMMutex;
class AMThread;

struct SipUAServerConfig;
struct AMMediaServiceMsgINFO;

typedef std::map<std::string, int> SipMediaMap;

class AMSipUAServer: public AMISipUAServer
{
  enum {
    WAIT_TIMER  = 300,
    REG_TIMER   = 300*1000,
    BUSY_TIMER  = 10,
  };

  struct MediaInfo
  {
    uint32_t    ssrc;
    uint32_t    session_id;
    uint16_t    rtp_port;
    bool        is_alive;
    bool        is_supported;
    std::string media;
    std::string sdp;
    MediaInfo();
  };

  struct SipClientInfo
  {
    int dialog_id;
    int call_id;
    std::string username;
    MediaInfo media_info[AM_RTP_MEDIA_NUM];
    SipClientInfo() :
      dialog_id(-1),
      call_id(-1) {}
    SipClientInfo(int dialog, int call) :
      dialog_id(dialog),
      call_id(call) {}
  };
  typedef std::queue<SipClientInfo*> SipClientInfoQue;

  public:
    static AMISipUAServer* get_instance();
    virtual bool start();
    virtual void stop();
    virtual uint32_t version();
    virtual bool set_sip_registration_parameter(AMSipRegisterParameter*);
    virtual bool set_sip_config_parameter(AMSipConfigParameter*);
    virtual bool set_sip_media_priority_list(AMMediaPriorityList*);
    virtual bool initiate_sip_call(AMSipCalleeAddress *address);
    virtual bool hangup_sip_call(AMSipHangupUsername *name);

  protected:
    virtual void inc_ref();
    virtual void release();

  public:
    void hang_up_all();
    void hang_up(SipClientInfo* sip_client);
    bool register_to_server(int expires);
    static void busy_status_timer(int arg);
    void delete_invalid_sip_client();

  private:
    AMSipUAServer();
    virtual ~AMSipUAServer();
    bool construct();

    uint32_t get_random_number();
    bool init_sip_ua_server();
    bool get_supported_media_types();

    bool start_sip_ua_server_thread();
    static void static_sip_ua_server_thread(void *data);
    void sip_ua_server_thread();

    bool start_connect_unix_thread();
    static void static_unix_thread(void *data);
    int create_unix_socket_fd(const char* socket_name);
    void connect_unix_thread();
    void release_resource();

    int unix_socket_conn(int fd, const char *server_name);
    bool recv_rtp_control_data();
    bool send_rtp_control_data(uint8_t *data, size_t len);
    const char* select_media_type(uint32_t uas_type, sdp_media_t* uac_type);
    void set_media_map();
    bool parse_sdp(AMMediaServiceMsgAudioINFO& info, const char* audio_sdp,
                   int32_t size);
    AM_MEDIA_NET_STATE recv_data_from_media_service(
                                 AMMediaServiceMsgBlock& service_msg);
    bool send_data_to_media_service(AMMediaServiceMsgBlock& send_msg);

  private:
    static AMSipUAServer *m_instance;
    static bool           m_busy;
    static std::mutex     m_lock;
    eXosip_t             *m_context;
    eXosip_event_t       *m_uac_event;
    osip_message         *m_answer;
    AMThread             *m_server_thread;
    AMThread             *m_sock_thread;
    AMEvent              *m_event;
    AMEvent              *m_event_support;
    AMEvent              *m_event_sdp;
    AMEvent              *m_event_sdps;
    AMEvent              *m_event_ssrc;
    AMEvent              *m_event_kill;
    SipUAServerConfig    *m_sip_config;
    AMSipUAServerConfig  *m_config;

    int                   m_ctrl_unix_fd[2];
    uint32_t              m_media_type;
    int                   m_unix_sock_fd;
    int                   m_media_service_sock_fd;
    uint16_t              m_connected_num;
    uint16_t              m_rtp_port;
    bool                  m_run;
    bool                  m_unix_sock_run;

    std::atomic_int       m_ref_count;
    std::string           m_sdps;
    std::string           m_config_file;

    SipClientInfoQue      m_sip_client_que;
    SipMediaMap           m_media_map;

};

#endif /* AM_SIP_UA_SERVER_H_ */
