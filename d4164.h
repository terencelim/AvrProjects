#ifndef D4164_H
#define D4164_H

#define CAS _BV(7)
#define WE _BV(5)

void WriteBit(uint8_t ra, uint8_t ca, uint8_t bit);
char ReadBit(uint8_t ra, uint8_t ca);

void WriteByte(uint8_t ra, uint8_t ca, uint8_t byte);
char ReadByte(uint8_t ra, uint8_t ca);

#endif
