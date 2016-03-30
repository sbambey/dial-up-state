#include "config.h"
#include "Arduino.h"
#include "send.h"

unsigned long last_timestamp;
unsigned long current_timestamp;

byte current_signal;
byte received = 0;
int shift_counter = 0;

String rec = "";
boolean active = 0;

String transfer_string = "";

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

/* PIN 53 */
void setup() {
  Serial.begin(9600);
  DDRB = B00000000;
  DDRL = B11111111;

  last_timestamp = micros();
}

void loop() {

  while((PINB & B00000001) == current_signal) {
    
    if(Serial.available() > 0) {
      transfer_string = Serial.readString();
    }

    int transfer_string_len = transfer_string.length();
    char characters[transfer_string_len];

    //Transfer string only if there is something to send and there is no incoming message
    if(transfer_string.length() > 0 && !active && shift_counter == 0) {
      transfer_string.toCharArray(characters,transfer_string_len);

      send_signature();
      
      for(int i=0; i<transfer_string.length(); i++) {
        for(int j=0; j<8;j++) {
          if(characters[i] << j & B10000000 == B10000000) {
            PORTL |= _BV(0);
            delayCycle(); 
          }
          else {
            PORTL &= ~(_BV(0));
            delayCycle();
          }
          
          /*if(binary_char.charAt(j) == '1') {
            PORTL |= _BV(0);
            delayCycle();
          }
          else {
            PORTL &= ~(_BV(0));
            delayCycle();
          }*/
        }
      }
      
      send_signature();
      
      PORTL &= ~(_BV(0));
      delayCycle();
      
      transfer_string = "";
    }
  }
  
  int multiplier;
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
      
      if(!active && received == B11000101) {
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
}
