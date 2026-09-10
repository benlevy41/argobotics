#include <unity.h>
#include <Arduino.h>
#include "SerialMsgReader.h"
#include "hardware_config.h"

void setUp(void) {
    // Set up before each test
}   

void tearDown(void) {
    // Clean up after each test
}

void test_serial_msg_reader_complete_message() {
    SerialMsgReader reader;
    char message[MESSAGE_LENGTH];
    char input[MESSAGE_LENGTH] = "+75,+13,-20,+00,-15\n"; // throttle, yaw, heave, pan, tilt
    
    bool message_complete = false;

    for (size_t i = 0; i < strlen(input); ++i) {
        message_complete = reader.add_char(input[i], message);
        if (message_complete) {
            break;
        }
    }

    TEST_ASSERT_TRUE(message_complete);
    TEST_ASSERT_EQUAL_STRING("+75,+13,-20,+00,-15", message);
}

void test_serial_msg_reader_incomplete_message() {
    SerialMsgReader reader;
    char message[MESSAGE_LENGTH];
    char input[MESSAGE_LENGTH] = "+75,+13,-20,";
    
    bool message_complete = false;

    for (size_t i = 0; i < strlen(input); ++i) {
        message_complete = reader.add_char(input[i], message);
        if (message_complete) {
            break;
        }
    }

    TEST_ASSERT_FALSE(message_complete);

    char input2[MESSAGE_LENGTH] = "+00,-15\n"; // continue the message
    for (size_t i = 0; i < strlen(input2); ++i) {
        message_complete = reader.add_char(input2[i], message);
        if (message_complete) {
            break;
        }
    }

    TEST_ASSERT_TRUE(message_complete);
    TEST_ASSERT_EQUAL_STRING("+75,+13,-20,+00,-15", message);
}

void test_serial_msg_reader_buffer_overflow() {
    SerialMsgReader reader;
    char message[MESSAGE_LENGTH];
    char input[MESSAGE_LENGTH + 4] = "+75,+13,-20,+00,-15,+55\n"; // throttle, yaw, heave, pan, tilt, extra data
    
    bool message_complete = false;

    for (size_t i = 0; i < strlen(input); ++i) {
        message_complete = reader.add_char(input[i], message);
        if (message_complete) {
            break;
        }
    }

    TEST_ASSERT_FALSE(message_complete);

    char input2[MESSAGE_LENGTH] = "+75,+13,-20,+00,-15\n";
    
    for (size_t i = 0; i < strlen(input2); ++i) {
        message_complete = reader.add_char(input2[i], message);
        if (message_complete) {
            break;
        }
    }

    TEST_ASSERT_TRUE(message_complete);
    TEST_ASSERT_EQUAL_STRING("+75,+13,-20,+00,-15", message);

}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_serial_msg_reader_complete_message);
    RUN_TEST(test_serial_msg_reader_incomplete_message); 
    RUN_TEST(test_serial_msg_reader_buffer_overflow); 

    UNITY_END();
}