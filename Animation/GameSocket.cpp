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

SOCKET send_sock;  //��WatchGameTCP�б���ʼ��
SOCKET recv_sock;  //��ListenWatchGameTCP�б���ʼ��

extern CChildView *global_view;

DWORD WINAPI TcpServerFun(LPVOID pM)
{  
  GameSocket::GetInstance()->ListenForWatchGameTCP();
  return 0;
} 

//���͹㲥����������Ҫ��ս����û�е��Ը��ҹ�ս��
void GameSocket::BroadcastUDP()
{
  if(!is_listening_tcp){
    //�ڷ���֮ǰ��TCP�������Խ��ܶԷ��յ�UDP����֮�����TCP����
    HANDLE tcp_thread = CreateThread(NULL, 0, TcpServerFun, NULL, 0, NULL);
    is_listening_tcp = true;
    cout<<"���Ѿ���ʼ����Watch Game TCP" <<endl;
  }

  //����UDP�㲥��Ϣ
  char sendBuf[PACKAGE_SIZE] = "Watch Game";
  sendto(broadcast_client_sock, sendBuf, strlen(sendBuf), 0, 
    (SOCKADDR*)&broacast_client_addr, sizeof(SOCKADDR));
}

//��ս����ļ�����
void GameSocket::ListenForBroadcastUDP()
{
  SOCKADDR_IN client_addr;
  int len = sizeof(SOCKADDR);

  //����   
  cout<<"���Ѿ���ʼ����Broadcast UDP" <<endl;
  while (true)
  {
    char recvBuf[PACKAGE_SIZE] = {};
    recvfrom(broadcast_server_sock, recvBuf, PACKAGE_SIZE, 0, (SOCKADDR*)&client_addr, &len);

    //�Խ��յ�����Ϣ�����ж�
    if(strcmp(recvBuf, "Watch Game") == 0){
      cout<<"��Ҫ����Ϸ��" <<endl;      
      //�������˷���TCP����
      WatchGameTCP(inet_ntoa(client_addr.sin_addr));
    }
  }
  return;
}

void GameSocket::WatchGameTCP(char ip[])
{
  //�����ͻ��˵�TCP���ӣ�����۲�ĳ̨����(������)����Ϸ״̬
  send_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (send_sock == INVALID_SOCKET){
    cout<<"ERROR: Set Up Watch Game TCP Wrong!" <<GetLastError() <<endl;  
    return;
  }

  //�趨Ŀ���������ַ
  SOCKADDR_IN servAddr;
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.S_un.S_addr = inet_addr(ip);
  cout<<ip<<endl;
  servAddr.sin_port = htons(TCP_PORT);

  //ͨ���������׽��� ���� ����IP��ַ�ķ���������Ӧ�Ķ˿ں�
  if ( connect(send_sock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == -1){
    cout<<"ERROR: Connect to Observer Failed!" <<endl;
    return;
  }
  //���յ����۲���Ϸ�����ʱ�򣬴򿪱�־λ
  global_view->m_is_being_watched = true;
  return;
}

void GameSocket::ListenForWatchGameTCP()
{
  listen(watch_game_server_sock, 10);

  //������ѭ���ȴ����Է������˵�����
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

//����ͨ��ר��
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

  //���ļ��ж�ȡIP��ַ
  std::ifstream in;
  std::string ip;
  in.open(GAME_CONFIG);
  in>>ip;
  in.close();

  //��ʼ���㲥client socket udp
  broadcast_client_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  broacast_client_addr.sin_family = AF_INET;
  broacast_client_addr.sin_addr.S_un.S_addr = inet_addr(ip.c_str());//255.255.255.255 192.168.1.105 127.0.0.1
  broacast_client_addr.sin_port = htons(UDP_PORT);

  //��ʼ���㲥server socket udp
  broadcast_server_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  broadcast_serv_addr.sin_family = AF_INET;
  broadcast_serv_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
  broadcast_serv_addr.sin_port = htons(UDP_PORT);
  if( bind(broadcast_server_sock, (SOCKADDR*)&broadcast_serv_addr, sizeof(SOCKADDR)) == SOCKET_ERROR){
    cout<<"ERROR Binding UDP broadcast server socket!" <<endl;
  }

  //��ʼ����սserver socket tcp  
  watch_game_server_sock = socket(AF_INET, SOCK_STREAM, 0);
  watch_game_server_addr.sin_family = AF_INET;
  watch_game_server_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
  watch_game_server_addr.sin_port = htons(TCP_PORT);
  //�������󶨵������˿��ϣ���ʼ�౾��ָ���˿ڵ�����
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