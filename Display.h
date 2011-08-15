#ifndef Display_h
#define Display_h

#include <stdlib.h>
#include <inttypes.h>

// character types
#define DSP_7SEG     0x01
#define DSP_7SEGINV  0x02
#define DSP_ALPHA    0x03
#define DSP_LED      0x04
#define DSP_M5X7     0x05

class Display {
public:
  Display(uint8_t clock, uint8_t latch, uint8_t data, int *types, int len);
  ~Display();
    
  void set(int idx, uint8_t value);
  void update();

private:
  /* data pins used */
  uint8_t  _clock;
  uint8_t  _latch;
  uint8_t  _data;
  /* data */
  int      _len;
  int      *_types;
  uint8_t  *_bytes;
  int      _byteslen;
  int      *_offsets;
};

#endif
