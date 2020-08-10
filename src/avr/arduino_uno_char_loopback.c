#include <avr/io.h>

#define FOSC 16000000L
#define BAUD 9600
#define MYUBRR (FOSC / 16 / BAUD - 1)

uint8_t byteBuffer;

/** \brief Initialize USART1
 *
 *  \param ubrr: 12 bits: should be clock / 16 / baud - 1
 *
 */
void USART1_Init(uint16_t ubrr);

/** \brief Transmit data with USART1 (blocking)
 *
 *  \param data: data to transmit
 *
 */
void USART1_Tx(uint8_t data);

/** \brief Receive data with USART1 (blocking)
 *
 *  \param statusCounters: should be NULL or a pointer to a 3 element array
 *      The 0 element will be the number of frame errors
 *          1 element will be the number of data overrun errors
 *          2 element will be the number of parity errors
 *
 *  \return the received data
 *
 */
uint8_t USART1_Rx(uint8_t *statusCounters);

void USART1_Init(uint16_t ubrr) {
  // Set Baud
  UBRR0H = (uint8_t)(ubrr >> 8) & 0xFF;
  UBRR0L = (uint8_t)(ubrr)&0xFF;
  UCSR0A = 0;
  // 8 data bits, 1 stop bit, 0 parity bits
  UCSR0C = (3 << UCSZ00);
  // Enable Tx and Rx
  UCSR0B = (1 << RXEN0) | (1 << TXEN0);
}

void USART1_Tx(uint8_t data) {
  while (!(UCSR0A & (1 << UDRE0))) {
    // wait for Tx data register to empty
  }
  UDR0 = data;
}

uint8_t USART1_Rx(uint8_t *statusCounters) {
  while (!(UCSR0A & (1 << RXC0))) {
    // wait for Tx data register to empty
  }
  const uint8_t status = UCSR0A;
  if (statusCounters && status & (1 << FE0))
    statusCounters[0]++;
  if (statusCounters && status & (1 << DOR0))
    statusCounters[1]++;
  if (statusCounters && status & (1 << UPE0))
    statusCounters[2]++;
  return UDR0;
}

/////////////////////////////////////////////

int main(void) {

  USART1_Init(MYUBRR);

  while (1) {
    byteBuffer = USART1_Rx(0);
    USART1_Tx(byteBuffer);
  }
  return 0;
}
