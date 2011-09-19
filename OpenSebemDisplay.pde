#include "Display.h"

// Configuration for display (can not be modified at runtime! The Display class will keep referencing this array)
// Os itens do seu display. (Nao alterar durante a execucao)
int dsp_cfg[] = { DSP_LED, DSP_ALPHA }; //Adicione neste vetor os componentes do seu display. As opcoes sao: DSP_7SEG, DSP_7SEGINV, DSP_ALPHA, DSP_LED, DSP_M5X7:
int dsp_len = sizeof(dsp_cfg) / sizeof(int);

/**
 * Cria um objeto Display. 
 * @param  clock     inteiro que indica o Pino para Clock
 * @param  latch     inteiro que indica o Pino para Latch
 * @param  data      inteiro que indica o Pino de entrada de dados
 * @param  dsp_cfg   array com os componentes do display
 * @param  dsp_len   inteiro com a quantidade de componentes do display
 */
Display dsp(12, 8, 11, dsp_cfg, dsp_len);

void setup() {
  #ifdef DBG_0  
  Serial.begin(9600);
  #endif
  dsp.set(0, 1);
  dsp.set(1, ' ');
  dsp.update();
}


int count = 0;
int ledState = 0;

void loop() {
  int i;
  for (i=0;i<=16;i++){
    dsp.set(1, i);
    dsp.update();
    delay(1000);
  }    

  for (i=0x21;i<=0x7e;i++){
    dsp.set(1, i);
    dsp.update();
    delay(1000);
  } 
  /*
  for (i='A';i<='Z';i++){
    dsp.set(1, i);
    dsp.update();
    delay(1000);
  }
  for (i='a';i<='z';i++){
    dsp.set(1, i);
    dsp.update();
    delay(1000);
  }
  */
}


