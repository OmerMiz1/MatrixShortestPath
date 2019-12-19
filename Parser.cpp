//
// Created by omer on 19/12/2019.
//

#include <unordered_map>
#include <list>
#include <iostream>
#include "Parser.h"
#include "Command.h"
#include "OpenServerCommand.h"
#include "ConnectCommand.h"
#include "DefineVarCommand.h"
using namespace std;

void Parser::genMap(list<string> input) {
  unordered_map<string, list<pair<string,Command*>>::iterator> cmdMap;
  list<pair<string,Command*>> cmdList;

  // Generates list of key-Command
  for(auto it = input.begin(); it != input.end(); ++it) {
    if ((*it).compare("openDataServer") == 0) {
      int port = stoi(*(++it));
      cmdList.emplace_back("OpenDataServer", new OpenServerCommand(port));
    } else if (((*it).compare("connectControlClient") == 0)) {
      string ip = *(++it);
      int port = stoi(*(++it));
      cmdList.emplace_back("connectControlClient", new ConnectCommand(port,ip));
    } else if ((*it).compare("var") == 0) {
      string name = *(++it);
      string direction = *(++it);
      string sim = *(++it);
      cmdList.emplace_back(name, new DefineVarCommand(name, direction, sim));
    } else if ((*it).compare("Print") == 0) {
      string msg = *(++it);
      //TODO find out how to make a Print annoymouse / as lambda?
      cmdList.emplace_back("Print",({
        [&msg]() {
        std::cout << msg << std::endl;
        }
      }));
      //TODO Print
    } else if ((*it).compare("Sleep") == 0) {
      //TODO Sleep
    } else if ((*it).compare("warp") == 0) {
      //TODO
    }

  }

  // Generates a map of key-iterator (iterator points to appropriate object)
  for(auto it = cmdList.begin(); it != cmdList.end(); it++) {
    cmdMap.emplace(it->first, it);
  }

}
