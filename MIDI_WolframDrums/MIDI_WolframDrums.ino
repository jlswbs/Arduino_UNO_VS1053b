// VS1053b MIDI stream hack - Wolfram cellular automata drums //

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

#define WIDTH   256 // CA long bits

  bool state[WIDTH];
  bool newstate[WIDTH];
  uint8_t k;
  bool rules[8] = {0, 1, 1, 1, 1, 0, 0, 0};

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

  for (int x=0; x<WIDTH; x++) state[x] = rand()%2;

}

void loop(){

  memset (newstate, 0, WIDTH);

  for (int x=0; x<WIDTH; x++){
        
    k = 4*state[(x-1+WIDTH)%WIDTH] + 2*state[x] + state[(x+1)%WIDTH];
    newstate[x] = rules[k];

  }

  memcpy (state, newstate, WIDTH);

  while(!digitalRead(MP3_DREQ)){}
   
  digitalWrite(MP3_XDCS, LOW);

  SPI.transfer(0x5E);
  SPI.transfer(0x80);
  SPI.transfer(0x4C);
  SPI.transfer(0x14);
  SPI.transfer(0x62);
  SPI.transfer(0x99);
  SPI.transfer(33);
  SPI.transfer(48 + rand()%64);
  
  if (state[0] == 1){

    SPI.transfer(0x00);
    SPI.transfer(0x99);
    SPI.transfer(35);
    SPI.transfer(48 + rand()%64);
    
  }

  if (state[1] == 1){

    SPI.transfer(0x00);
    SPI.transfer(0x99);
    SPI.transfer(36);
    SPI.transfer(48 + rand()%64);
    
  }

  if (state[2] == 1){

    SPI.transfer(0x00);
    SPI.transfer(0x99);
    SPI.transfer(37);
    SPI.transfer(48 + rand()%64);
    
  }

  if (state[3] == 1){

    SPI.transfer(0x00);
    SPI.transfer(0x99);
    SPI.transfer(38);
    SPI.transfer(48 + rand()%64);
    
  }

  if (state[4] == 1){

    SPI.transfer(0x00);
    SPI.transfer(0x99);
    SPI.transfer(39);
    SPI.transfer(48 + rand()%64);
    
  }

  if (state[5] == 1){

    SPI.transfer(0x00);
    SPI.transfer(0x99);
    SPI.transfer(40);
    SPI.transfer(48 + rand()%64);
    
  }

  if (state[6] == 1){

    SPI.transfer(0x00);
    SPI.transfer(0x99);
    SPI.transfer(41);
    SPI.transfer(48 + rand()%64);
    
  }

  if (state[7] == 1){

    SPI.transfer(0x00);
    SPI.transfer(0x99);
    SPI.transfer(42);
    SPI.transfer(48 + rand()%64);
    
  }

  if (state[8] == 1){

    SPI.transfer(0x00);
    SPI.transfer(0x99);
    SPI.transfer(54);
    SPI.transfer(48 + rand()%64);
    
  }

  if (state[9] == 1){

    SPI.transfer(0x00);
    SPI.transfer(0x99);
    SPI.transfer(60);
    SPI.transfer(48 + rand()%64);
    
  }

  digitalWrite(MP3_XDCS, HIGH);

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
  0x58,0x04,0x04,0x02,0x08,0x08

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