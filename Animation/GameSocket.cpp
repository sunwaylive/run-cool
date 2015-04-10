#pragma  once
#include "stdafx.h"
#include "GameSocket.h"
#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>

using std::cin;
using std::cout;
using std::endl;

SOCKET send_sock;  //在WatchGameTCP中被初始化
SOCKET recv_sock;  //在ListenWatchGameTCP中被初始化

extern CChildView *global_view;

DWORD WINAPI TcpServerFun(LPVOID pM)
{  
  GameSocket::GetInstance()->ListenForWatchGameTCP();
  return 0;
} 

//发送广播，我现在想要观战，有没有电脑给我观战的
void GameSocket::BroadcastUDP()
{
  if(!is_listening_tcp){
    //在发送之前打开TCP监听，以接受对方收到UDP请求之后发起的TCP连接
    HANDLE tcp_thread = CreateThread(NULL, 0, TcpServerFun, NULL, 0, NULL);
    is_listening_tcp = true;
    cout<<"我已经开始监听Watch Game TCP" <<endl;
  }

  //发送UDP广播消息
  char sendBuf[PACKAGE_SIZE] = "Watch Game";
  sendto(broadcast_client_sock, sendBuf, strlen(sendBuf), 0, 
    (SOCKADDR*)&broacast_client_addr, sizeof(SOCKADDR));
}

//观战请求的监听端
void GameSocket::ListenForBroadcastUDP()
{
  SOCKADDR_IN client_addr;
  int len = sizeof(SOCKADDR);

  //阻塞   
  cout<<"我已经开始监听Broadcast UDP" <<endl;
  while (true)
  {
    char recvBuf[PACKAGE_SIZE] = {};
    recvfrom(broadcast_server_sock, recvBuf, PACKAGE_SIZE, 0, (SOCKADDR*)&client_addr, &len);

    //对接收到的消息进行判断
    if(strcmp(recvBuf, "Watch Game") == 0){
      cout<<"我要看游戏！" <<endl;      
      //服务器端发起TCP连接
      WatchGameTCP(inet_ntoa(client_addr.sin_addr));
    }
  }
  return;
}

void GameSocket::WatchGameTCP(char ip[])
{
  //创建客户端的TCP连接，请求观察某台机器(服务器)的游戏状态
  send_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (send_sock == INVALID_SOCKET){
    cout<<"ERROR: Set Up Watch Game TCP Wrong!" <<GetLastError() <<endl;  
    return;
  }

  //设定目标服务器地址
  SOCKADDR_IN servAddr;
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.S_un.S_addr = inet_addr(ip);
  cout<<ip<<endl;
  servAddr.sin_port = htons(TCP_PORT);

  //通过服务器套接字 连接 参数IP地址的服务器的相应的端口号
  if ( connect(send_sock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == -1){
    cout<<"ERROR: Connect to Observer Failed!" <<endl;
    return;
  }
  //当收到被观察游戏请求的时候，打开标志位
  global_view->m_is_being_watched = true;
  return;
}

void GameSocket::ListenForWatchGameTCP()
{
  listen(watch_game_server_sock, 10);

  //下面是循环等待来自服务器端的连接
  SOCKADDR_IN clientAddr;
  int len = sizeof(SOCKADDR);
  while(true){
    recv_sock = accept(watch_game_server_sock, (SOCKADDR*)&clientAddr, &len);
    if(recv_sock == SOCKET_ERROR){
      cout<<"Waiting for Watch Game TCP." <<endl;
      return;
    }

    while (true)
    {
      global_view->ReceiveDataFromTCP();
    }

    return;
  }
}

//网络通信专用
void GameSocket::InitSocket()
{
  WORD wVersionRequested;
  WSADATA wsaData;
  int err;

  wVersionRequested = MAKEWORD( 2, 2 );

  err = WSAStartup( wVersionRequested, &wsaData );
  if ( err != 0 ) {
    return;
  }

  if ( LOBYTE( wsaData.wVersion ) != 2 ||
    HIBYTE( wsaData.wVersion ) != 2 ) {
      WSACleanup( );
      return; 
  }

  //从文件中读取IP地址
  std::ifstream in;
  std::string ip;
  in.open(GAME_CONFIG);
  in>>ip;
  in.close();

  //初始化广播client socket udp
  broadcast_client_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  broacast_client_addr.sin_family = AF_INET;
  broacast_client_addr.sin_addr.S_un.S_addr = inet_addr(ip.c_str());//255.255.255.255 192.168.1.105 127.0.0.1
  broacast_client_addr.sin_port = htons(UDP_PORT);

  //初始化广播server socket udp
  broadcast_server_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  broadcast_serv_addr.sin_family = AF_INET;
  broadcast_serv_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
  broadcast_serv_addr.sin_port = htons(UDP_PORT);
  if( bind(broadcast_server_sock, (SOCKADDR*)&broadcast_serv_addr, sizeof(SOCKADDR)) == SOCKET_ERROR){
    cout<<"ERROR Binding UDP broadcast server socket!" <<endl;
  }

  //初始化观战server socket tcp  
  watch_game_server_sock = socket(AF_INET, SOCK_STREAM, 0);
  watch_game_server_addr.sin_family = AF_INET;
  watch_game_server_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
  watch_game_server_addr.sin_port = htons(TCP_PORT);
  //服务器绑定到本机端口上，开始监本机指定端口的连接
  if (bind(watch_game_server_sock, (SOCKADDR*)&watch_game_server_addr, sizeof(SOCKADDR))){
    cout<<"ERROR Binding TCP watch game server socket!" <<endl;
  }  
}

void GameSocket::ReleaseSocket()
{
  closesocket(broadcast_client_sock);
  closesocket(broadcast_server_sock);
  closesocket(watch_game_server_sock);
  closesocket(send_sock);
  closesocket(recv_sock);
  WSACleanup();
}