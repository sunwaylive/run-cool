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

  void BroadcastUDP();     //通过UDP来广播自己观战的请求，通过响应找出正在玩的玩家
  void ListenForBroadcastUDP(); //监听search alive的请求，收到时，回发自己的IP地址

  void WatchGameTCP(char ip[] = "");       //决定观察该电脑
  void ListenForWatchGameTCP();   //监听watch的请求，收到时，开始同步游戏状态

  //网络通信专用
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