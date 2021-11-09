#include <MIDI.h>

MIDI_CREATE_DEFAULT_INSTANCE();

const int led = 13;

const int pir = 9;
const int zero_cross_pin = 3;
const int lamps[] = {6, 5};

bool events_enable = false;
bool pir_state = false;

void handle_note_on(byte channel, byte note, byte velocity) {
  if(note == 60) {
    // analogWrite(3, velocity);
    set_lamp_value(velocity * 8, 0);
  } else if(note == 61) {
    // analogWrite(5, velocity);
  } else if(note == 62) {
    // analogWrite(6, velocity);
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
    // analogWrite(3, value * 2);
    set_lamp_value(value * 8, 0);
  } else if(number == 2) {
    // analogWrite(5, value * 2);
  } else if(number == 3) {
    // analogWrite(6, value * 2);
  }
}

int lamps_value[] = {2000, 2000};

void zero_cross() {
  delayMicroseconds(lamps_value[0]);
  // digitalWrite(lamps[0], HIGH);
  digitalWrite(lamps[1], HIGH);
  delayMicroseconds(100);
  // digitalWrite(lamps[0], LOW);
  digitalWrite(lamps[1], LOW);
}

void set_lamp_value(int value, int idx) {
  lamps_value[idx] = 2000 + (1023 - value) * 6;
}


void setup() {
  // put your setup code here, to run once:
  pinMode(led, OUTPUT);

  pinMode(lamps[0], OUTPUT);
  pinMode(lamps[1], OUTPUT);

  pinMode(pir, INPUT);

  attachInterrupt(digitalPinToInterrupt(zero_cross_pin), zero_cross, RISING);

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
  
  /*for(size_t i = 0; i < sizeof(notes)/sizeof(notes[0]); i++) {
    MIDI.sendNoteOn(notes[i], 127, 2);
    delay(250);
    MIDI.sendNoteOff(notes[i], 127, 2);
    delay(100);
  }*/

  /*digitalWrite(lamps[0], HIGH);
  delay(500);
  digitalWrite(lamps[0], LOW);
  delay(500);
  digitalWrite(lamps[1], HIGH);
  delay(500);
  digitalWrite(lamps[1], LOW);
  delay(500);*/
  /*for(int i = 0; i < 1023; i += 10) {
    set_lamp_value(i, 0);
    delay(10);
  }*/
}
