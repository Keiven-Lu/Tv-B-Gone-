#include "EU_US_CODE.h"
#include <avr/sleep.h>

void xmitCodeElement(uint16_t ontime, uint16_t offtime, uint8_t PWM_code );
void quickflashLEDx( uint8_t x );
void delay_ten_us(uint16_t us);
void quickflashLED( void );
uint8_t read_bits(uint8_t count);

#define putstring_nl(s) Serial.println(s)
#define putstring(s) Serial.print(s)
#define putnum_ud(n) Serial.print(n, DEC)
#define putnum_uh(n) Serial.print(n, HEX)

#define MAX_WAIT_TIME 65535 

extern const IrCode* const NApowerCodes[] PROGMEM;
extern const IrCode* const EUpowerCodes[] PROGMEM;
extern uint8_t num_NAcodes, num_EUcodes;

void xmitCodeElement(uint16_t ontime, uint16_t offtime, uint8_t PWM_code )
{
  TCNT2 = 0;
  if(PWM_code) {
    pinMode(IRLED, OUTPUT);
    TCCR2A = _BV(COM2A0) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
    TCCR2B = _BV(WGM22) | _BV(CS21);
  }
  else {
    digitalWrite(IRLED, HIGH);
  }
  delay_ten_us(ontime);

TCCR2A = 0;
  TCCR2B = 0;

  digitalWrite(IRLED, LOW);


  delay_ten_us(offtime);
}


uint8_t bitsleft_r = 0;
uint8_t bits_r=0;
PGM_P code_ptr;


uint8_t read_bits(uint8_t count)
{
  uint8_t i;
  uint8_t tmp=0;

 
  for (i=0; i<count; i++) {

    if (bitsleft_r == 0) {

      bits_r = pgm_read_byte(code_ptr++);

      bitsleft_r = 8;
    }

    bitsleft_r--;

    tmp |= (((bits_r >> (bitsleft_r)) & 1) << (count-1-i));
  }

  return tmp;
}



#define BUTTON_PRESSED 0 

uint16_t ontime, offtime;
uint8_t i,num_codes;
uint8_t region;

void setup()   
{
  Serial.begin(9600);

  TCCR2A = 0;
  TCCR2B = 0;

  digitalWrite(LED, LOW);
  digitalWrite(IRLED, LOW);
  pinMode(LED, OUTPUT);
  pinMode(IRLED, OUTPUT);
  pinMode(REGIONSWITCH, INPUT_PULLUP);
  pinMode(TRIGGER, INPUT_PULLUP);

  delay_ten_us(5000); 

  if (digitalRead(REGIONSWITCH)) {
    region = NA;
    DEBUGP(putstring_nl("NA"));
  }
  else {
    region = EU;
    DEBUGP(putstring_nl("EU"));
  }


  DEBUGP(putstring("\n\rNA Codesize: ");
  putnum_ud(num_NAcodes);
  );
  DEBUGP(putstring("\n\rEU Codesize: ");
  putnum_ud(num_EUcodes);
  );

  if (region == NA)
    quickflashLEDx(3);
  else 
    quickflashLEDx(6);
}

void sendAllCodes() 
{
  bool endingEarly = false;  
      

  if (digitalRead(REGIONSWITCH)) {
    region = NA;
    num_codes = num_NAcodes;
  }
  else {
    region = EU;
    num_codes = num_EUcodes;
  }


  for (i=0 ; i<num_codes; i++) 
  {
    PGM_P data_ptr;


    DEBUGP(putstring("\n\r\n\rCode #: ");
    putnum_ud(i));


    if (region == NA) {
      data_ptr = (PGM_P)pgm_read_word(NApowerCodes+i);
    }
    else {
      data_ptr = (PGM_P)pgm_read_word(EUpowerCodes+i);
    }

    DEBUGP(putstring("\n\rAddr: ");
    putnum_uh((uint16_t)data_ptr));


    const uint8_t freq = pgm_read_byte(data_ptr++);

    OCR2A = freq;
    OCR2B = freq / 3; // 33% duty cycle

    DEBUGP(putstring("\n\rOCR1: ");
    putnum_ud(freq);
    );
    DEBUGP(uint16_t x = (freq+1) * 2;
    putstring("\n\rFreq: ");
    putnum_ud(F_CPU/x);
    );


    const uint8_t numpairs = pgm_read_byte(data_ptr++);
    DEBUGP(putstring("\n\rOn/off pairs: ");
    putnum_ud(numpairs));


    const uint8_t bitcompression = pgm_read_byte(data_ptr++);
    DEBUGP(putstring("\n\rCompression: ");
    putnum_ud(bitcompression);
    putstring("\n\r"));


    PGM_P time_ptr = (PGM_P)pgm_read_word(data_ptr);
    data_ptr+=2;
    code_ptr = (PGM_P)pgm_read_word(data_ptr);



#if 0

    for (uint8_t k=0; k<numpairs; k++) {
      uint8_t ti;
      ti = (read_bits(bitcompression)) * 4;

      ontime = pgm_read_word(time_ptr+ti);
      offtime = pgm_read_word(time_ptr+ti+2);
      DEBUGP(putstring("\n\rti = ");
      putnum_ud(ti>>2);
      putstring("\tPair = ");
      putnum_ud(ontime));
      DEBUGP(putstring("\t");
      putnum_ud(offtime));
    }
    continue;
#endif


    cli();
    for (uint8_t k=0; k<numpairs; k++) {
      uint16_t ti;

 
      ti = (read_bits(bitcompression)) * 4;

     
      ontime = pgm_read_word(time_ptr+ti);  // read word 1 - ontime
      offtime = pgm_read_word(time_ptr+ti+2);  // read word 2 - offtime

      xmitCodeElement(ontime, offtime, (freq!=0));
    }
    sei();


    bitsleft_r=0;


    quickflashLED();
    
    
    delay_ten_us(20500);

  
    if (digitalRead(TRIGGER) == BUTTON_PRESSED) 
    {
      endingEarly = true;
      delay_ten_us(50000); //500ms delay 
      quickflashLEDx(4);
      
      delay_ten_us(MAX_WAIT_TIME); // wait 655.350ms
      delay_ten_us(MAX_WAIT_TIME); // wait 655.350ms
      break; //exit the POWER code "for" loop
    }
  } 

  if (endingEarly==false)
  {
   
    delay_ten_us(MAX_WAIT_TIME); // wait 655.350ms
    delay_ten_us(MAX_WAIT_TIME); // wait 655.350ms
    quickflashLEDx(8);
  }
} 

void loop() 
{
  sleepNow();
  
  if (digitalRead(TRIGGER) == BUTTON_PRESSED) 
  {
    delay_ten_us(50000);  
    sendAllCodes();
  }
}


void delay_ten_us(uint16_t us) {
  uint8_t timer;
  while (us != 0) {

    for (timer=0; timer <= DELAY_CNT; timer++) {
      NOP;
      NOP;
    }
    NOP;
    us--;
  }
}



void quickflashLED( void ) {
  digitalWrite(LED, HIGH);
  delay_ten_us(3000);   // 30 ms ON-time delay
  digitalWrite(LED, LOW);
}


void quickflashLEDx( uint8_t x ) {
  quickflashLED();
  while(--x) {
    delay_ten_us(25000);     
    quickflashLED();
  }
}





void sleepNow()
{
  set_sleep_mode(TRIGGER);                    

  sleep_enable();                             

  attachInterrupt(0, wakeUpNow, LOW);        
  // wakeUpNow when pin 2 gets LOW

  sleep_mode();                               
  // THE PROGRAM CONTINUES FROM HERE ON WAKE

  sleep_disable();                            

  detachInterrupt(0);                         
                                              
}

void wakeUpNow()
{

}
