// VS1053b MIDI stream hack - Logistic equation music //

#include <SPI.h>

#define MP3_XCS   6
#define MP3_XDCS  7
#define MP3_DREQ  2
#define MP3_RST   8

//VS10xx SCI Registers
#define SCI_MODE        0x00
#define SCI_STATUS      0x01
#define SCI_BASS        0x02
#define SCI_CLOCKF      0x03
#define SCI_DECODE_TIME 0x04
#define SCI_AUDATA      0x05
#define SCI_WRAM        0x06
#define SCI_WRAMADDR    0x07
#define SCI_HDAT0       0x08
#define SCI_HDAT1       0x09
#define SCI_AIADDR      0x0A
#define SCI_VOL         0x0B
#define SCI_AICTRL0     0x0C
#define SCI_AICTRL1     0x0D
#define SCI_AICTRL2     0x0E
#define SCI_AICTRL3     0x0F

  float r = 3.7f;
  float x = 0.1f;
  bool drum = true;

void setup(){
  
  pinMode(MP3_RST, OUTPUT);
  pinMode(MP3_DREQ, INPUT);
  pinMode(MP3_XCS, OUTPUT);
  pinMode(MP3_XDCS, OUTPUT);
 
  SPI.begin();

  SPI.setClockDivider(SPI_CLOCK_DIV16); // SPI speed 1MHz (16MHz / 16 = 1MHz)
  SPI.transfer(0xFF);

  digitalWrite(MP3_RST, HIGH);
  digitalWrite(MP3_XCS, HIGH);
  digitalWrite(MP3_XDCS, HIGH);
  
  WriteRegister(SCI_CLOCKF, 0x8BE8);    // set multiplier to 3.5x
  WriteRegister(SCI_VOL, 0x3F3F);       // set volume
  
  SPI.setClockDivider(SPI_CLOCK_DIV4);  // set SPI speed 4MHz (16MHz / 4 = 4MHz)

  load_header();

}

void loop(){

  float nx = x;
  x = r * nx * (1.0f - nx);
  uint8_t out = 127.0f * x;

  while(!digitalRead(MP3_DREQ)){}
   
  digitalWrite(MP3_XDCS, LOW);

  SPI.transfer(0x5E);
  SPI.transfer(0x80);
  SPI.transfer(0x4C);
  SPI.transfer(0x14);
  SPI.transfer(0x62);
  SPI.transfer(0x90 + (out%3));
  SPI.transfer(map(out, 0, 127, 24, 64));
  SPI.transfer(48 + rand()%64);

  if(drum){

    SPI.transfer(0x00);
    SPI.transfer(0x99);
    SPI.transfer(map(out,0,127,35,72));
    SPI.transfer(96);

  } else {

    SPI.transfer(0x0);
    SPI.transfer(0x99);
    SPI.transfer(35);
    SPI.transfer(112);

  }

  digitalWrite(MP3_XDCS, HIGH);

  drum = !drum;

}

void WriteRegister(char address, int data){
  
  while(!digitalRead(MP3_DREQ));
  digitalWrite(MP3_XCS, LOW);
  SPI.transfer(0x02);
  SPI.transfer(address);
  SPI.transfer((data >> 8) & 0xFF);
  SPI.transfer(data & 0xFF);
  while(!digitalRead(MP3_DREQ));
  digitalWrite(MP3_XCS, HIGH);

}

const uint8_t midi_header[] = {

  // Header chunk
  0x4D,0x54,0x68,0x64,0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x01,0x01,0xE0,
  // Track chunk
  0x4D,0x54,0x72,0x6B,0x00,0x00,0x02,0xC3,
  // Midi events
  0x00,0xFF,0x51,0x03,0x07,0xA1,0x20,0x00,0xFF,
  0x58,0x04,0x04,0x02,0x08,0x08,
  0x00,0xC0,0x0D,       // chan 1 instrument
  0x00,0xB0,0x0A,0x20,  // chan 1 pan
  0x00,0xB0,0x5B,0x60,  // chan 1 reverb
  0x00,0xC1,0x64,       // chan 2 instrument
  0x00,0xB1,0x0A,0x40,  // chan 2 pan
  0x00,0xB1,0x5B,0x60,  // chan 2 reverb
  0x00,0xC2,0x6C,       // chan 3 instrument
  0x00,0xB2,0x0A,0x60,  // chan 3 pan
  0x00,0xB2,0x5B,0x60   // chan 3 reverb

};

void load_header(){

  for (int i = 0; i < sizeof(midi_header); i++){

    uint8_t p = midi_header[i];
    while(!digitalRead(MP3_DREQ)) {}
    digitalWrite(MP3_XDCS, LOW);
    SPI.transfer(p);
    digitalWrite(MP3_XDCS, HIGH);
      
  }

}