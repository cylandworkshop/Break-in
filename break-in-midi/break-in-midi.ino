#include <MIDI.h>
#include <TimerOne.h>

MIDI_CREATE_DEFAULT_INSTANCE();

#define LAMP_COUNT 2

const int led = 13;

const int pir = 9;
const int zero_cross_pin = 3;
const int lamps[LAMP_COUNT] = {6, 5};

bool events_enable = false;
bool pir_state = false;

void handle_note_on(byte channel, byte note, byte velocity) {
  if(note == 60) {
    set_lamp_value(velocity, 1);
  } else if(note == 61) {
    set_lamp_value(velocity, 0);
  } else if(note == 63) {
    digitalWrite(led, velocity > 64 ? HIGH : LOW);
  } else if(note == 64) {
    events_enable = true;
    if(pir_state) {
      MIDI.sendNoteOn(65, 127, 2);
    }
  }
}

void handle_note_off(byte channel, byte note, byte velocity) {
  if(note == 64) {
    events_enable = false;
  }
}

void handle_cc(byte channel, byte number, byte value) {
  if(number == 1) {
    set_lamp_value(value, 1);
  } else if(number == 2) {
    set_lamp_value(value, 0);
  }
}

int lamps_value[LAMP_COUNT] = {0, 0};

volatile int t = 0;             // Variable to use as a counter
volatile bool zero_cross = 0; // Boolean to store a "switch" to tell us if we have
const int FREQ_STEP = 73;

void handle_zero_cross() {
  zero_cross = true;
}

void dim_check() {
  if(zero_cross) {
    for(uint8_t i = 0; i < LAMP_COUNT; i++) {
      if(t >= lamps_value[i]) {
        digitalWrite(lamps[i], HIGH);
      }
    }

    if(t > 127) {
      t = 0;
      zero_cross = false;
    } else {
      t++;
    }
  } else {
    for(uint8_t i = 0; i < LAMP_COUNT; i++) {
      digitalWrite(lamps[i], LOW);
    }
  }
}

void set_lamp_value(int value, int idx) {
  lamps_value[idx] = 127 - value;
}


void setup() {
  // put your setup code here, to run once:
  pinMode(led, OUTPUT);

  pinMode(lamps[0], OUTPUT);
  pinMode(lamps[1], OUTPUT);

  pinMode(pir, INPUT);

  attachInterrupt(digitalPinToInterrupt(zero_cross_pin), handle_zero_cross, RISING);
  Timer1.initialize(FREQ_STEP);
  Timer1.attachInterrupt(dim_check, FREQ_STEP);

  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.setHandleNoteOn(handle_note_on);
  MIDI.setHandleNoteOff(handle_note_off);
  MIDI.setHandleControlChange(handle_cc);
  MIDI.turnThruOff();
}

void loop() {
  // put your main code here, to run repeatedly:
  MIDI.read();
  
  if(pir_state) {
    // wait for transient to low
    if(digitalRead(pir) == LOW) {
      MIDI.sendNoteOff(66, 127, 2);
      digitalWrite(led, LOW);
      pir_state = false;
    }
  } else {
    // wait for transient to high
    if(digitalRead(pir) == HIGH) {
      if(events_enable) {
        MIDI.sendNoteOn(65, 127, 2);
      }
      MIDI.sendNoteOn(66, 127, 2);
      digitalWrite(led, HIGH);
      pir_state = true;
    }
  }

  /*for(int i = 0; i < 127; i++) {
    set_lamp_value(127 - i, 0);
    set_lamp_value(i, 1);
    delay(3);
  }*/
}
