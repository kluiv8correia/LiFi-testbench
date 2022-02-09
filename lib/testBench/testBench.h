#ifndef TESTBENCH_H
#define TESTBENCH_H

// DEFINES ======================
#include <Arduino.h>
// ==============================


// FUNCTIONS ====================
// convert to binary
void binConvert(uint8_t test, uint8_t* target, const uint8_t grouping=8) {
    for (int i=0; i<grouping; i++) {
        if ((1 << (grouping-1)) & test)
            target[i] = 1;
        else
            target[i] = 0;
        test <<= 1;
    }
}


// display the bit stream
void displayStream(uint8_t* target, size_t size, uint8_t grouping=8) {
    for (int i=0; i<size; ++i) {
        if ((i % grouping) == 0)
            Serial.print(" ");
        Serial.print(target[i]);
    }
}


// generate CRC
void generateCRC(uint8_t* stream, size_t size, const uint8_t* quotient, uint8_t* target, size_t CRC_LEN) {
    uint8_t offset;

    // create a copy
    uint8_t* temp = (uint8_t*) malloc((size-16)*sizeof(uint8_t));
    memcpy(temp, stream, size-16);

    for (int i=0; (i + quotient[0] - quotient[3]) < size; ++i) {
        if(stream[i] == 1) {
            stream[i] = 0;
            for (int j=1; j<4; ++j) {
                offset = quotient[0] - quotient[j];
                stream[i+offset] = (stream[i+offset] == 1) ? 0 : 1;
            }
        }
    }

    // point to the CRC
    target = &stream[size-16];
    memcpy(stream, temp, size-16);
    
    // free the temp
    free(temp);
}


// differential signal
void generate(uint8_t level, const uint8_t light=4, const uint8_t ir=2) {
    digitalWrite(light, level);
    digitalWrite(ir, level);
}


// send the buffer along with start and stop sequences
inline void sendBuffer(uint8_t* buffer, size_t size, size_t bitInterval, const uint8_t light=4, const uint8_t ir=2) {
    // set the light to OFF however keep IR ON
    digitalWrite(light, LOW);

    // wait for 3 bit lengths
    delay(bitInterval * 3);
    digitalWrite(light, HIGH);
    delay(bitInterval);

    // start sending bits
    for (int i=0; i<size; ++i) {
        generate(buffer[i]);
        delay(bitInterval);
    }

    // set the IR to LOW
    digitalWrite(light, HIGH);
    digitalWrite(ir, LOW);
    delay(bitInterval * 3);

    // set the light back to IDLE
    digitalWrite(light, LOW);
    delay(bitInterval);

    // reset to normal
    digitalWrite(light, HIGH);
    digitalWrite(ir, HIGH);
}
// ==============================

#endif