
#ifndef LOBBY_H
#define LOBBY_H
#include <iostream>
#include <vector>
#include <memory>
#include <map>
#include <thread>
#include <mutex>
#include <chrono>

class Lobby {
  public:
  Lobby(std::vector<std::string>lobby_info): lobby_code(lobby_info[0]) {};
  std::string lobby_code;
  std::vector<int> playersID;
  void add_player(int playerID){
    playersID.push_back(playerID);
  }
  std::string get_code(){
    return lobby_code;
  }



};
#endif //LOBBY_H
