/*******************************************************************************
 * am_rtsp_server.h
 *
 * History:
 *   2014-12-19 - [Shiming Dong] created file
 *
 * Copyright (c) 2015 Ambarella, Inc.
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
#ifndef AM_RTSP_SERVER_H_
#define AM_RTSP_SERVER_H_

#include "am_rtsp_server_if.h"

#include <netinet/in.h>
#include <atomic>
#include <mutex>
#include <map>

struct RtspServerConfig;

class AMEvent;
class AMMutex;
class AMThread;
class AMRtspServerConfig;
class AMRtspClientSession;

class AMRtspServer: public AMIRtspServer
{
  friend class AMRtspClientSession;
  typedef std::map<uint32_t, AMRtspClientSession*> AMRtspClientMap;

  public:
    static AMIRtspServer* get_instance();
    virtual bool start();
    virtual void stop();
    virtual uint32_t version();

  protected:
    virtual void inc_ref();
    virtual void release();

  protected:
    uint16_t get_server_port_tcp();
    uint32_t get_random_number();

  private:
    static void static_server_thread(void *data);
    static void static_unix_thread(void *data);

    void server_thread();
    void server_thread_epoll();
    void connect_unix_thread();
    void connect_unix_thread_epoll();
    bool start_server_thread();
    bool start_connect_unix_thread();
    void server_abort();
    bool setup_server_socket_tcp(uint16_t server_port);
    void abort_client(AMRtspClientSession &client);
    bool delete_client_session(uint32_t identify);
    bool find_client_and_kill(struct in_addr &addr);
    bool get_source_addr(sockaddr_in& client_addr, sockaddr_in& source_addr);
    bool get_rtsp_url(const char *stream_name,
                      char *buf,
                      uint32_t size,
                      struct sockaddr_in &client_addr);

    /* unix domain socket */
    int unix_socket_conn(const char *server_name);
    void unix_socket_close();
    bool recv_rtp_control_data();
    void destroy_all_client_session(bool client_abort = true);
    bool need_authentication();
    bool send_rtp_control_data(uint8_t *data, size_t len);
    void reject_client(int fd);
    bool process_ctrl_data(int fd);
    bool process_client_connect_request(int fd);
  private:
    AMRtspServer();
    virtual ~AMRtspServer();
    bool construct();

  private:
    RtspServerConfig    *m_rtsp_config; /* No need to delete */
    AMRtspServerConfig  *m_config;
    AMThread            *m_server_thread;
    AMThread            *m_sock_thread;
    AMEvent             *m_event;
    AMMutex             *m_client_mutex;
    int                  m_srv_sock_tcp;
    int                  m_unix_sock_fd;
    int                  m_pipe[2];
    int                  m_ctrl_unix_fd[2];
    uint16_t             m_port_tcp;
    std::atomic<bool>    m_run;
    std::atomic<bool>    m_unix_sock_con;
    std::atomic<bool>    m_unix_sock_run;
    std::string          m_config_file;
    AMRtspClientMap      m_client_map;
    std::atomic_int      m_ref_count;

  private:
    static AMRtspServer *m_instance;
    static std::mutex    m_lock;
};

#endif /* AM_RTSP_SERVER_H_ */
