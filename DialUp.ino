#include "config.h"
#include "send.h"

unsigned long last_timestamp;
unsigned long current_timestamp;

byte current_signal;
byte received = 0;
unsigned int shift_counter = 0;
unsigned int multiplier;

String rec = "";
boolean rdy = 0;
boolean active = 0;

String transfer_string = "";
unsigned int transfer_string_len;
char characters[300];

void send_signature() {
  /* Signature: 11000101 */

  PORTL |= _BV(0);
  delayCycle();

  PORTL |= _BV(0);
  delayCycle();

  PORTL &= ~(_BV(0));
  delayCycle();

  PORTL &= ~(_BV(0));
  delayCycle();

  PORTL &= ~(_BV(0));
  delayCycle();
  
  PORTL |= _BV(0);
  delayCycle();

  PORTL &= ~(_BV(0));
  delayCycle();

  PORTL |= _BV(0);
  delayCycle();
}

/* PIN 53 photodiode, PIN 49 laser */
void setup() {
  Serial.begin(9600);
  Serial.setTimeout(10);
  DDRB = B00000000;
  DDRL = B11111111;

  last_timestamp = micros();
}

void loop() {

  while((PINB & B00000001) == current_signal) {
    
    if((PINB & B00000001) == 0 && shift_counter == 0  && !active && Serial.available() > 0) {

      send_signature();
      PORTL &= ~(_BV(0));
      delayCycle();
      
      transfer_string = Serial.readString();
      transfer_string_len = transfer_string.length();
      transfer_string.toCharArray(characters,300);

      send_signature();
      
      for(unsigned int i=0; i<transfer_string_len; i++) {
        for(int j=7; j>=0;j--) {
          if((characters[i] >> j) & B00000001) {  // ON
            PORTL |= _BV(0);
            delayCycle(); 
          }
          else {  // OFF
            PORTL &= ~(_BV(0));
            delayCycle();
          }
        }
      }
      
      send_signature();
      
      PORTL &= ~(_BV(0));
      delayCycle();
      
      transfer_string = "";
    }
  }
  
  current_timestamp = micros();
  
  if(current_signal == 1) {
    multiplier = ((current_timestamp-last_timestamp-CORRECTION_OFFSET)/CYCLE);
  }
  else {
    multiplier = ((current_timestamp-last_timestamp+CORRECTION_OFFSET)/CYCLE);
  }
  if(multiplier < 9) {
    for(int i=0; i<multiplier; i++) {
      received = received << 1;
      received |= current_signal;
      
      shift_counter++;
      
      if(!active && !rdy && received == B11000101) {
        rdy = 1;
        shift_counter = 0;
      }
      else if(rdy && !active && received == B11000101) {
        active = 1;
        shift_counter = 0;
        //Serial.println("Message start");
      }
      
      if(shift_counter == 8) {
        if(active && received == B11000101) {
          active = 0;
          //Serial.println("Message end");
          Serial.println(rec);
          rec = "";
          delay(1000);
        }
        else if(active) {
          rec += String((char)received);
        }
        shift_counter = 0;
      }
    }
  }
  
  current_signal = (PINB & B00000001);
  last_timestamp = current_timestamp;
  multiplier = 0;
}
