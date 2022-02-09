// INCLUES ======================
#include <Arduino.h>

#include "testBench.h"
// ==============================


// DEFINES ======================
#define BAUDRATE 115200
#define BIT_INTERVAL 10

#define IR 2
#define LIGHT 4

#define CHAR_LIMIT 1024
#define BYTE_LENGTH 8
#define CRC_LEN 16
#define ARBITRATION_LEN 15

// ==============================



// GLOBAL CONSTANTS =============
const uint8_t quotient[] = {16, 12, 5, 0};
const uint8_t id[] = {0, 0, 1, 0, 1, 0, 1, 1, 1, 0, 0};
// ==============================


// GLOBAL VARS ==================
char testString[1024];
uint8_t* stream;
// ==============================


void setup () {
    Serial.begin(BAUDRATE);

    // welcome screen
    Serial.println("\n----------------------- LiFi TESTBENCH -----------------------\n");

    // set pinModes
    pinMode(IR, OUTPUT);
    pinMode(LIGHT, OUTPUT);

    // set initial states for pins
    digitalWrite(IR, HIGH);
    digitalWrite(LIGHT, HIGH);
}
void loop() {
    // get the string from the user
    size_t size;
    Serial.print("Enter the test string: ");
    while (!Serial.available());
    size = Serial.readBytesUntil('\r', testString, CHAR_LIMIT);
    testString[size] = '\0'; // add the null character at the end
    Serial.printf("%s\n[size: %d]\n", testString, size);

    // split the string into 128-16 bit wide bytes and schedule sends
    size_t npackets = ceil((float) size / 14);
    Serial.printf("Sending %d packets over LiFi ---------------------------------------\n\n", npackets);
    for (int i=0; i<npackets; ++i) {
        // allocate the required bit stream
        Serial.printf("Allocate...%dth packet\n", i+1);
        size_t rows = (i == (npackets-1)) ? size - ((npackets-1)*14) : 14;
        stream = (uint8_t*) malloc((rows*BYTE_LENGTH+CRC_LEN+ARBITRATION_LEN) * sizeof(uint8_t)); // including the CRC bytes

        // convert to binary and set to allocated memory
        Serial.println("Binary Convert...");
        Serial.print("start point: "); Serial.println(i*14+0);
        Serial.print("end point: "); Serial.println(i*14+(rows-1));
        for (int j=0; j<rows; ++j)
            binConvert(testString[i*14+j], &stream[ARBITRATION_LEN+j*BYTE_LENGTH]);
        
        // set the 16 bit CRC field to 0
        for (int j=0; j<16; ++j)
            stream[ARBITRATION_LEN+j+(rows*BYTE_LENGTH)] = 0;

        // display stream
        Serial.print("stream Only:");
        displayStream(&stream[ARBITRATION_LEN], rows*BYTE_LENGTH);
        Serial.println();

        // generate the CRC
        Serial.print("CRC Remainder:");
        generateCRC(&stream[ARBITRATION_LEN], rows*BYTE_LENGTH+16, quotient, &stream[ARBITRATION_LEN+rows*BYTE_LENGTH], CRC_LEN);
        displayStream(&stream[ARBITRATION_LEN+rows*BYTE_LENGTH], 16);
        Serial.println();

        // create the arbitration field
        Serial.print("Arbitration Field:");
        memcpy(stream, id, 11);
        binConvert(rows+1, &stream[11], 4); // CRC must also be INCLUDED and remove the top length offset
        displayStream(stream, ARBITRATION_LEN, 11);
        Serial.println();

        // display whole packet
        Serial.print("Packet Content:");
        displayStream(stream, ARBITRATION_LEN, 100);
        displayStream(&stream[ARBITRATION_LEN], rows*BYTE_LENGTH+CRC_LEN, 8);
        Serial.println();

        // send the packet
        Serial.println("Sending Packet...");
        sendBuffer(stream, rows*BYTE_LENGTH+CRC_LEN+ARBITRATION_LEN, BIT_INTERVAL);

        // free the memory
        free(stream);
        Serial.println();

        // delay before sending next package
        delay(BIT_INTERVAL*7);
    }
    Serial.println("--------------------------------------------------------------------\n");
}