// message.h

#ifndef MESSAGE_H
#define MESSAGE_H

#include <iostream>
#include <vector>

enum class instruct_t {
    PING,                   // ...
    INVITE,                 // ...
    ECHO_MSG,               // ...
    MESSAGE,                // Send a message to a specific usr
    TO_ALL,                 // Send a message to all the connected usr
    CREATE_LOBBY,           // ...
    JOIN_LOBBY,             // ...
    START_GAME,             // ...
    GET_CONNECTED_USR,      // to retrieve the list of the connected usr
    SIGN_IN,
    CHECK_TOKEN,// ...
    NONE
};

enum class category_t {
    SOCIAL,
    GAME,
    NONE
};

struct header {
    instruct_t  instruction;        // renamed from header to avoid naming conflict
    category_t  category;           // category of the message related
    int         size;               // the size of the body
    int         sender_id;          // id of the sender
};

struct message {
    header msg_header;              // renamed from header to avoid naming conflict
    std::vector<u_int8_t> body;     // content of the message

    // [DEBUG] allowing to see msg printed in a good format !
    friend std::ostream& operator<<(std::ostream &os, const message &msg) {
        os << "[MESSAGE]:\n" 
           << "\t[INSTRUCT]: " << static_cast<int>(msg.msg_header.instruction) << "\n"
           << "\t[CATEGORY]: " << static_cast<int>(msg.msg_header.category) << "\n"
           << "\t[BODY SIZE]: " << msg.msg_header.size << "\n"
           << "\t[SENDER ID]: " << msg.msg_header.sender_id << "\n"
           << "\t[BODY]: " << std::string(msg.body.begin(), msg.body.end())<<"\n";
        return os;
    }
};

// specific for server
struct message_out {
    int session_id;             // to who we want to send the message
    message msg;                // the message
};

#endif // MESSAGE_H