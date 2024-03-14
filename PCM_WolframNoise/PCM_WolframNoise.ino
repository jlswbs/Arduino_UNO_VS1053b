// VS1053b PCM mode stereo 8bit 22050 Hz - Wolfram 1D cellular automata as noise glitch //

#include <SPI.h>

#define MP3_XCS 6
#define MP3_XDCS 7
#define MP3_DREQ 2
#define MP3_RST 8

//VS10xx SCI Registers
#define SCI_MODE 0x00
#define SCI_STATUS 0x01
#define SCI_BASS 0x02
#define SCI_CLOCKF 0x03
#define SCI_DECODE_TIME 0x04
#define SCI_AUDATA 0x05
#define SCI_WRAM 0x06
#define SCI_WRAMADDR 0x07
#define SCI_HDAT0 0x08
#define SCI_HDAT1 0x09
#define SCI_AIADDR 0x0A
#define SCI_VOL 0x0B
#define SCI_AICTRL0 0x0C
#define SCI_AICTRL1 0x0D
#define SCI_AICTRL2 0x0E
#define SCI_AICTRL3 0x0F

#define WIDTH   64
#define BPM     120
#define TICK    (15624L*60) / BPM / 4

  uint8_t l, r;

class Synth {
public:

  uint8_t l;
  uint8_t r;

  bool state[WIDTH];
  bool newst[WIDTH];
  bool rules[8];

	void calculate();
  void setrule(uint8_t rule);

};

void Synth::setrule(uint8_t rule) {

  for (int i=0; i<WIDTH; i++) state[i] = rand()%2;
  for (int n=0; n<8; n++) rules[n] = bitRead(rule, n);

}

void Synth::calculate() {

  memset (newst, 0, sizeof(newst));

  for (int x=0; x<WIDTH; x++) {     
    uint8_t k = 4*state[(x-1+WIDTH)%WIDTH] + 2*state[x] + state[(x+1)%WIDTH];
    newst[x] = rules[k];
  }

  memcpy (state, newst, sizeof(state));

  for (int n = 0; n < 8; n++) {
    
    bitWrite(l, n, state[(WIDTH/2)+n]);
    bitWrite(r, n, state[(WIDTH/2)-n]);
  
  }

}

Synth wolfram;

ISR(TIMER1_COMPA_vect) {

  wolfram.setrule(rand()%256);

}

void setup() {

  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  OCR1A = TICK;
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS12) | (1 << CS10);
  TIMSK1 |= (1 << OCIE1A);
  interrupts();

  pinMode(MP3_RST, OUTPUT);
  pinMode(MP3_DREQ, INPUT);
  pinMode(MP3_XCS, OUTPUT);
  pinMode(MP3_XDCS, OUTPUT);

  SPI.begin();

  SPI.setClockDivider(SPI_CLOCK_DIV16);  // SPI speed 1MHz (16MHz / 16 = 1MHz)
  SPI.transfer(0xFF);

  digitalWrite(MP3_RST, HIGH);
  digitalWrite(MP3_XCS, HIGH);
  digitalWrite(MP3_XDCS, HIGH);

  WriteRegister(SCI_CLOCKF, 0x6000);    // set multiplier to 3.0x
  WriteRegister(SCI_VOL, 0x4F4F);       // set volume

  SPI.setClockDivider(SPI_CLOCK_DIV4);  // set SPI speed 4MHz (16MHz / 4 = 4MHz)

  load_header();
  
}

void loop() {

  while (!digitalRead(MP3_DREQ)) {}

  wolfram.calculate();

  l = wolfram.l;
  r = wolfram.r;

  digitalWrite(MP3_XDCS, LOW);
  SPI.transfer(l);
  SPI.transfer(r);
  digitalWrite(MP3_XDCS, HIGH);

}

void WriteRegister(char address, int data) {

  while (!digitalRead(MP3_DREQ));
  digitalWrite(MP3_XCS, LOW);
  SPI.transfer(0x02);
  SPI.transfer(address);
  SPI.transfer((data >> 8) & 0xFF);
  SPI.transfer(data & 0xFF);
  while (!digitalRead(MP3_DREQ));
  digitalWrite(MP3_XCS, HIGH);

}

const uint8_t wav_header[44] = {

  0x52, 0x49, 0x46, 0x46,  // RIFF
  0xFF, 0xFF, 0xFF, 0xFF,  // size
  0x57, 0x41, 0x56, 0x45,  // WAVE
  0x66, 0x6d, 0x74, 0x20,  // fmt
  0x10, 0x00, 0x00, 0x00,  // subchunk1size
  0x01, 0x00,              // audio format - pcm
  0x02, 0x00,              // numof channels - stereo
  0x22, 0x56, 0x00, 0x00,  // samplerate - 22050
  0x44, 0xac, 0x00, 0x00,  // byterate = samplerate * channels * blocks
  0x02, 0x00,              // blockalign = channels * blocks
  0x08, 0x00,              // bits per sample - 8
  0x64, 0x61, 0x74, 0x61,  // subchunk3id -"data"
  0xFF, 0xFF, 0xFF, 0xFF   // subchunk3size (endless)

};

void load_header() {

  for (int i = 0; i < sizeof(wav_header); i++) {

    uint8_t p = wav_header[i];
    while (!digitalRead(MP3_DREQ)) {}
    digitalWrite(MP3_XDCS, LOW);
    SPI.transfer(p);
    digitalWrite(MP3_XDCS, HIGH);

  }

}