void delayCycle() {
  unsigned int us = CYCLE;
  us <<= 2;

  __asm__ __volatile__ (
          "1: sbiw %0,1" "\n\t" // 2 cycles
          "brne 1b" : "=w" (us) : "0" (us) // 2 cycles
  );
}

typedef void state_fn(struct State *);

struct State {
  state_fn * next;
} State;

