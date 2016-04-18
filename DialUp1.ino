#include "config.h"
#include "send.h"

struct State state;

unsigned long last_timestamp;
unsigned long current_timestamp;

byte current_signal;
byte received = 0;
unsigned int shift_counter = 0;
unsigned int multiplier;

String rec = "";
bool rdy = 0;
bool active = 0;
bool inhibit = 0;

byte sread;
unsigned int c = 0;

String transfer_string = "";
unsigned int transfer_string_len;
char characters[400];

unsigned long timeout;

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



///////////////


state_fn idle, rx, tx;

void idle(struct State * state) {
  
  if((PINB & B00000001) == current_signal) {
    //Serial.println("idle");
    if(Serial.available() > 0) {
      sread = Serial.read();
      if(sread == '\r') {
        characters[c] = sread;
        rdy = 1;
      }
      else {
        characters[c] = sread;
        c++;
      }
      //timeout=millis();
    }

    if(shift_counter == 0  && rdy && !active) {
      state->next = tx;
    }
  }
  else {
    state->next = rx;
  }
}

void rx(struct State * state) {
  //Serial.println("rx");
  while(true) {
    if(!(PINB & B00000001) == current_signal) {
      current_timestamp = micros();
      
      if(current_signal == 1) {
        multiplier = ((current_timestamp-last_timestamp-CORRECTION_OFFSET)/CYCLE);
      }
      else {
        multiplier = ((current_timestamp-last_timestamp+CORRECTION_OFFSET)/CYCLE);
      }
  
      last_timestamp = current_timestamp;
      
      if(multiplier < 9) {
        for(int i=0; i<multiplier; i++) {
          received = received << 1;
          received |= current_signal;
          
          shift_counter++;
          
          if(!active && received == B11000101) {
            active = 1;
            shift_counter = 0;
            Serial.println("{\"a\":\"Message start.\", \"b\":\"System Message\"}");
          }
          
          if(shift_counter == 8) {
            Serial.println(received);
            if(active && received == B11000101) {
              active = 0;
              //Serial.println("Message end");
              Serial.println(rec);
              rec = "";
              state->next = idle;
            }
            else if(active) {
              rec += String((char)received);
            }
            shift_counter = 0;
          }
        }
        multiplier = 0;
      }
      timeout = millis();
    }
    else if((millis()-timeout) > TIMEOUT) {
      Serial.println("{\"a\":\"Message failed. Timeout reached.\", \"b\":\"System Message\"}");
      Serial.print("{\"a\":\"Timeout: ");
      Serial.print(millis()-timeout);
      Serial.println("\", \"b\":\"System Message\"}");
  
      received = B00000000;
      shift_counter = 0;
      active = 0;
      rec = "";
      
      state->next = idle;
    }
    current_signal = (PINB & B00000001);
    if(state->next == idle) break;
  }
}

void tx(struct State * state) {
  //Serial.println("tx");
  send_signature();
      
  for(unsigned int i=0; i<=c; i++) {
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
  rdy = 0;
  c = 0;
  memset(characters, 0, sizeof characters);

  state->next = idle;
}


////////////////


/* PIN 53 photodiode, PIN 49 laser */
void setup() {
  Serial.begin(9600);
  Serial.setTimeout(10);
  DDRB = B00000000;
  DDRL = B11111111;

  //Start state
  state = { idle };
  
  last_timestamp = micros();
}

void loop() {
  state.next(&state);
}
