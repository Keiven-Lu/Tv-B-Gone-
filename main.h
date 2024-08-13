#include <avr/pgmspace.h>

#define NA 1
#define EU 0

#define LED LED_BUILTIN 
#define IRLED 3
#define TRIGGER 2
#define REGIONSWITCH 5

#define NUM_ELEM(x) (sizeof (x) / sizeof (*(x)));

#define DEBUG 0
#define DEBUGP(x) if (DEBUG == 1) { x ; }

#define NOP __asm__ __volatile__ ("nop")

#define DELAY_CNT 25

#define freq_to_timerval(x) (F_CPU / 8 / x - 1)

struct IrCode {
  uint8_t timer_val;
  uint8_t numpairs;
  uint8_t bitcompression;
  uint16_t const *times;
  uint8_t const *codes;
};
