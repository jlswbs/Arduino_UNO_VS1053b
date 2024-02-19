// MacMillan fractal map music //

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

#define BPM 120

  float x = 0.1f;
  float y = 0.1f;
  float m = 1.6f;
  float e = 0.5f;

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
  
  load_code();

  WriteRegister(SCI_CLOCKF, 0x8BE8);  // set multiplier to 3.5x
  WriteRegister(SCI_VOL, 0x3F3F);     // left, right volume 0xFE-0x0
  WriteRegister(SCI_BASS, 0x0000);    // bass, treble control
  
  SPI.setClockDivider(SPI_CLOCK_DIV4); // set SPI speed 4MHz (16MHz / 4 = 4MHz)

  uint8_t room = 0;   // reverb type
  uint8_t rval = 127; // reverb value

  reverb(0, room, rval);
  reverb(1, room, rval);
  reverb(2, room, rval);
  reverb(3, room, rval);
  reverb(4, room, rval);
  reverb(5, room, rval);
 
}

void loop() {

  float nx = x;
  float ny = y;
     
  x = ny;
  y = - nx + 2.0f * m * (ny /(1.0f + powf(ny,2.0f))) + e * ny;
            
  uint8_t xout = 64.0f + (13.6f * x);
  uint8_t yout = 64.0f + (13.6f * y);

  uint8_t poly = yout%6;
  uint8_t prog = xout;
  
  if (prog == 19) prog = 0;   // replace church organ to grand piano
  if (prog == 78) prog = 77;  // replace whistle to shakuhachi

  uint8_t vol = 32 + (yout/2);
  uint8_t note = 24 + (xout/2);
  uint8_t pan = 64.0f + (9.0f * x);

  note_on(poly, prog & 123, note & 127, vol & 127);
  
  uint8_t drum = map(yout, 0, 127, 27, 50); // note 35-81 drum kit

  note_on(9, prog & 123, drum & 127, vol & 127);

  int tempo = 60000 / BPM;
  delay(tempo / 4);
  
  uint8_t rel_off = yout%8;
  
  if (rel_off == 0) {all_sound_off(0); panning(0, pan & 127);}
  if (rel_off == 1) {all_sound_off(1); panning(1, pan & 127);}
  if (rel_off == 2) {all_sound_off(2); panning(2, pan & 127);}
  if (rel_off == 3) {all_sound_off(3); panning(3, pan & 127);}
  if (rel_off == 4) {all_sound_off(4); panning(4, pan & 127);}
  if (rel_off == 5) {all_sound_off(5); panning(5, pan & 127);}

  delay(tempo / 4);

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

void note_on(byte chan, byte inst, byte note, byte vol){

  WriteRegister(SCI_MODE, 0x0c00);

  SPI.transfer(0xC0 | chan);
  SPI.transfer(0x00);
  SPI.transfer(inst);
  SPI.transfer(0x00);
  
  SPI.transfer(0x90 | chan);
  SPI.transfer(0x00);
  SPI.transfer(note);
  SPI.transfer(0x00);
  SPI.transfer(vol);
  SPI.transfer(0x00);

}

void note_off(byte chan, byte note){

  WriteRegister(SCI_MODE, 0x0c00);

  SPI.transfer(0x80 | chan);
  SPI.transfer(0x00);
  SPI.transfer(note);
  SPI.transfer(0x00);
  SPI.transfer(0x00);
  SPI.transfer(0x00);
  
}

void reverb(byte chan, byte typ, byte rev){
  
  WriteRegister(SCI_MODE, 0x0c00);

  SPI.transfer(0xB0 | chan);
  SPI.transfer(0x00);
  SPI.transfer(0x5B);
  SPI.transfer(0x00);
  SPI.transfer(rev);
  SPI.transfer(0x00);
  
  SPI.transfer(0xB0 | chan);
  SPI.transfer(0x00);
  SPI.transfer(0x0C);
  SPI.transfer(0x00);
  SPI.transfer(typ);
  SPI.transfer(0x00);

}

void panning(byte chan, byte pan){
  
  WriteRegister(SCI_MODE, 0x0c00);

  SPI.transfer(0xB0 | chan);
  SPI.transfer(0x00);
  SPI.transfer(0x0A);
  SPI.transfer(0x00);
  SPI.transfer(pan);
  SPI.transfer(0x00);

}

void all_sound_off(byte chan){
  
  WriteRegister(SCI_MODE, 0x0c00);

  SPI.transfer(0xB0 | chan);
  SPI.transfer(0x00);
  SPI.transfer(0x78);
  SPI.transfer(0x00);
  SPI.transfer(0xFF);
  SPI.transfer(0x00);

}

void pitch(byte chan, byte pitch){

  WriteRegister(SCI_MODE, 0x0c00);

  SPI.transfer(0xE0 | chan);
  SPI.transfer(0x00);
  SPI.transfer(pitch);
  SPI.transfer(0x00);

}

#define PLUGIN_SIZE 1039

const static word plugin[PLUGIN_SIZE] PROGMEM = { /* Compressed plugin */
  0x0007,0x0001, /*copy 1*/
  0x8050,
  0x0006,0x03f0, /*copy 1008*/
  0x2800,0x8080,0x0006,0x2016,0xf400,0x4095,0x0006,0x0017,
  0x3009,0x1c40,0x3009,0x1fc2,0x6020,0x0024,0x0000,0x1fc2,
  0x2000,0x0000,0xb020,0x4542,0x3613,0x0024,0x0006,0x0057,
  0x3e15,0x1c15,0x0020,0x1fd4,0x3580,0x3802,0xf204,0x3804,
  0x0fff,0xfe44,0xa244,0x1804,0xf400,0x4094,0x2800,0x1985,
  0x3009,0x1bc2,0xf400,0x4500,0x2000,0x0000,0x36f5,0x3c15,
  0x3009,0x3857,0x2800,0x1b40,0x0030,0x0457,0x3009,0x3857,
  0x0030,0x0a57,0x3e14,0xf806,0x3701,0x8024,0x0006,0x0017,
  0x3e04,0x9c13,0x0020,0x1fd2,0x3b81,0x8024,0x36f4,0xbc13,
  0x36f4,0xd806,0x0030,0x0717,0x2100,0x0000,0x3f05,0xdbd7,
  0x0030,0xf80f,0x0000,0x1f0e,0x2800,0x7680,0x0000,0x004d,
  0xf400,0x4595,0x3e00,0x17cc,0x3505,0xf802,0x3773,0x0024,
  0x3763,0x0024,0x3700,0x0024,0x0000,0x09c2,0x6024,0x0024,
  0x3600,0x1802,0x2830,0xf855,0x0000,0x004d,0x2800,0x2240,
  0x36f3,0x0024,0x3613,0x0024,0x3e12,0xb817,0x3e12,0x3815,
  0x3e05,0xb814,0x3625,0x0024,0x0000,0x800a,0x3e10,0x3801,
  0x3e10,0xb803,0x3e11,0x3810,0x3e04,0x7812,0x34c3,0x0024,
  0x3440,0x0024,0x4080,0x0024,0x001b,0x3301,0x2800,0x2c85,
  0x0000,0x0180,0x0000,0x0551,0x0000,0xaf02,0x293c,0x1f40,
  0x0007,0xffc1,0xb010,0x134c,0x0018,0x0001,0x4010,0x10d0,
  0x0007,0xffc1,0xfe20,0x020c,0x0000,0x0591,0x48b6,0x0024,
  0x4dd6,0x0024,0x0001,0x2202,0x293c,0x1f40,0x4380,0x2003,
  0xb010,0x134c,0x0018,0x0001,0x4010,0x1010,0xfe20,0x020c,
  0x48b6,0x844c,0x4dd6,0x0024,0xb880,0x2003,0x3434,0x0024,
  0x2800,0x5280,0x3083,0x0024,0x001c,0xccc2,0x0000,0x05d1,
  0x34d3,0x0024,0x3404,0x0024,0x3404,0x420c,0x3001,0x05cc,
  0xa408,0x044c,0x3100,0x0024,0x6010,0x0024,0xfe20,0x0024,
  0x48b6,0x0024,0x4dd6,0x0024,0x4310,0x0024,0x4488,0x2400,
  0x0000,0x0551,0x2800,0x3295,0x3404,0x0024,0xf290,0x00cc,
  0x3800,0x0024,0x3434,0x0024,0x3073,0x0024,0x3013,0x0024,
  0x2800,0x4340,0x3800,0x0024,0x3083,0x0024,0x3000,0x0024,
  0x6402,0x0024,0x0000,0x1001,0x2800,0x3618,0x0018,0x0002,
  0x3434,0x4024,0x3133,0x0024,0x3100,0x0024,0xfe20,0x0024,
  0x48b6,0x0024,0x4dd6,0x0024,0x2800,0x4340,0x3900,0xc024,
  0x4010,0x1011,0x6402,0x0024,0x0000,0x0590,0x2800,0x3918,
  0x0000,0x0024,0xf290,0x04cc,0x3900,0x0024,0x3434,0x0024,
  0x3073,0x0024,0x3013,0x0024,0x2800,0x4340,0x3800,0x0024,
  0x3183,0x0024,0x3100,0x0024,0x6402,0x0024,0x0000,0x1001,
  0x2800,0x3c98,0x0019,0x9982,0x3434,0x0024,0x3033,0x0024,
  0x3000,0x0024,0xfe20,0x0024,0x48b6,0x0024,0x4dd6,0x0024,
  0x2800,0x4340,0x3800,0xc024,0x4010,0x0024,0x6402,0x0024,
  0x001d,0x7082,0x2800,0x4198,0x0000,0x0024,0xf290,0x1010,
  0x3033,0x0024,0x3800,0x0024,0x3404,0x0024,0x3073,0x0024,
  0x3013,0x0024,0x3800,0x0024,0x0004,0x4d50,0x3010,0x0024,
  0x30f0,0x4024,0x3434,0x4024,0x3143,0x0024,0x3910,0x0024,
  0x2800,0x4340,0x39f0,0x4024,0x3434,0x0024,0x3033,0x0024,
  0x3000,0x0024,0xfe20,0x0024,0x48b6,0x0024,0x4dd6,0x0024,
  0x3800,0xc024,0x001e,0x9982,0x0001,0x1012,0x0000,0x0381,
  0x34d3,0x184c,0x3444,0x0024,0x3073,0x0024,0x3013,0x0024,
  0x3000,0x0024,0xfe20,0x0024,0x48b6,0x0024,0x4dd6,0x0024,
  0x4380,0x3003,0x3400,0x0024,0x293d,0x2900,0x3e00,0x0024,
  0x3009,0x33c0,0x293b,0xc540,0x0010,0x0004,0x34d3,0x184c,
  0x3444,0x0024,0x3073,0x13c0,0x3073,0x0024,0x293b,0xf880,
  0x0001,0x1011,0x0001,0x0010,0x0001,0x1011,0x34d3,0x184c,
  0x3430,0x0024,0x4010,0x0024,0x0000,0x05c1,0x3e10,0x0024,
  0x293b,0xac80,0x0006,0x0092,0x0000,0x05d1,0x36f3,0x134c,
  0x3404,0x0024,0x3083,0x0024,0x3000,0x0024,0x6012,0x0024,
  0x0013,0x3304,0x2800,0x5198,0x0001,0xc682,0x0000,0x0500,
  0x0001,0x0012,0x3404,0x584c,0x3133,0x0024,0x3100,0x4024,
  0x0000,0x05d1,0xfe22,0x0024,0x48b6,0x0024,0x4dd6,0x0024,
  0x3e10,0xc024,0x3430,0x8024,0x4204,0x0024,0x293b,0xb580,
  0x3e00,0x8024,0x36e3,0x134c,0x3434,0x0024,0x3083,0x0024,
  0x3000,0x0024,0x6090,0x0024,0x3800,0x1812,0x36f4,0x4024,
  0x36f1,0x1810,0x36f0,0x9803,0x36f0,0x1801,0x3405,0x9014,
  0x36f3,0x0024,0x36f2,0x1815,0x2000,0x0000,0x36f2,0x9817,
  0x3613,0x0024,0x3e12,0xb817,0x3e12,0x3815,0x3e05,0xb814,
  0x3615,0x0024,0x0000,0x800a,0x3e10,0x3801,0x3e10,0xb804,
  0x3e01,0x7810,0x0008,0x04d0,0x2900,0x1480,0x3001,0x0024,
  0x4080,0x03cc,0x3000,0x0024,0x2800,0x7485,0x4090,0x0024,
  0x0000,0x0024,0x2800,0x6245,0x0000,0x0024,0x0000,0x0081,
  0x3000,0x0024,0x6012,0x0024,0x0000,0x0401,0x2800,0x70c5,
  0x0000,0x0024,0x6012,0x0024,0x0000,0x0024,0x2800,0x6645,
  0x0000,0x0024,0x2900,0x1680,0x0000,0x0024,0x4088,0x008c,
  0x0000,0x2000,0x6400,0x0024,0x0000,0x3c00,0x2800,0x5ed8,
  0x0000,0x0024,0x2800,0x6300,0x3801,0x0024,0x6400,0x038c,
  0x0000,0x0024,0x2800,0x6318,0x0000,0x0024,0x3013,0x0024,
  0x2900,0x1480,0x3801,0x0024,0x4080,0x0024,0x0000,0x0024,
  0x2800,0x6255,0x0000,0x0024,0x6890,0x03cc,0x2800,0x7480,
  0x3800,0x0024,0x2900,0x1680,0x0008,0x0510,0x3800,0x0024,
  0x0000,0x3c00,0x6400,0x0024,0x003f,0xff00,0x2800,0x6b08,
  0x0000,0x0024,0x0000,0x3fc0,0x6400,0x0024,0x0000,0x3c00,
  0x2800,0x73c5,0x6400,0x0024,0x0000,0x0024,0x2800,0x73d5,
  0x0000,0x0024,0xb880,0x184c,0x2900,0x1480,0x3009,0x3800,
  0x4082,0x9bc0,0x6014,0x0024,0x0000,0x3c04,0x2800,0x6941,
  0x0000,0x3dc1,0x2900,0x1680,0x0000,0x0024,0xf400,0x4004,
  0x0000,0x3dc1,0x6412,0x0024,0x0008,0x0490,0x2800,0x6a85,
  0x0000,0x0000,0x0000,0x0400,0x2800,0x7480,0x3800,0x0024,
  0x0008,0x04d0,0x3001,0x4024,0xa50a,0x0024,0x0000,0x03c0,
  0xb50a,0x0024,0x0000,0x0300,0x6500,0x0024,0x0000,0x0024,
  0x2900,0x1488,0x0000,0x6f48,0x0000,0x0380,0x6500,0x0024,
  0x0000,0x0024,0x2800,0x7195,0x0000,0x0024,0x2900,0x1480,
  0x0000,0x0024,0x4080,0x03cc,0x0000,0x0080,0x2800,0x70d5,
  0x0000,0x0024,0x2800,0x7480,0x3800,0x0024,0x2900,0x1680,
  0x0000,0x0024,0x408a,0x0024,0x0008,0x0510,0x3613,0x0024,
  0x3e11,0x4024,0x30f0,0x0024,0x3e10,0x0024,0x3000,0x4024,
  0x2931,0xe080,0x3e00,0x4024,0x36d3,0x0024,0x0000,0x0000,
  0x0008,0x0490,0x3800,0x0024,0x36f1,0x5810,0x36f0,0x9804,
  0x36f0,0x1801,0x3405,0x9014,0x36f3,0x0024,0x36f2,0x1815,
  0x2000,0x0000,0x36f2,0x9817,0x0005,0xbe51,0x0001,0x0010,
  0x3613,0x0024,0x3e05,0xb814,0x3635,0x0024,0x0000,0x800a,
  0xb880,0x104c,0xb882,0x33c0,0x2914,0xbec0,0x0004,0xc580,
  0x0019,0x98c0,0x0004,0x4e90,0x3800,0x0024,0x001f,0xff00,
  0x2931,0x6c40,0x3900,0x0024,0x2931,0x6640,0x0000,0x0024,
  0x2900,0x5500,0x0000,0x8001,0x2912,0x0d00,0x3613,0x0024,
  0x6012,0x0024,0x0000,0x8005,0x2800,0x7b18,0x0004,0x4d50,
  0x2912,0x0d00,0x3613,0x108c,0x2934,0x4180,0x3ce0,0x0024,
  0x0000,0x1000,0x3423,0x0024,0x2900,0x0a80,0x34e1,0x0024,
  0xb882,0x0042,0x30f0,0xc024,0x4dc2,0x0024,0x3810,0x0024,
  0x2800,0x7b00,0x38f0,0x4024,0x3e12,0xb817,0x3e12,0x3815,
  0x3e05,0xb814,0x3615,0x0024,0x0000,0x800a,0x3e10,0x3801,
  0x0000,0x0081,0xb880,0xb811,0x0030,0x0291,0x3e14,0x0024,
  0x0030,0x0690,0x3e14,0xb813,0x0030,0x00d3,0x0007,0x9252,
  0x3800,0x0024,0x3910,0x0024,0x3a00,0x0024,0x0000,0xc0c0,
  0x3900,0x0024,0x0030,0x0000,0x0006,0x0051,0x2908,0x6400,
  0x3b00,0x0024,0xb880,0x008c,0x3800,0x0024,0x3800,0x0024,
  0x0003,0x0d40,0x0006,0xc490,0x2908,0x7f80,0x3009,0x2000,
  0x0030,0x0ad0,0x3800,0x184c,0x002b,0x1100,0x3e10,0x0024,
  0x2909,0xa9c0,0x3e10,0x4024,0x000a,0x8001,0x2908,0x7f80,
  0x36e3,0x0024,0xb880,0x2000,0x0006,0x0010,0x3009,0x2410,
  0x0006,0x0011,0x3009,0x2410,0x0008,0x0490,0x3810,0x0024,
  0x3800,0x0024,0x0000,0x0890,0x290f,0xfcc0,0x0006,0x8380,
  0x000a,0x8001,0x0000,0x0950,0x290f,0xfcc0,0x0006,0xb380,
  0x0000,0x09c0,0x0030,0x0690,0x6890,0x2000,0x0030,0x1310,
  0x6890,0x2000,0x0030,0x0490,0x2900,0x1e00,0x3800,0x0024,
  0x36f4,0x9813,0x36f4,0x1811,0x36f0,0x1801,0x3405,0x9014,
  0x36f3,0x0024,0x36f2,0x1815,0x2000,0x0000,0x36f2,0x9817,
  0x0007,0x0001, /*copy 1*/
  0x5800,
  0x0006,0x0004, /*copy 4*/
  0x1800,0x1800,0x98cc,0x7395,
  0x0007,0x0001, /*copy 1*/
  0x8025,
  0x0006,0x0002, /*copy 2*/
  0x2a00,0x1ace,
  0x0007,0x0001, /*copy 1*/
  0x8022,
  0x0006,0x0002, /*copy 2*/
  0x2a00,0x1a0e,0x000a,0x0001,0x0050,};

void load_code(void){
  
  int i = 0;
  while (i< PLUGIN_SIZE) {
    word addr, n, val;
    addr = pgm_read_word (&(plugin[i++]));
    n = pgm_read_word (&(plugin[i++]));
    if (n & 0x8000U) {
      n &= 0x7FFF;
      val = pgm_read_word (&(plugin[i++]));
      while (n--) {
        WriteRegister(addr , val);       
      }
    } 
    else {
      while (n--) {
        val = pgm_read_word (&(plugin[i++]));
        WriteRegister(addr , val);        
      }
    }
  }
  return;

}