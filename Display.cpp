#include "Display.h"
#include <WProgram.h>

void memcpy_bits(uint8_t  *dst, uint8_t  *src, int dstoffset, int n) {
  uint8_t *bytedst = &dst[(int) dstoffset / 8];
  int bitoffset = (int) dstoffset % 8;
  int bytetot = (int) (n / 8);
  if (bitoffset || !bytetot) {
    bytetot++;
  }
  int copied = 0;
  uint8_t mask;
  for(int i = 0; i < bytetot; i++) {
    uint8_t bytesrc = src[i];
    uint8_t basemask = 0xFF;
    if ((n - copied) < 8) {
      basemask = 0;
      for(int x = 0; x < n-copied; x++) {
        basemask = basemask | (0x80 >> x);
      }
    }
    // copy first byte
    mask = basemask >> bitoffset;
    *dst = *dst & ~mask;
    bytesrc = bytesrc >> bitoffset;
    *dst = *dst | bytesrc;
    if (bitoffset) {
      // if there is an offset, copy the remaining bits to the second byte
      bytesrc = src[i];
      mask = basemask << (8 - bitoffset);
      *(dst+1) = *(dst+1) & ~mask;
      bytesrc = bytesrc << (8 - bitoffset);
      *(dst+1) = *(dst+1) | bytesrc;
    }
    dst++;
    copied += 8;
    if(i < (bytetot-1)) break;
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
        bits_needed += 14;
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
      //TODO:
      memcpy_bits(_bytes, buf, _offsets[idx], 14);
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

void Display::update()
{
    int sent = 0;
    for(int i = _byteslen-1; i >= 0; i--) {
      shiftOut(_data, _clock, LSBFIRST, _bytes[i]);
    }
    digitalWrite(_latch, HIGH);
    delay(1);
    digitalWrite(_latch, LOW);
}

