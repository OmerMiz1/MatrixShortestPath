//
// Created by Dor Refaeli on 28/12/2019.
//

#include <thread>
#include <iostream>
#include "SymbolTable.h"
#include "Expression.h"

SymbolTable::SymbolTable() {
    this->father = nullptr;
}

SymbolTable::SymbolTable(SymbolTable *father) : father(father) {}

float SymbolTable::getVariable(string name) {
    //while asked for a variable that's updated by the server and wasn't updated yet, wain 0.1 seconds
    while (this->getIngoing().count(name) && !this->recursiveContains(name))
    {
        //TODO remove only the first line before submitting!!!
        std::cout << "asked for a variable that the server havn't added yet \"" + name + "\"" << std::endl;
        this_thread::sleep_for(100ms);
    }
    //if the variable isn't known to the symbol table (or his fathers)
    if (!this->recursiveContains(name)) {
        //it's an expression, evaluate him and return his value
        Interpreter interpreter;
        interpreter.setVariables(this->updatedMap());
        Expression *expression = interpreter.interpret(name);
        return expression->calculate();
    } else if (this->contains(name)) {
        return localVariables[name];
    } else if (this->father != nullptr) {
        return this->father->getVariable(name);
    }
    throw "Error getting a variable";
}

void SymbolTable::setVariable(string name, float num) {

    if (!this->contains(name) && this->recursiveContains(name)) {
        this->father->setVariable(name, num);
    } else {
        this->localVariables[name] = num;
        this->addToOutgoingIfNeeded(name, num);
    }
}

void SymbolTable::setRemoteVariable(string name, string direction, string simLocation) {
    //removes the - sim(" - from the start of the location string
    simLocation.erase(0,5);
    //removes the - ) - from the start of the location string
    simLocation.pop_back();
    //removes the - " - from the start of the location string
    simLocation.pop_back();
    this->remoteVariables[name] = make_pair(direction, simLocation);
    if (direction == "<-") {
        addToIngoing(name, simLocation);
    }
}

bool SymbolTable::contains(string name) {
    return this->localVariables.count(name);
}

bool SymbolTable::recursiveContains(string name) {
    if (this->father != nullptr) {
        return this->contains(name) || this->father->recursiveContains(name);
    } else {
        return this->contains(name);
    }
}

map<string, string> SymbolTable::getIngoing() {
    return this->ingoing;
}

map<string, float> SymbolTable::getOutgoing() {

    map<string, float> temp;
    auto it = this->outgoing.begin();
    while (it != this->outgoing.end()) {
        temp[it->first] = it->second;
        it++;
    }
    this->clearOutgoing();
    return temp;
}

map<string, float> SymbolTable::updatedMap() {

    map<string, float> temp;
    if (this->father != nullptr) {
        temp = this->father->updatedMap();
    }
    auto it = this->localVariables.begin();
    while (it != this->localVariables.end()) {
        temp[it->first] = it->second;
        ++it;
    }
    return temp;
}

void SymbolTable::addToIngoing(string name, string simLocation) {

    this->ingoing[name] = simLocation;
}

void SymbolTable::addToOutgoingIfNeeded(string name, float num) {

    if (this->father == nullptr) {
        if (this->remoteVariables.count(name)) {
            if (this->remoteVariables[name].first == "->") {
                this->outgoing[this->remoteVariables[name].second] = num;
            }
        }
    } else {
        this->father->addToOutgoingIfNeeded(name, num);
    }
}

void SymbolTable::clearOutgoing() {
    this->outgoing.clear();
}
