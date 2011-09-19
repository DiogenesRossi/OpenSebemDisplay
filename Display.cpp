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
  Serial.print("bitoffset: ");
  Serial.print(bitoffset);
  Serial.print("\n");

  Serial.print("dstidx: ");
  Serial.print(dstidx);
  Serial.print("\n");

  Serial.print("minbytes: ");
  Serial.print(minbytes);
  Serial.print("\n");

  int copied = 0;
  uint8_t mask;
  // loop through the bytes to be modified
  for(int i = 0; i < minbytes; i++) {
    uint8_t bytesrc = src[i];
    uint8_t basemask = 0xFF;  // default copy all bits
    // if bits left are less than 8, define the mask
    if ((n - copied) < 8) {
      // TODO: use mask array for optimization
      //basemask = 0;
      //for(int x = 0; x < n-copied; x++) {
      //  basemask = basemask | (0x80 >> x);
      //}
      basemask = maskTable[n - copied];
    }
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
      Diagram
           --------------------------  
           |       0       1        |  
       1 - |     ------  ------     | - a
       0 - |    | \    |    / |     | - COM
       7 - |  7 |  \ 8 | 9 / a|  2  | - b
       8 - |    |   \  |  /   |     | - 2
       6 - |    f------  ------b    | - f
       9 - |    |   /  |  \   |     | - 3
       d - |  6 |  / e | d \ c|  3  | - NC 
       e - |    | /    |    \ |     | - 5
       c - |     ------  ------     | - 4
           |       5       4        |
           --------------------------
          
      byte[0]                  byte[1]                  byte[3]
      07,06,05,04,03,02,01,00  07,06,05,04,03,02,01,00  07      [ bit sequence ]
      02,01,15,13,11,10,05,03, 14,16,04,06,18,09,07,08, nc      [pinout on shiftout ]
      01,18,16,13,09,10,08,04, 05,15,03,02,17,14,06,07, 12      [pinouts on display ]
                */
      switch(value) {
        case '\"':buf[0] = 0x01; buf[1] = 0x40; break;
        case '!': buf[0] = 0x03; buf[1] = 0x00; break;
        case '#': buf[0] = 0x3c; buf[1] = 0x55; break;  //
        case '$': buf[0] = 0xdd; buf[1] = 0x55; break;
        case '%': buf[0] = 0x01; buf[1] = 0x2a; break;
        case '&': buf[0] = 0xce; buf[1] = 0xa9; break;  //
        case '\'':buf[0] = 0x40; buf[1] = 0x00; break;  //
        case '(': buf[0] = 0x00; buf[1] = 0x28; break;  //
        case ')': buf[0] = 0x00; buf[1] = 0x82; break;  //
        case '*': buf[0] = 0x00; buf[1] = 0xff; break;
        case '+': buf[0] = 0x00; buf[1] = 0x55; break;  //
        case ',': buf[0] = 0x00; buf[1] = 0x02; break;  //
        case '-': buf[0] = 0x00; buf[1] = 0x11; break;  //
        case '.': buf[0] = 0x00; buf[1] = 0x08; break;  //
        case '/': buf[0] = 0x00; buf[1] = 0x22; break;  //
        case '0': buf[0] = 0xff; buf[1] = 0x22; break;
        case '1': buf[0] = 0x00; buf[1] = 0x44; break;
        case '2': buf[0] = 0xee; buf[1] = 0x11; break;
        case '3': buf[0] = 0xfc; buf[1] = 0x10; break;  //
        case '4': buf[0] = 0x31; buf[1] = 0x11; break;  //
        case '5': buf[0] = 0xdd; buf[1] = 0x11; break;  //
        case '6': buf[0] = 0xdf; buf[1] = 0x11; break;  //
        case '7': buf[0] = 0xc0; buf[1] = 0x24; break;  //
        case '8': buf[0] = 0xff; buf[1] = 0x11; break;  //
        case '9': buf[0] = 0xfd; buf[1] = 0x11; break;  //
        case ':': buf[0] = 0x00; buf[1] = 0x44; break;  //
        case ';': buf[0] = 0x00; buf[1] = 0x42; break;  //
        case '<': buf[0] = 0x00; buf[1] = 0x28; break;  //
        case '=': buf[0] = 0x04; buf[1] = 0x01; break;  //
        case '>': buf[0] = 0x00; buf[1] = 0x82; break;  //
        case '?': buf[0] = 0xc1; buf[1] = 0x24; break;  //
        case '@': buf[0] = 0xfb; buf[1] = 0x14; break;  //
        case 'A': buf[0] = 0xf3; buf[1] = 0x11; break;
        case 'B': buf[0] = 0xfc; buf[1] = 0x54; break;  //
        case 'C': buf[0] = 0xcf; buf[1] = 0x00; break;  //
        case 'D': buf[0] = 0xfc; buf[1] = 0x44; break;  //
        case 'E': buf[0] = 0xcf; buf[1] = 0x01; break;  //
        case 'F': buf[0] = 0xc3; buf[1] = 0x01; break;  //
        case 'G': buf[0] = 0xdf; buf[1] = 0x10; break;  //
        case 'H': buf[0] = 0x33; buf[1] = 0x11; break;  //
        case 'I': buf[0] = 0xcc; buf[1] = 0x44; break;  //
        case 'J': buf[0] = 0x3e; buf[1] = 0x00; break;  //
        case 'K': buf[0] = 0x03; buf[1] = 0x29; break;  //
        case 'L': buf[0] = 0x0f; buf[1] = 0x00; break;  //
        case 'M': buf[0] = 0x33; buf[1] = 0xa0; break;  //
        case 'N': buf[0] = 0x33; buf[1] = 0x88; break;  //
        case 'O': buf[0] = 0xff; buf[1] = 0x00; break;  //
        case 'P': buf[0] = 0xe3; buf[1] = 0x11; break;  //
        case 'Q': buf[0] = 0xff; buf[1] = 0x08; break;  //
        case 'R': buf[0] = 0xe3; buf[1] = 0x19; break;  //
        case 'S': buf[0] = 0xdc; buf[1] = 0x90; break;  //
        case 'T': buf[0] = 0xc0; buf[1] = 0x44; break;  //
        case 'U': buf[0] = 0x3f; buf[1] = 0x00; break;  //
        case 'V': buf[0] = 0x03; buf[1] = 0x22; break;  //
        case 'W': buf[0] = 0x33; buf[1] = 0x0a; break;  //
        case 'X': buf[0] = 0x00; buf[1] = 0xaa; break;  //
        case 'Y': buf[0] = 0x00; buf[1] = 0xa4; break;  //
        case 'Z': buf[0] = 0xcc; buf[1] = 0x22; break;  //
        case '[': buf[0] = 0x87; buf[1] = 0x00; break;  //
        case '\\':buf[0] = 0x00; buf[1] = 0x88; break;  //
        case ']': buf[0] = 0x78; buf[1] = 0x00; break;  //
        case '^': buf[0] = 0x81; buf[1] = 0x40; break;  //
        case '_': buf[0] = 0x0c; buf[1] = 0x00; break;  //
        case '`': buf[0] = 0x40; buf[1] = 0x00; break;  //
        case 'a': buf[0] = 0x86; buf[1] = 0x45; break;  //
        case 'b': buf[0] = 0x07; buf[1] = 0x05; break;  //
        case 'c': buf[0] = 0x06; buf[1] = 0x01; break;  //
        case 'd': buf[0] = 0x06; buf[1] = 0x45; break;  //
        case 'e': buf[0] = 0x87; buf[1] = 0x41; break;  //
        case 'f': buf[0] = 0x83; buf[1] = 0x01; break;  //
        case 'g': buf[0] = 0x85; buf[1] = 0x45; break;  //
        case 'h': buf[0] = 0x03; buf[1] = 0x05; break;  //
        case 'i': buf[0] = 0x00; buf[1] = 0x04; break;  //
        case 'j': buf[0] = 0x04; buf[1] = 0x44; break;  //
        case 'k': buf[0] = 0x00; buf[1] = 0x6c; break;  //
        case 'l': buf[0] = 0x00; buf[1] = 0x44; break;  //
        case 'm': buf[0] = 0x12; buf[1] = 0x15; break;  //
        case 'n': buf[0] = 0x02; buf[1] = 0x05; break;  //
        case 'o': buf[0] = 0x06; buf[1] = 0x05; break;  //
        case 'p': buf[0] = 0x83; buf[1] = 0x41; break;  //
        case 'q': buf[0] = 0x81; buf[1] = 0x45; break;  //
        case 'r': buf[0] = 0x02; buf[1] = 0x01; break;  //
        case 's': buf[0] = 0x85; buf[1] = 0x05; break;  //
        case 't': buf[0] = 0x07; buf[1] = 0x01; break;  //
        case 'u': buf[0] = 0x06; buf[1] = 0x04; break;  //
        case 'v': buf[0] = 0x02; buf[1] = 0x02; break;  //
        case 'w': buf[0] = 0x12; buf[1] = 0x0a; break;  //
        case 'x': buf[0] = 0x00; buf[1] = 0xaa; break;  //
        case 'y': buf[0] = 0x01; buf[1] = 0x45; break;  //
        case 'z': buf[0] = 0x04; buf[1] = 0x03; break;  //
        case '{': buf[0] = 0x84; buf[1] = 0x83; break;  //
        case '|': buf[0] = 0x00; buf[1] = 0x44; break;  //
        case '}': buf[0] = 0x48; buf[1] = 0x38; break;  //
        case '~': buf[0] = 0xc0; buf[1] = 0x00; break;  //
        default : 
          buf[0] = 0x00; buf[1] = 0x00;
          if (value < 8){
             buf[0]=0x80>>value;
          }else if (value < 17){
             buf[1]=0x80>>value-8;
          }
          break;
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

