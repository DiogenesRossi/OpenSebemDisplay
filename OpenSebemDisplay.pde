#include "Display.h"

#define BUTTON_PIN 5

// Configuration for display (can not be modified at runtime! The Display class will keep referencing this array)
int dsp_cfg[] = { DSP_LED, DSP_7SEGINV };
int dsp_len = sizeof(dsp_cfg) / sizeof(int);

//Display::Display(clock,latch, data, *types, len)
Display dsp(12, 8, 11, dsp_cfg, dsp_len);

void setup() {
  pinMode(BUTTON_PIN, INPUT);
  digitalWrite(BUTTON_PIN, HIGH);

  dsp.set(0, '0');
  dsp.set(1, '0');
  dsp.update();
}


int count = 0;
int ledState = 0;

void loop() {
  // read the state of the switch
  int reading = digitalRead(BUTTON_PIN);

  if (reading == LOW) {
    delay(200);  // quick debounce :)
    count++;
    if (count > 15) {
      count = 0;
      ledState = !ledState;
      dsp.set(0, '0'+ledState);
    }
    // convert number to hex
    char buf[1];
    sprintf(buf, "%.1X", count);
    dsp.set(1, buf[0]);
    dsp.update();
  }
}

