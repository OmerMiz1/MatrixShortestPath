//
// Created by omer on 19/12/2019.
//


#include "Parser.h"
#include "Command.h"
#include "OpenServerCommand.h"
#include "ConnectCommand.h"
#include "DefineVarCommand.h"
#include "Print.h"
#include "Sleep.h"
#include "ConditionParser.h"
#include "LoopCommand.h"

using namespace std;

/*int getParseType(string);*/
int addOpenServerCmd(list<string>::iterator,list<pair<string,Command*>>);
int addConnectServerCmd(list<string>::iterator,list<pair<string,Command*>>);
int addDefineVarCmd(list<string>::iterator,list<pair<string,Command*>>);
int addPrintCmd(list<string>::iterator,list<pair<string,Command*>>);
int addSleepCmd(list<string>::iterator,list<pair<string,Command*>>);
int addWhileCmd(list<string>::iterator,list<pair<string,Command*>>);


void Parser::updateMap(list<string> input) {
  list<pair<string,Command*>> cmdList;
  list<pair<string,Command*>> localVarList;
  auto it = input.begin();

  // Generates list of key-Command
  while(it != input.end()) {
    //TODO support local var like: "var h0 = heading"

    if(it->compare("OpenServerCommand") == 0) {
      advance(it,addOpenServerCmd(it, cmdList));
    } else if (it->compare("connectControlClient")==0 ){
      advance(it,addConnectServerCmd(it, cmdList));
    } else if (it->compare("var")==0 ){
      advance(it,addDefineVarCmd(it, cmdList));
    } else if (it->compare("Print")==0 ){
      advance(it, addPrintCmd(it, cmdList));
    } else if (it->compare("Sleep")==0 ){
      advance(it, addSleepCmd(it, cmdList));
    } else if (it->compare("while")==0 ){
      advance(it, addWhileCmd(it, cmdList));
    } else if (it->compare("if")==0 ){
      advance(it, addIfCmd(it,cmdList));
    } else {
      advance(it, addOther(it,cmdList));
    }

    if (it->compare("while") == 0 || it->compare("if") == 0) {
      list<string> innerScopeTokens;
      string closeScopeToken("}");
      Parser *parser = new Parser();
      string type = *it;
      string conditionVal = *(++it);
      Command *conditionCmd;

      // insert all tokens up until end of scope "}" reached.
      while ((++it)->compare("}") != 0) { // NOTE: no nested scopes.
        innerScopeTokens.emplace_back(*it);
      }

      parser->updateMap(innerScopeTokens);



      /*
      string type = *it;
      string condition = *(++it);
      Command *loopCmd = new LoopCommand((type+condition));
      while (it->compare("}") != 0) {
        string name = *it;
        string value = *(++it);
        if (name.compare("Print") == 0) {
          loopCmd->addVar(name)
        } else if (name.compare("Sleep")) {

        } else if (*//*ASSIGNMENT*//*) {

        }
      }*/
      cmdList.emplace_back(type, loopCmd);

    } else if (it->compare("if") == 0) {

    } else { // enters here if there is an assignment like "alt = 15"
      auto var = varMap.find(*it);
      //error - var not found in map.
      if(var == varMap.end()) {
        throw "Variable not found/declared: " + (*it);
      }
      string name = *it;
      string value = *(++it);
      cmdList.emplace_back(name, new DefineVarCommand(name, value));
    }
    //TODO support FunctionCommands when done.
  }

  // Generates a map of key-iterator (iterator points to appropriate object)
  for(auto it = cmdList.begin(); it != cmdList.end(); it++) {
    this->cmdMap.emplace(it->first, it);
  }

  for(auto it = localVarList.begin(); it != localVarList.end(); it++) {
    this->localVarMap.emplace(it->first,it);
  }
}

/*int getParseType(string str) {
  if(str.compare("OpenServerCommand") == 0) {
    return 1;
  } else if (str.compare("connectControlClient")==0 ){
    return 2;
  } else if (str.compare("var")==0 ){
    return 3;
  } else if (str.compare("Print")==0 ){
    return 4;
  } else if (str.compare("Sleep")==0 ){
    return 5;
  } else if (str.compare("while")==0 ){
    return 6;
  } else if (str.compare("if")==0 ){
    return 7;
  } else {
    return 8;
  }
}*/

int addOpenServerCmd(list<string>::iterator it, list<pair<string,Command*>> list) {
  ++it;
  string port = *it;
  list.emplace_back("OpenDataServer", new OpenServerCommand(port));
  return 2;
}

int addConnectServerCmd(list<string>::iterator it, list<pair<string,Command*>> list) {
  ++it;
  string ip = *it;
  ++it;
  string port = *it;
  // string.c_str returns the string as const char*.
  list.emplace_back("connectControlClient", new ConnectCommand(port.c_str(),ip.c_str()));
  return 3;
}

int addDefineVarCmd(list<string>::iterator it,list<pair<string,Command*>> cmds) {
  ++it;
  string name = *it;
  ++it;
  string operation = *it; // operation such as <-, -> and =
  ++it;
  string value = *it;
  cmds.emplace_back(name, new DefineVarCommand(name, operation, value));
  return 4;
  /*if(operation.compare("=") == 0) { // = is for local variables
    localVarList.emplace_back(name, new DefineVarCommand(name, operation, value));
  } else { // -> and <- means server and client can access value
    cmdList.emplace_back(name, new DefineVarCommand(name, operation, value));
  }*/
}

int addPrintCmd(list<string>::iterator it, list<pair<string, Command *>> list) {
  string name = *it;
  ++it;
  list.emplace_back(name, new Print(*it));
  return 2;
}

int addSleepCmd(list<string>::iterator it, list<pair<string, Command *>> list) {
  string name = *it;
  ++it;
  list.emplace_back(name, new Sleep(*it));
  return 2;
}

int addWhileCmd(list<string>::iterator it, list<pair<string, Command *>> list) {
  std::list<string> innerScopeTokens;
  string closeScopeToken("}");
  Parser *parser = new Parser();
  int tokensCount = 0;


  //TODO continue building Parser here first of all
  ++it;
  ++tokensCount;
  string conditionVal = *it;

  ++it;
  ++tokensCount;
  //TODO put the while loop in a dedicated function below.

  // insert entire scope (tokens) into a list.
  tokensCount += updateScopeTokens(it, innerScopeTokens);

  // parse tokens into a command map (with a difference parser object!).
  parser->updateMap(innerScopeTokens);
  LoopCommand* conditionCmd = new LoopCommand(conditionVal);
  conditionCmd->setCommandMap(parser->getCmdMap());
  conditionCmd->setVarMap(parser->getVarMap());
  //TODO not sure about this cast here. supposed to be fine.
  list.emplace_back("while", dynamic_cast<Command*>(conditionCmd));
  return tokensCount;
}

int addIfCmd(list<string>::iterator it, list<pair<string, Command *>> list) {

}

int updateScopeTokens(list<string>::iterator it, list<string> list) {
  int count = 0;
  for(count=0; it->compare("}") != 0; ++it, ++count;) { // NOTE: no nested scopes.
    list.emplace_back(*it);
  }
}