#ifndef __Timer_H_
#define __Timer_H_
#include <Arduino.h>

class Timer {
private:
  unsigned long start;
  unsigned long last;

public:
  Timer();
  unsigned long now() { return millis(); }
  boolean elapsed(unsigned long milliseconds);
  void reset();
};

#endif