#include "Display.h"
#include <WProgram.h>

char maskTable[9]={0x00,0x80,0xc0,0xe0,0xf0,0xf8,0xfc,0xfe,0xff};

/**
 * Atualiza o dado de um componente do display. 
 *    Apenas atualiza a imagem da mémoria do display. 
 *    Display::update() copia da imagem na memória para o display físico.
 * @param  *dst       ponteiro para imagem da memória do display
 * @param  *src       ponteiro com o novo dado a ser atualizado
 * @param  dstoffset  posição do primeiro bit referente ao componente, na imagem da memória do display
 * @param  n          quantidade de bits usado pelo componente
 */
void memcpy_bits(uint8_t  *dst, uint8_t  *src, int dstoffset, int n) {
  /* A imagem tem que ser atualizada de byte em byte, um componente pode usar 1 ou vários bytes, 
     sendo que os outros bits dentro do mesmo byte não utilizados devem ser preservados
  */
  int bitoffset = (int) dstoffset % 8;
  int dstidx = (int) (dstoffset / 8);
  int minbytes = (int) (n / 8); // quantidade de bytes minima a atualizar no dst
  if (bitoffset || !minbytes) {
    minbytes++; // se o offset for maior que 0 usa um byte +
  }
  int copied = 0;
  uint8_t mask;
  // loop through the bytes to be modified
  for(int i = 0; i < minbytes; i++) {
    uint8_t bytesrc = src[i];
 /*  
    uint8_t basemask = 0xFF;  // default copy all bits
    // if bits left are less than 8, define the mask
    if ((n - copied) < 8) {
      // TODO: use mask array for optimization
      basemask = 0;
      for(int x = 0; x < n-copied; x++) {
        basemask = basemask | (0x80 >> x);
      }
    }
*/    
    uint8_t basemask = maskTable[n - copied];
    // copy first byte
    mask = basemask >> bitoffset;
    dst[dstidx] = dst[dstidx] & ~mask; // zero masked bits
    bytesrc = bytesrc >> bitoffset;
    dst[dstidx] = dst[dstidx] | bytesrc; // copy shifted bits from source
    if (bitoffset) {
      // if there is an offset, copy the remaining bits to the second byte
      bytesrc = src[i];
      // TODO: mask = ~mask should also work
      mask = basemask << (8 - bitoffset);
      dst[dstidx+1] = dst[dstidx+1] & ~mask;
      bytesrc = bytesrc << (8 - bitoffset);
      dst[dstidx+1] = dst[dstidx+1] | bytesrc;
    }
    dstidx++;
    copied += 8;
    // the last byte is already modified
    if(i >= (minbytes-1)) break;
  }
}

Display::Display(uint8_t clock, uint8_t latch, uint8_t data, int *types, int len)
{
  _clock = clock;
  _latch = latch;
  _data = data;
  _types = types;
  _len = len;
  _offsets = (int *)calloc(_len, sizeof(int));
  int bits_needed = 0;
  for(int i = 0; i < _len; i++) {
    _offsets[i] = bits_needed;  // save accumulated bit start position
    switch(_types[i]) {
      case DSP_7SEG:
      case DSP_7SEGINV:
        bits_needed += 8;
        break;
      case DSP_ALPHA:
        bits_needed += 16;
        break;
      case DSP_LED:
        bits_needed += 1;
        break;
      case DSP_M5X7:
        bits_needed += 35;
        break;
    }
  }
  int m = (bits_needed % 8);
  if (m) {
    bits_needed += (8 - m);
  }
  _byteslen = bits_needed / 8;
  _bytes = (uint8_t *)calloc(_byteslen, sizeof(uint8_t));
  // enable data output
  pinMode(clock, OUTPUT);
  pinMode(latch, OUTPUT);
  pinMode(data, OUTPUT);
}

Display::~Display()
{
  free(_offsets);
  free(_bytes);
}

void Display::set(int idx, uint8_t value)
{
  uint8_t    buf[6];
  
  memset(buf, 0, sizeof(buf));
  switch(_types[idx]) {
    case DSP_7SEG:
    case DSP_7SEGINV:
      switch(value) {
        case '*': buf[0] = 0xFF; break;
        case '.': buf[0] = 0x01; break;
        case '0': buf[0] = 0xfc; break;
        case '1': buf[0] = 0x60; break;
        case '2': buf[0] = 0xda; break;
        case '3': buf[0] = 0xf2; break;
        case '4': buf[0] = 0x66; break;
        case '5': buf[0] = 0xb6; break;
        case '6': buf[0] = 0xbe; break;
        case '7': buf[0] = 0xe0; break;
        case '8': buf[0] = 0xfe; break;
        case '9': buf[0] = 0xf6; break;
        case 'A': buf[0] = 0xee; break;
        case 'a': buf[0] = 0xfa; break;
        case 'B': buf[0] = 0x3e; break;
        case 'b': buf[0] = 0x3e; break;
        case 'c': buf[0] = 0x1a; break;
        case 'C': buf[0] = 0x9c; break;
        case 'D': buf[0] = 0x7a; break;
        case 'd': buf[0] = 0x7a; break;
        case 'e': buf[0] = 0xde; break;
        case 'E': buf[0] = 0x9e; break;
        case 'F': buf[0] = 0x8e; break;
        case 'f': buf[0] = 0x8e; break;
        case 'G': buf[0] = 0xf6; break;
        case 'g': buf[0] = 0xf6; break;
        case 'H': buf[0] = 0x6e; break;
        case 'h': buf[0] = 0x2e; break;
        case 'I': buf[0] = 0x60; break;
        case 'i': buf[0] = 0x08; break;
        case 'J': buf[0] = 0x70; break;
        case 'j': buf[0] = 0x70; break;
        case 'L': buf[0] = 0x1c; break;
        case 'l': buf[0] = 0x0c; break;
        case 'N': buf[0] = 0xec; break;
        case 'n': buf[0] = 0x2a; break;
        case 'O': buf[0] = 0x3a; break;
        case 'o': buf[0] = 0x3a; break;
        case 'P': buf[0] = 0xce; break;
        case 'p': buf[0] = 0xce; break;
        case 'Q': buf[0] = 0xe6; break;
        case 'q': buf[0] = 0xe6; break;
        case 'R': buf[0] = 0x0a; break;
        case 'r': buf[0] = 0x0a; break;
        case 'S': buf[0] = 0xb6; break;
        case 's': buf[0] = 0xb6; break;
        case 'T': buf[0] = 0x1e; break;
        case 't': buf[0] = 0x1e; break;
        case 'U': buf[0] = 0x7c; break;
        case 'u': buf[0] = 0x38; break;
        case 'Y': buf[0] = 0x76; break;
        case 'y': buf[0] = 0x76; break;
        case '-': buf[0] = 0x02; break;
        case '?': buf[0] = 0xcb; break;
        case '=': buf[0] = 0x12; break;
        case '"': buf[0] = 0x44; break;
        default : buf[0] = 0x00; break;
      }
      if (_types[idx] == DSP_7SEGINV) {
        buf[0] = ~buf[0];
      }
      memcpy_bits(_bytes, buf, _offsets[idx], 8);
      break;
    case DSP_ALPHA:
      /* Alpha Numeric Display
          Diagram:                 Pins:
          a1   a2
         ---- ----            A1 - 01  10 - D2
        |\   |   /|            J - 02  11 - Com
      f | \h |j /k| b          H - 03  12 - Dot
        |  \ | /  |            F - 04  13 - C
         -g1- -g2-            G1 - 05  14 - L
        |   /|\   |            M - 06  15 - G2
      e |  /n|m \l| c          N - 07  16 - B
        | /  |   \|            E - 08  17 - K
         ---- ----  . dp      D1 - 09  18 - A2
          d1   d2

      byte[0]                  byte[1]                  byte[3]
      a1,a2,b1,c1,d1,d2,e1,f1, g1,g2,h1,j1,k1,l1,m1,n1, dp      
      07,06,05,04,03,02,01,00  07,06,05,04,03,02,01,00  07      [ bit sequence ]
      02,01,15,13,11,10,05,03, 14,16,04,06,18,09,07,08, nc      [pinout on shiftout ]
      01,18,16,13,09,10,08,04, 05,15,03,02,17,14,06,07, 12      [pinouts on display ]
                */
      switch(value) {
        case ',': buf[0] = 0xff; buf[1] = 0xff; break;
        case '1': buf[0] = 0x38; buf[1] = 0x00; break;
        case 'A': buf[0] = 0xf3; buf[1] = 0xc0; break;
        /*
        case ',': buf[0] = 0x80; buf[1] = 0x00; break;
        case 'A': buf[0] = 0x4c; buf[1] = 0x02; break;
        case 'B': buf[0] = 0x53; buf[1] = 0x08; break;
        case 'C': buf[0] = 0x0f; buf[1] = 0x00; break;
        case 'D': buf[0] = 0x13; buf[1] = 0x08; break;
        case 'E': buf[0] = 0x0f; buf[1] = 0x02; break;
        case 'F': buf[0] = 0x0c; buf[1] = 0x02; break;
        case 'G': buf[0] = 0x4f; buf[1] = 0x00; break;
        case 'H': buf[0] = 0x4c; buf[1] = 0x02; break;
        case 'I': buf[0] = 0x13; buf[1] = 0x08; break;
        case 'J': buf[0] = 0x07; buf[1] = 0x00; break;
        case 'K': buf[0] = 0xac; buf[1] = 0x02; break;
        case 'L': buf[0] = 0x0f; buf[1] = 0x00; break;
        case 'M': buf[0] = 0x2c; buf[1] = 0x20; break;
        case 'N': buf[0] = 0x8c; buf[1] = 0x20; break;
        case 'O': buf[0] = 0x0f; buf[1] = 0x00; break;
        case 'P': buf[0] = 0x4c; buf[1] = 0x02; break;
        case 'Q': buf[0] = 0x8f; buf[1] = 0x00; break;
        case 'R': buf[0] = 0xcc; buf[1] = 0x02; break;
        case 'S': buf[0] = 0x4b; buf[1] = 0x02; break;
        case 'T': buf[0] = 0x10; buf[1] = 0x08; break;
        case 'U': buf[0] = 0x0f; buf[1] = 0x00; break;
        case 'V': buf[0] = 0x2c; buf[1] = 0x10; break;
        case 'W': buf[0] = 0x8c; buf[1] = 0x10; break;
        case 'X': buf[0] = 0xa0; buf[1] = 0x30; break;
        case 'Y': buf[0] = 0x20; buf[1] = 0x28; break;
        case 'Z': buf[0] = 0x23; buf[1] = 0x10; break;
        case '[': buf[0] = 0x0e; buf[1] = 0x00; break;
        case '\\': buf[0] = 0x80; buf[1] = 0x20; break;
        case ']': buf[0] = 0x01; buf[1] = 0x00; break;
        case '^': buf[0] = 0x80; buf[1] = 0x10; break;
        case '_': buf[0] = 0x03; buf[1] = 0x00; break;
        case ' ': buf[0] = 0x00; buf[1] = 0x00; break;
        case '~': buf[0] = 0x12; buf[1] = 0x20; break;
        case '"': buf[0] = 0x18; buf[1] = 0x00; break;
        case '#': buf[0] = 0x53; buf[1] = 0x0a; break;
        case '$': buf[0] = 0x5b; buf[1] = 0x0a; break;
        case '%': buf[0] = 0x79; buf[1] = 0x1a; break;
        case '&': buf[0] = 0x97; buf[1] = 0x22; break;
        case '\'': buf[0] = 0x20; buf[1] = 0x00; break;
        case '(': buf[0] = 0xa0; buf[1] = 0x00; break;
        case ')': buf[0] = 0x05; buf[1] = 0x30; break;
        case '*': buf[0] = 0xf0; buf[1] = 0x3a; break;
        case '+': buf[0] = 0x50; buf[1] = 0x0a; break;
        case ';': buf[0] = 0x80; buf[1] = 0x40; break;
        case '-': buf[0] = 0x40; buf[1] = 0x02; break;
        case '.': buf[0] = 0x80; buf[1] = 0x00; break;
        case '/': buf[0] = 0x20; buf[1] = 0x10; break;
        case '0': buf[0] = 0x2f; buf[1] = 0x10; break;
        case '1': buf[0] = 0x10; buf[1] = 0x08; break;
        case '2': buf[0] = 0x47; buf[1] = 0x02; break;
        case '3': buf[0] = 0x43; buf[1] = 0x02; break;
        case '4': buf[0] = 0x48; buf[1] = 0x02; break;
        case '5': buf[0] = 0x4b; buf[1] = 0x02; break;
        case '6': buf[0] = 0x4f; buf[1] = 0x02; break;
        case '7': buf[0] = 0x00; buf[1] = 0x00; break;
        case '8': buf[0] = 0x4f; buf[1] = 0x02; break;
        case '9': buf[0] = 0x4b; buf[1] = 0x02; break;
        case '<': buf[0] = 0x43; buf[1] = 0x10; break;
        case '=': buf[0] = 0x43; buf[1] = 0x02; break;
        case '>': buf[0] = 0x83; buf[1] = 0x20; break;
        case '?': buf[0] = 0x40; buf[1] = 0x08; break;
        case '@': buf[0] = 0x07; buf[1] = 0x0a; break;
        */
        default : buf[0] = 0x00; buf[1] = 0x00; break;
      }
      memcpy_bits(_bytes, buf, _offsets[idx], 16);
      break;
    case DSP_LED:
      if (value == '0') {
        buf[0] = 0x00;
      } else {
        buf[0] = 0x80;
      }
      memcpy_bits(_bytes, buf, _offsets[idx], 1);
      break;
    case DSP_M5X7:
      buf[0] = 0xFF;
      buf[1] = 0xFF;
      buf[2] = 0xFF;
      buf[3] = 0xFF;
      buf[4] = 0xFF;
      memcpy_bits(_bytes, buf, _offsets[idx], 35);
      break;
  }
}

char *toBinary(int value, char *ret){
  int i;
  for (i=0;i<8;i++){
    ret[i]=((value & 0x80)==0x80?'1':'0');  //0x80="10000000"B
    value=(value<<1);
  }
  ret[8]='\0';
  return ret;
}

void Display::update()
{
    char buf[16];
    int sent = 0;
    for(int i = _byteslen-1; i >= 0; i--) {
      shiftOut(_data, _clock, LSBFIRST, _bytes[i]);
#ifdef DBG_0
      toBinary(_bytes[i], buf);
      Serial.print("line: %d");
      Serial.print(i);
      Serial.print(" = ");
      Serial.print(buf);
      sprintf(buf," 0x%.2x\n", _bytes[i]);
      Serial.print(buf);
#endif
    }
    digitalWrite(_latch, HIGH);
    delay(1);
    digitalWrite(_latch, LOW);
}

