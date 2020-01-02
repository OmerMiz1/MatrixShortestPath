//
// Created by omer on 19/12/2019.
//

#define MAX_CHARS 1024
#define MAX_CLIENTS 1

#include "OpenServerCommand.h"

using namespace std;

OpenServerCommand::OpenServerCommand(SymbolTable *sym, ProgramState *state)
        : symTable(sym), programState(state) {}

/** Creates a listening socket, listen to socket with independent thread.
 *
 * @param it points to "OpenServerCommand" token followed by "<port_val>"
 * @return number of tokens to advance in main loop.
 */
int OpenServerCommand::execute(list<string>::iterator it) {
    int port = 0;
    sockaddr_in address{};

    // Parse port token
    ++it;
    port = stoi(*it);

    // OPEN SOCKET
    this->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        programState->turnOff();
        throw "Could not create a socket.";
    }

    address.sin_family = AF_INET; // address is in IPv4
    address.sin_addr.s_addr = INADDR_ANY; // give me any ip alloc'd for my machine
    address.sin_port = htons(port); // more info tirgul 6 page 32

    // BIND
    if (bind(sockfd, (struct sockaddr *) &address, sizeof(address)) == -1) {
        programState->turnOff();
        throw "Could not bind the socket to an IP";
    }

    // LISTEN
    if (listen(sockfd, MAX_CLIENTS) == -1) {
        programState->turnOff();
        throw "Error listening to to port";
    }

    // ACCEPT
    while (!(this->client_sock = accept(sockfd, (struct sockaddr *) &address, (socklen_t *) &address))) {
        if (client_sock == -1) {
            programState->turnOff();
            throw "Error accepting client";
        }
    }

    // IF ALL OK, CREATE LISTENING THREAD
    try {
        thread th(&OpenServerCommand::startListening, this);
        th.detach();
    } catch (const char *e) {
        programState->turnOff();
        throw e;
    }

    return 2;
}

/** Listens to simulator requests 10 times a second and update the variables
 * that were defined to be updated in the '.txt' file.
 *
 * @param client_sock client's socket generated at OpenServerCommand::execute().
 * @param symTable to update.
 */
void OpenServerCommand::startListening() {
    int valRead;
    char buffer[MAX_CHARS];
    string::iterator it;

    // Initialize clock variables for putting thread to sleep.
    chrono::milliseconds duration, timePassed;
    chrono::steady_clock::time_point start, end;
    duration = chrono::milliseconds(100);
    unordered_map<string, int> pathToIndexMap = Parser::parseXml("../generic_small.xml");
    unordered_map<int,float> *currSample = new unordered_map<int,float>;
    unordered_map<int,float> *nextSample = new unordered_map<int,float>;

    // The listening loop, will break only if running will become false.
    while (programState->getState()) {
        // Start clock
        start = chrono::steady_clock::now();

        // Read to buffer until the entire message is received (might be cut midway)
        do {
            valRead = read(client_sock, buffer, MAX_CHARS);

            // Error reading
            if (valRead < 0) {
                programState->turnOff();
                throw "Error reading from simulator.";
            }

            // Parse data from simulator into data.
            string temp(buffer);
            parseSimInput(buffer, currSample);
            int firstEol = temp.find_first_of('\n');
            if(firstEol != 0) {
                parseSimInput(buffer+firstEol, nextSample);
                break;
            }
        } while (programState->getState());

//        this_thread::sleep_for(1500ms);
//        cout <<bufStr1 << "\n" << endl; //TODO clear before submitting

        // If nullptr returned - recieved corrupt data
        if (!currSample->empty()) {
            // Otherwise update variables declared '<-' in the global SymbolTable.
            for (auto pair : symTable->getIngoing()) {
                /* pair.first == "name" , pair.second == "path"
                 * newVals.first == "index", newVals.second == "value"
                 * */
                int index = pathToIndexMap[pair.second];
                symTable->setVariable(pair.first, (*currSample)[index]);
            }
        }

        // End clock and then calculate time passed.
        end = chrono::steady_clock::now();
        timePassed = chrono::duration_cast<chrono::milliseconds>(end - start);

        // Read from server at-most 10 times a second (can be less)
        if (duration.count() - timePassed.count() > 0) {
            this_thread::sleep_for(duration - timePassed);
        }
    }

    close(this->sockfd);
}

/**
 *
 * @param OpenServerCommand sends the incoming string. String looks like:
 * {0.0,32.4,33.1,....} and should have 36 or so floats.
 * @return Map<INDEX, VALUE> with updated values from serer.
 */
void OpenServerCommand::parseSimInput(char buffer[MAX_CHARS], unordered_map<int,float>* map) {
    string incoming(buffer), temp;
    int index = 0;

    // Check if all chars are valid.
    if(incoming.find_first_not_of("0123456789.,") != string::npos) {
        return;
    }

    for(char c : incoming) {
        if(c == '\n') {
            map->emplace(index,stof(temp));
            return;
        } else if(c == ',') {
            map->emplace(index,stof(temp));
            index++;
            temp.clear();
            continue;
        } else {
            temp+=c;
        }
    }
}

