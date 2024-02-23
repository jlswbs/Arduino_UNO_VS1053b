// VS1053b DSP hack - Sine noise glitch generator //

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

float coef = 1.4865f; // coefficient for frequency
uint16_t out;
int16_t lp1, lp2, bp2, lowns, sah, rndm, lfrt;

void setup() {
  
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
  WriteRegister(SCI_VOL, 0x5F5F);       // left, right volume 0xFE-0x0
  WriteRegister(SCI_BASS, 0x0000);      // bass, treble control
  
  SPI.setClockDivider(SPI_CLOCK_DIV4);  // set SPI speed 4MHz (16MHz / 4 = 4MHz)

  WriteRegister(SCI_AUDATA, 0xAC44);    // set sample rate 44100 Hz 
  WriteRegister(SCI_AIADDR, 0x4020);    // run new sine test

}

void loop() {

  rndm = rand();
    
  if (lfrt-- == 0) {
    lfrt = (rndm & 511) + 2048;
    sah = rndm;
  }

  WriteRegister(SCI_AUDATA, sah);
    
  bp2 = (sah/5 - bp2/128 - lp2) / 2 + bp2;
  lp2 = bp2/2 + lp2 + sah;                   
  lowns += (rndm-lowns) / 5  + (rndm / 40);
  lp1 += (rndm/8 - lp1)  /  9;

  out = 128 + ((lp1 + bp2/3 + lowns/20) / 64);

  out = map(out, 0, 255, 20*coef, 20000*coef);

  WriteRegister(SCI_AICTRL0, out);
  WriteRegister(SCI_AICTRL1, -out);
    
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