#pragma  once
#include <WinSock2.h>
#include "ChildView.h"

class GameSocket{
public:
  static GameSocket* GetInstance(){
    static GameSocket *gs = new GameSocket();
    return gs;
  }

  void InitSocket();
  void ReleaseSocket();

  void BroadcastUDP();     //ͨ��UDP���㲥�Լ���ս������ͨ����Ӧ�ҳ�����������
  void ListenForBroadcastUDP(); //����search alive�������յ�ʱ���ط��Լ���IP��ַ

  void WatchGameTCP(char ip[] = "");       //�����۲�õ���
  void ListenForWatchGameTCP();   //����watch�������յ�ʱ����ʼͬ����Ϸ״̬

  //����ͨ��ר��
private:
  SOCKET broadcast_client_sock, broadcast_server_sock;
  SOCKADDR_IN broacast_client_addr, broadcast_serv_addr;
  SOCKET watch_game_server_sock;
  SOCKADDR_IN watch_game_server_addr;

  bool is_listening_tcp;

private:
  GameSocket(){
    is_listening_tcp = false;
    InitSocket();
  }

  ~GameSocket(){
    ReleaseSocket();
  }
};