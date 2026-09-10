#ifndef SERIAL_MSG_READER_H
#define SERIAL_MSG_READER_H

#include "hardware_config.h"

class SerialMsgReader {
    private:
        char buffer[MESSAGE_LENGTH];
        int index = 0;
        bool discarding = false;

    public:
        bool add_char(char c, char* message);
};

#endif