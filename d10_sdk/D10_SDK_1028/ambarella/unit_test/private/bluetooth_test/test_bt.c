#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <pthread.h>
#include <signal.h>
#include "bluetooth/bluetooth.h"
#include "bluetooth/rfcomm.h"

#define BUFSIZE 512
int server_sock, client_sock;
pthread_t thread_recv_id;
pthread_t thread_send_id;

void main_stop(int signo)
{
  printf("stop bt process!\n");
  close(client_sock);
  close(server_sock);
  exit(0);
}

void signal_pipe(int signo)
{
  int retval = 0;
  pthread_kill(thread_recv_id, SIGQUIT);
  pthread_join(thread_recv_id, NULL);

  close(client_sock);
  printf("the client disconnect!\n");
  pthread_exit( &retval );
  
}

void thread_recv_out(int signo)
{
  int retval = 0;
  pthread_exit( &retval );
}

void thread_recv(void *arg)
{
  char recvbuf[BUFSIZE];
  int bytes_recv;
  int retval = 0;
  int sockfd = *(int *)arg;

  memset(&recvbuf, 0, BUFSIZE);

  signal( SIGQUIT, thread_recv_out );

  while(1)
  {
    bytes_recv = recv(sockfd, recvbuf, BUFSIZE, 0);
    if( bytes_recv > 0 )
    {
        printf("<%s\n", recvbuf);
        if( strcmp(recvbuf, "goodbye") == 0 ){
            printf("client is down!\n");
            pthread_exit( &retval );
        }
        memset(recvbuf, 0, bytes_recv);
    }
  }
}

void thread_send( void *arg )
{
  char sendbuf[BUFSIZE];
  int buflen = 0;
  int sockfd = *(int *)arg;

  memset(&sendbuf, 0, BUFSIZE);

  signal( SIGPIPE, signal_pipe );

  while(1)
  {
    scanf("%s", sendbuf);
    buflen = strlen(sendbuf);
    sendbuf[buflen] = '\r';
    sendbuf[buflen+1] = '\n';
    send(sockfd, sendbuf, buflen+2, 0);
    //send(sockfd, sendbuf, strlen(sendbuf), 0);
  }
}

int main(int argc, char **argv)
{
  struct sockaddr_rc server_addr = {0};
  struct sockaddr_rc client_addr = {0};
  char buf[512] = {0};
  int server_sock,  status;
  int on = 1;
  int opt = sizeof(client_addr);

  //create server socket
  printf("creating secket ....\n");
  server_sock = socket(PF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
  if( server_sock < 0 ){
    perror("create socket error");
    exit(1);
  } else printf("create socket success!\n");

  //Enable address reuse
  status = setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, (void *)&on, sizeof(on));
  if( status < 0 ){
    perror("set socket opt error");
    exit(1);
  } else printf("set socket opt success!\n");

  //setup server_addr
  server_addr.rc_family = AF_BLUETOOTH;
  server_addr.rc_bdaddr = *BDADDR_ANY;
  server_addr.rc_channel = (uint8_t)22;
  
  //bind server_sock
  printf("binding socket ...\n");
  status = bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if( status < 0 ){
    perror("bind socket error");
    exit(1);
  } else printf("bind socket success!\n");

  //listen server_sock
  printf("listening socket ...\n");
  status = listen(server_sock, 5);
  if( status < 0 ){
    perror("listen socket error");
    exit(1);
  } else printf("listen socket success!\n");

  //registered SIGINT signal
  signal( SIGINT, main_stop );

  while(1)
  {
    //accept socket
    printf("accepting socket ...\n");
    client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &opt);
    if( client_sock < 0 ){
      perror("accept socket error");
      exit(1);
    } else printf("accept socket success!\n");
  
    ba2str(&client_addr.rc_bdaddr, buf);
    printf("accept connect from %s\n", buf);
    memset(buf, 0, sizeof(buf));

    //create thread_recv
    if( pthread_create( &thread_recv_id, NULL, (void *)(&thread_recv), (void *)(&client_sock))==0 )
        printf("thread_recv create success!\n");
    else
    {
      printf("thread_recv create fail!\n");
      continue;
    }

     //create thread_recv
    if( pthread_create( &thread_send_id, NULL, (void *)(&thread_send), (void *)(&client_sock))==0 )
        printf("thread_send create success!\n");
    else
    {
      printf("thread_send create fail!\n");
      continue;
    }
  }
  return 0;
}

