#include "Display.h"

// Configuration for display (can not be modified at runtime! The Display class will keep referencing this array)
// Os itens do seu display. (Nao alterar durante a execucao)
int dsp_cfg[] = { DSP_7SEGINV,  DSP_LED, DSP_7SEGINV }; //Adicione neste vetor os componentes do seu display. As opcoes sao: DSP_7SEG, DSP_7SEGINV, DSP_ALPHA, DSP_LED, DSP_M5X7:
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
}


int count = 0;
int ledState = 0;

void loop() {
    delay(500);
    count++;
    if (count > 15) {
      count = 0;
      ledState = !ledState;
      dsp.set(1, '0'+ledState);
    }
    if (ledState){
      // convert number to hex
      char buf[1];
      sprintf(buf, "%.1X", count);
      dsp.set(0, buf[0]);
    }
    dsp.update();
}


