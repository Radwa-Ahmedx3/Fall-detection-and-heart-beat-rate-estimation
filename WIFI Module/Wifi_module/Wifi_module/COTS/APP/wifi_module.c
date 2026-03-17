#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>
#include "lcd.h"

#define BAUD 115200

#define BLYNK_AUTH_TOKEN "tXaS2EX1wjwMv1SvCIz-si2XfgGck52e"

// Initialize UART
void UART_Init() {
    UCSRA = (1 << U2X);
    uint16_t ubrr_val = (F_CPU / 8 / BAUD) - 1;
    UBRRH = (unsigned char)(ubrr_val >> 8);
    UBRRL = (unsigned char)ubrr_val;
    UCSRB = (1 << TXEN) | (1 << RXEN); // TX and RX enabled
    UCSRC = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0);
}

// Send a single character
void UART_Transmit(char data) {
    while (!(UCSRA & (1 << UDRE)));
    UDR = data;
}

// Send a full string
void UART_SendString(const char* str) {
    while (*str) {
        UART_Transmit(*str++);
    }
}

// NEW FUNCTION: Listen for a specific response with a timeout
uint8_t UART_WaitFor(const char* target) {
    uint8_t targetLen = strlen(target);
    uint8_t index = 0;

    // A timeout counter. At 16MHz, this gives the ESP a few seconds to reply
    uint32_t timeout = 2000000;

    while (timeout > 0) {
        // Check if a new character has arrived in the RX buffer
        if (UCSRA & (1 << RXC)) {
            char c = UDR; // Read the character

            // Check if it matches the letter we are looking for
            if (c == target[index]) {
                index++;
                if (index == targetLen) {
                    return 1; // Success! We found the target string!
                }
            } else {
                index = 0; // Sequence broke, start over
                if (c == target[0]) index = 1; // Catch if this new letter is the start of the target
            }
        }
        timeout--;
    }
    return 0; // Failure/Timeout: The ESP never replied
}

int main(void) {
    UART_Init();
    LCD_init();
    LCD_displayString("code begin");

    // Give the ESP-01 time to fully power up
    _delay_ms(3000);

    // Set ESP-01 to Station Mode
    UART_SendString("AT+CWMODE=1\r\n");
    UART_WaitFor("OK"); // Wait for ESP to confirm it changed modes

    // Connect to Wi-Fi
    UART_SendString("AT+CWJAP=\"nana\",\"01023025564\"\r\n");
    LCD_clearScreen();
    LCD_displayString("wifi connecting");

    // Wait for the ESP to say "WIFI GOT IP" (means connection is fully established)
    if (UART_WaitFor("WIFI GOT IP")) {
        LCD_clearScreen();
        LCD_displayString("wifi connected!");
    } else {
        LCD_clearScreen();
        LCD_displayString("wifi failed :(");
        while(1); // Halt the program if Wi-Fi fails
    }

    char httpRequest[150];
    char cipSendCmd[30];
    char lcdBuffer[16];
    int counter = 0;

    while(1) {
        sprintf(httpRequest, "GET /external/api/update?token=%s&v1=%d HTTP/1.1\r\nHost: blynk.cloud\r\nConnection: close\r\n\r\n", BLYNK_AUTH_TOKEN, counter);

        // 1. Open TCP connection
        UART_SendString("AT+CIPSTART=\"TCP\",\"blynk.cloud\",80\r\n");
        UART_WaitFor("OK"); // Instantly moves on the millisecond the connection opens!

        // 2. Send byte length
        sprintf(cipSendCmd, "AT+CIPSEND=%d\r\n", strlen(httpRequest));
        UART_SendString(cipSendCmd);

        // 3. Wait for the ESP to reply with the ">" prompt indicating it's ready for data
        UART_WaitFor(">");

        // 4. Send the actual HTTP request
        UART_SendString(httpRequest);

        // 5. Wait for the ESP to confirm the data was sent successfully
        UART_WaitFor("SEND OK");

        // Update the LCD screen
        LCD_clearScreen();
        sprintf(lcdBuffer, "Sent: %d", counter);
        LCD_displayString(lcdBuffer);

        counter++;
        if (counter > 100) counter = 0;

        // The ONLY delay left in the program.
        // We need this so we don't spam the Blynk servers and get blocked.
        // 1.5 seconds is very safe for Blynk's rate limits.
        _delay_ms(1500);
    }
    return 0;
}
