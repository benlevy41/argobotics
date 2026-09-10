#include "SerialMsgReader.h"
#include <cstring>

bool SerialMsgReader::add_char(char c, char* message) {
    if (discarding) { 
        if (c == '\n') {
            discarding = false;
            index = 0;
        }
        return false;
    } 
    
    if (c == '\n') {
        buffer[index] = '\0'; // null terminator for c string
        index = 0;
        strcpy(message, buffer);
        return true; // message complete
    } 
    
    if (index >= MESSAGE_LENGTH - 1) { 
        index = 0;
        buffer[0] = '\0';
        discarding = true;
        return false;
    }

    buffer[index] = c;
    index++;
    return false; // message not complete
}