#include <MIDIUSB.h>

// MIDI note numbers for octaves
#define NOTE_C3 48
#define NOTE_Db3 49
#define NOTE_D3 50
#define NOTE_Eb3 51
#define NOTE_E3 52
#define NOTE_F3 53
#define NOTE_Gb3 54
#define NOTE_G3 55
#define NOTE_Ab3 56
#define NOTE_A3 57
#define NOTE_Bb3 58
#define NOTE_B3 59
#define NOTE_C4 60
#define NOTE_Db4 61
#define NOTE_D4 62
#define NOTE_Eb4 63
#define NOTE_E4 64
#define NOTE_F4 65
#define NOTE_Gb4 66
#define NOTE_G4 67
#define NOTE_Ab4 68
#define NOTE_A4 69
#define NOTE_Bb4 70
#define NOTE_B4 71
#define NOTE_C5 72
#define NOTE_Db5 73
#define NOTE_D5 74
#define NOTE_Eb5 75
#define NOTE_E5 76
#define NOTE_F5 77
#define NOTE_Gb5 78
#define NOTE_G5 79
#define NOTE_Ab5 80
#define NOTE_A5 81
#define NOTE_Bb5 82
#define NOTE_B5 83
#define NOTE_C6 84
#define NOTE_Db6 85
#define NOTE_D6 86
#define NOTE_Eb6 87
#define NOTE_E6 88
#define NOTE_F6 89
#define NOTE_Gb6 90
#define NOTE_G6 91
#define NOTE_Ab6 92
#define NOTE_A6 93
#define NOTE_Bb6 94
#define NOTE_B6 95

// LED and Button connections
#define RED_LED_PIN 16
#define GREEN_LED_PIN 15
#define BLUE_LED_PIN 14
#define RED_BUTTON 9
#define GREEN_BUTTON 8
#define BLUE_BUTTON 7

#define MIDI_CHANNEL 0  // actual channel - 1
#define ARPEGGIO_DELAY 90  // Delay in milliseconds
#define NOTE_VELOCITY 120  // Set a fixed velocity for all notes

// Define expanded arpeggios for each button
#define ARPEGGIO_LENGTH 15
byte blueArpeggio[ARPEGGIO_LENGTH] = {NOTE_Db3, NOTE_Eb3, NOTE_F3, NOTE_Ab3, NOTE_C4, NOTE_Db4, NOTE_Eb4, NOTE_F4, NOTE_Ab4, NOTE_C5, NOTE_Db5, NOTE_Eb5, NOTE_F5, NOTE_Ab5, NOTE_C6};
byte greenArpeggio[ARPEGGIO_LENGTH] = {NOTE_Gb3, NOTE_E3, NOTE_Ab3, NOTE_Bb3, NOTE_Db4, NOTE_Gb4, NOTE_E4, NOTE_Ab4, NOTE_Bb4, NOTE_Db5, NOTE_Gb5, NOTE_E5, NOTE_Ab5, NOTE_Bb5, NOTE_Db6};
byte redArpeggio[ARPEGGIO_LENGTH] = {NOTE_F3, NOTE_Eb3, NOTE_G3, NOTE_Ab3, NOTE_C4, NOTE_F4, NOTE_Eb4, NOTE_G4, NOTE_Ab4, NOTE_C5, NOTE_F5, NOTE_Eb5, NOTE_G5, NOTE_Ab5, NOTE_C6};

bool redButtonPressed = false;
bool greenButtonPressed = false;
bool blueButtonPressed = false;

void setup() {
  Serial.begin(9600);

  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);

  pinMode(RED_BUTTON, INPUT_PULLUP);
  pinMode(GREEN_BUTTON, INPUT_PULLUP);
  pinMode(BLUE_BUTTON, INPUT_PULLUP);
}

void loop() {
  checkButtonAndPlayArpeggio(RED_BUTTON, RED_LED_PIN, redArpeggio, MIDI_CHANNEL, &redButtonPressed);
  checkButtonAndPlayArpeggio(GREEN_BUTTON, GREEN_LED_PIN, greenArpeggio, MIDI_CHANNEL, &greenButtonPressed);
  checkButtonAndPlayArpeggio(BLUE_BUTTON, BLUE_LED_PIN, blueArpeggio, MIDI_CHANNEL, &blueButtonPressed);
}

void checkButtonAndPlayArpeggio(int buttonPin, int ledPin, byte* arpeggio, byte channel, bool *buttonPressed) {
  int buttonState = digitalRead(buttonPin);

  if (buttonState == LOW && !(*buttonPressed)) {
    *buttonPressed = true;
    digitalWrite(ledPin, HIGH);
    playArpeggio(buttonPin, arpeggio, channel, ARPEGGIO_LENGTH);
    digitalWrite(ledPin, LOW);
    *buttonPressed = false;
  }
}

void playArpeggio(int buttonPin, byte* notes, byte channel, int length) {
  unsigned long lastNoteTime = 0;
  int i = 0;
  bool ascending = true;
  byte lastNotePlayed = 0;

  while (digitalRead(buttonPin) == LOW) {
    unsigned long currentTime = millis();
    if (currentTime - lastNoteTime >= ARPEGGIO_DELAY) {
      lastNoteTime = currentTime;

      if (lastNotePlayed != 0) {
        sendNoteOff(lastNotePlayed, channel);  // Ensure note off before next note on
      }

      byte noteToPlay = notes[i];
      sendNoteOn(noteToPlay, channel, NOTE_VELOCITY);
      lastNotePlayed = noteToPlay;
      delay(10);  // Short delay for note articulation

      if (ascending) {
        if (i < length - 1) {
          i++;
        } else {
          ascending = false;
        }
      } else {
        if (i > 0) {
          i--;
        } else {
          ascending = true;
        }
      }
    }
  }
  sendNoteOff(lastNotePlayed, channel); // Ensure the last note is turned off
}

void sendNoteOn(byte pitch, byte channel, byte velocity) {
  midiEventPacket_t tx;
  tx.header = 0x09; // Note on message
  tx.byte1 = 0x90 | channel; // Note on status byte + MIDI channel
  tx.byte2 = pitch; // MIDI note number
  tx.byte3 = velocity; // Velocity
  MidiUSB.sendMIDI(tx);
  MidiUSB.flush();
}

void sendNoteOff(byte pitch, byte channel) {
  midiEventPacket_t tx;
  tx.header = 0x08; // Note off message
  tx.byte1 = 0x80 | channel; // Note off status byte + MIDI channel
  tx.byte2 = pitch; // MIDI note number
  tx.byte3 = 0; // Velocity (0 to indicate note release)
  MidiUSB.sendMIDI(tx);
  MidiUSB.flush();
}