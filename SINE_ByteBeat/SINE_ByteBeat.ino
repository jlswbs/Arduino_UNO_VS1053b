// VS1053b DSP hack - ByteBeat sine generator //

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
long t;
uint8_t l, r;

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

  l = 20*t*t*(t>>11)*(t<<2^t>>10)-(t|t>>9);
  r = 16*t*t*(t>>11)*(t<<0^t>>10)-(t|t>>9);

  l = l | r;
  r = r ^ l;

  uint16_t left = map(l, 0, 255, 20*coef, 8000*coef);
  uint16_t right = map(r, 0, 255, 20*coef, 8000*coef);

  WriteRegister(SCI_AICTRL0, left);
  WriteRegister(SCI_AICTRL1, right);

  t++;

  delay(1);
    
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