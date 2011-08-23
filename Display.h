#ifndef Display_h
#define Display_h

#include <stdlib.h>
#include <inttypes.h>

#define DBG_0     // habilita o debug

// character types
#define DSP_7SEG     0x01   // display de 7-segmentos [catodo comum] 
#define DSP_7SEGINV  0x02   // display de 7-segmentos [anodo comum]  
#define DSP_ALPHA    0x03   // display de 16-segmentos - alfanumerico
#define DSP_LED      0x04   // led
#define DSP_M5X7     0x05   // tela matricial 5x7; Nao implementado

class Display {
public:
  Display(uint8_t clock, uint8_t latch, uint8_t data, int *types, int len); 
  ~Display();
    
  void set(int idx, uint8_t value);   // atualiza na memoria, o valor do componente idx
  void update();    // Atualiza o display fisicamente

private:
  /* data pins used */
  uint8_t  _clock;  //Pino para Clock
  uint8_t  _latch;  //Pino para Latch
  uint8_t  _data;   //Pino para dados
  /* data */
  int      _len;        // Quantidade de componentes para o display 
  int      *_types;     // Ponteiro para o vetor com os tipos de componentes
  uint8_t  *_bytes;     // Ponteiro para a imagem do display 
  int      _byteslen;   // Quantidade de bytes usados para imagem do display 
  int      *_offsets;   // Ponteiro com a posiço que inicia cada componente dentro da imagem do display
};

#endif
