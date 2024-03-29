// VS1053b DSP hack sine - Arcade drums //

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

#define SIZE    180
#define BPM     120
#define TICK    (15624L*60) / BPM / 2

  uint16_t outx;
  uint16_t outy;
  bool sw = true;

class Synth {
public:

  bool trig = 0;
  bool noise = 0;
  int type = 0;
  int wave = 0;
  int i = 0;
  int k = 1;

  int16_t fand[SIZE] = { 0, 17, 34, 52, 69, 86, 103, 120, 137, 154, 171, 187, 203, 219, 234, 250, 264, 279, 293, 307, 321, 334, 347, 359, 371,  383,  394,  404,  414,  424,  433,  441,  449,  456,  463,  469,  475,  480,  485,  489,  492,  495,  497,  498,  499,  500,  499,  498,  497,  495,  492,  489,  485,  480,  475,  469,  463,  456,  449,  441,  433,  424,  414,  404,  394,  383,  371,  359,  347,  334,  321,  307, 293, 279, 264, 250, 234, 219, 203, 187, 171, 154, 137, 120, 103, 86, 69, 52, 34, 17, 0, -18, -35, -53, -70, -87, -104, -121, -138, -155, -172, -188, -204, -220, -235, -250, -265, -280, -294, -308, -322, -335, -348, -360, -372, -384, -395, -405, -415, -425, -434, -442, -450, -457, -464, -470, -476, -481, -486, -490, -493, -496, -498, -499, -500, -500, -500, -499, -498, -496, -493, -490, -486, -481, -476, -470, -464, -457, -450, -442, -434, -425, -415, -405, -395, -384, -372, -360, -348, -335, -322, -308, -294, -280, -265, -250, -235, -220, -204, -188, -172, -155, -138, -121, -104, -87, -70, -53, -35, -18 };
  int16_t harmo[SIZE] = { 0, 416, 365, 321, 334, 307, 279, 279, 254, 232, 227, 203, 185, 178, 155, 140, 131, 110, 97, 88, 68, 57, 48, 30, 21, 11, -4, -11,  -21,  -34,  -40,  -48,  -60,  -63,  -71,  -80,  -82,  -88,  -95,  -95,  -101, -104, -103, -107, -109, -106, -109, -107, -103, -104, -101, -95,  -95,  -88,  -82,  -80,  -71,  -63,  -60,  -48,  -40,  -34,  -21,  -11,  -4, 11, 21, 30, 48, 57, 68, 88, 97, 110,  131,  140, 155, 178, 185, 203, 227, 232, 254, 279, 279, 307, 334, 321, 365, 416, 0, -417, -366, -322, -335, -308, -280, -280, -255, -233, -228, -204, -186, -179, -156, -141, -132, -111, -98,  -89,  -69,  -58,  -49,  -31,  -22,  -12,  3,  10, 20, 33, 39, 47, 59, 62, 70, 79, 81, 87, 94, 94, 100,  103,  102,  106,  108,  105,  108,  106,  102,  103,  100,  94, 94, 87, 81, 79, 70, 62, 59, 47, 39, 33, 20, 10, 3,  -12,  -22,  -31,  -49,  -58,  -69,  -89,  -98,  -111, -132, -141, -156, -179, -186, -204, -228, -233, -255, -280, -280, -308, -335, -322, -366, -417 };
  int16_t rnd[SIZE];

	int calculate();

};

int Synth::calculate() {

  if (trig == 1) {

    if (noise) for (int j = 0; j < SIZE; j++) rnd[j] = rand() % SIZE;
    else for (int j = 0; j < SIZE; j++) rnd[j] = (rand() % SIZE) / j;

    i = 0;
    k = 1;
    trig = 0;

  }

  if (i > SIZE-1) {

    i = 0;
    k++;

  }

  if (type == 0) wave = 512 + (fand[i]+harmo[i]);
  if (type == 1) wave = 512 + (fand[i]+(rnd[i]>>2));
  if (type == 2) wave = 512 + rnd[i];
  
  i ++;

  return wave / k;

}

Synth kick;

ISR(TIMER1_COMPA_vect) {

  kick.trig = 1;
  kick.noise = rand() % 2;
  kick.type = rand() % 3;
  sw = !sw;

}

void setup(){

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

  SPI.setClockDivider(SPI_CLOCK_DIV16); // SPI speed 1MHz (16MHz / 16 = 1MHz)
  SPI.transfer(0xFF);

  digitalWrite(MP3_RST, HIGH);
  digitalWrite(MP3_XCS, HIGH);
  digitalWrite(MP3_XDCS, HIGH);
  
  WriteRegister(SCI_CLOCKF, 0x6000);    // set multiplier to 3.0x
  WriteRegister(SCI_VOL, 0x5F5F);       // set volume
  
  SPI.setClockDivider(SPI_CLOCK_DIV4);  // set SPI speed 4MHz (16MHz / 4 = 4MHz)

  WriteRegister(SCI_AUDATA, 0xAC44);    // set sample rate 44100 Hz 
  WriteRegister(SCI_AIADDR, 0x4020);    // run new sine test

}

void loop(){

  int16_t out = kick.calculate();

  if (sw) {

    outx = map(out, 0, 512, 100, 8000);
    outy = map(-out, 0, 512, 100, 7000);

  } else {
    
    outx = 100;
    outy = 100;

  }

  WriteRegister(SCI_AICTRL0, outx); // left channel frequency 
  WriteRegister(SCI_AICTRL1, outy); // right channel frequency

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