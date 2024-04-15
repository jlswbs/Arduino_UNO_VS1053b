// VS1053b DSP code hack - ByteBeat generator with delay //

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

#define BPM   120


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

  WriteRegister(SCI_CLOCKF, 0x8BE8);    // set multiplier to 3.5x
  WriteRegister(SCI_VOL, 0x3F3F);       // left, right volume 0xFE-0x0
  WriteRegister(SCI_BASS, 0x0000);      // bass, treble control
  
  SPI.setClockDivider(SPI_CLOCK_DIV4);  // set SPI speed 4MHz (16MHz / 4 = 4MHz)

  WriteRegister(SCI_AUDATA, 0xAC44);    // set sample rate 44100 Hz
  WriteRegister(SCI_AIADDR, 0x50);      // plugin enable

  WriteRegister(SCI_AUDATA, 8000);      // 8kHz retro sound

}


void loop() {

  uint16_t a = random(1, 256);
  uint16_t b = random(1, 256);
  uint16_t c = random(1, 256);
  uint16_t d = random(1, 256);

  WriteRegister(SCI_AICTRL0, a);
  WriteRegister(SCI_AICTRL1, b);
  WriteRegister(SCI_AICTRL2, c);
  WriteRegister(SCI_AICTRL3, d);

  int tempo = 60000 / BPM;
  delay(tempo / 1);

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

int ReadRegister(char address){
  
  while(!digitalRead(MP3_DREQ));
  digitalWrite(MP3_XCS, LOW);
  SPI.transfer(0x03);
  SPI.transfer(address);
  char response1 = SPI.transfer(0xFF);
  while(!digitalRead(MP3_DREQ));
  char response2 = SPI.transfer(0xFF);
  while(!digitalRead(MP3_DREQ));
  digitalWrite(MP3_XCS, HIGH);
  int resultvalue = response1 << 8;
  resultvalue |= response2;
  return resultvalue;
  
}

#define PLUGIN_SIZE 814

const static word plugin[PLUGIN_SIZE] PROGMEM = {

  0x0007,0x0001, /*copy 1*/
	0x8050,
	0x0006,0x0318, /*copy 792*/
	0x0030,0x0697,0x3700,0x0024,0x0000,0x0801,0xc010,0x0024,
	0x3f00,0x0024,0x2800,0x3e40,0x0006,0x2016,0x3613,0x0024,
	0x3e12,0x0024,0x3e05,0xb814,0x3615,0x0024,0x3e00,0x7842,
	0x3e00,0xf844,0x3e01,0x7846,0x3e01,0xf850,0x0038,0x0007,
	0x0002,0x0010,0x3e04,0x938c,0x3000,0xc024,0x4730,0x0024,
	0xf400,0x4012,0x2915,0xb380,0x3420,0x0804,0x3613,0x12cc,
	0x4480,0x3840,0x3e10,0x4024,0x2915,0xb380,0x3e00,0x8024,
	0x3411,0x0024,0x3411,0x4024,0x2915,0x9900,0x3431,0x8024,
	0x418a,0x4004,0x428c,0x1bc2,0x36f0,0x4024,0x2915,0xa940,
	0x36f0,0x0024,0x2915,0x9f40,0x0000,0x0024,0x0008,0x0002,
	0x4732,0x0024,0xf100,0x4052,0x6392,0xa800,0x6126,0x2001,
	0x0000,0x0024,0x2800,0x2055,0x0000,0x0024,0xb882,0x0024,
	0x3800,0x4024,0x36f4,0x8024,0x36f1,0xd810,0x36f1,0x5806,
	0x36f0,0xd804,0x36f0,0x5802,0x3405,0x9014,0x36f3,0x0024,
	0x36f2,0x0024,0x2000,0x0000,0x0000,0x0024,0x3613,0x0024,
	0x3e12,0x0024,0x3e05,0xb814,0x3615,0x0024,0x3e00,0x3841,
	0x3e00,0xb850,0x0007,0x9250,0x3e14,0x7812,0xb880,0x930c,
	0x3800,0x0024,0x0030,0x00d0,0x3440,0x0024,0x2908,0x6400,
	0x3800,0x0024,0x003f,0xfcc1,0x0030,0x0050,0x3000,0x134c,
	0xb010,0x0024,0x38f0,0x0024,0x3430,0x8024,0x6284,0x0000,
	0x0010,0x0001,0x2800,0x3dc5,0x0000,0x0024,0x002f,0xffc1,
	0xb010,0x0024,0x3800,0x0024,0x000b,0x4000,0x0006,0xc490,
	0x2908,0x7f80,0x3009,0x2000,0x0015,0xbd51,0x0030,0x0ad0,
	0x3800,0x0024,0x0000,0x01c0,0x2915,0x8300,0x0006,0xc050,
	0x0000,0x0300,0xb882,0x0024,0x2914,0xbec0,0x0006,0x6410,
	0x0001,0x0000,0x0030,0x0350,0x6284,0x2080,0x0000,0x0100,
	0x2800,0x2f45,0x3800,0x0024,0xf202,0x0000,0xc010,0x0024,
	0x3800,0x0024,0x0030,0x0390,0x3000,0x0024,0x6080,0x0024,
	0x0006,0x6552,0x2800,0x3195,0x1fe0,0x0040,0x003f,0xffc0,
	0x3800,0x0024,0x1fe0,0x0040,0x0006,0x6410,0x3870,0x138c,
	0x3a00,0x00cc,0x0000,0x00c0,0x0030,0x03d2,0x3200,0x4024,
	0xb100,0x0024,0x002e,0xe001,0x3800,0x0024,0x3420,0x0024,
	0x6012,0x0024,0x0017,0x7001,0x2800,0x3cc5,0x6012,0x0024,
	0x000b,0xb801,0x2800,0x3bc5,0x6010,0x0024,0x0000,0x01c0,
	0x2800,0x3815,0x0030,0x1090,0x3800,0x0024,0x0000,0x0080,
	0x0030,0x0050,0x3000,0x4024,0xc100,0x0024,0x3800,0x0024,
	0x0000,0x0000,0x0006,0x9f90,0x3009,0x2000,0x0006,0x9f50,
	0xb880,0xa000,0x0006,0x9fd0,0x3009,0x2000,0x36f4,0x5812,
	0x36f0,0x9810,0x36f0,0x1801,0x3405,0x9014,0x36f3,0x0024,
	0x36f2,0x0024,0x2000,0x0000,0x0000,0x0024,0x0000,0x01c0,
	0x0030,0x1090,0x2800,0x3800,0x3800,0x0024,0x0000,0x0140,
	0x0030,0x1090,0x2800,0x3800,0x3800,0x0024,0x2800,0x2940,
	0xc010,0x0024,0x0000,0x1097,0x3613,0x0024,0x3e12,0x0024,
	0x3e05,0xb814,0x2912,0x1300,0x3685,0x0024,0x0007,0x9250,
	0xb880,0x0024,0x2912,0x1180,0x3800,0x0024,0x0000,0x0084,
	0x0018,0x0000,0x3613,0x0024,0x6890,0x3840,0x3e10,0x0024,
	0x002e,0xe000,0x2900,0x22c0,0x3e00,0x0024,0x36e3,0x0024,
	0x2908,0xbe80,0x002e,0xe005,0x0000,0x0241,0x0030,0x0690,
	0x3000,0x0024,0xc010,0x0024,0xb882,0x2000,0x0000,0x0000,
	0x0006,0xa050,0x3009,0x2000,0x0000,0x1000,0x0006,0xa010,
	0x3009,0x2000,0x0004,0x0000,0x2915,0x7ac0,0x0000,0x0010,
	0x2912,0x1300,0x0000,0x0024,0x0008,0x0000,0xb882,0x0024,
	0x2915,0x7ac0,0x0038,0x0010,0x0030,0x0310,0x3000,0x184c,
	0x0002,0x0050,0x3800,0x0024,0x0030,0x0350,0x3000,0x0024,
	0x0002,0x0090,0x3800,0x0024,0x0030,0x0390,0x3000,0x0024,
	0x0002,0x00d0,0x3800,0x0024,0x0030,0x03d0,0x3000,0x0024,
	0x0002,0x0110,0x2912,0x0ec0,0x3800,0x0024,0xf400,0x4001,
	0x0000,0x1000,0x6106,0x0024,0x0000,0x0024,0x2800,0x4889,
	0x0000,0x0024,0x3433,0x184c,0xf400,0x4512,0xf400,0x4510,
	0x2912,0x1f80,0x34d3,0x0024,0x0000,0x0004,0x0000,0x0800,
	0x6400,0x10cc,0x0000,0x0800,0x2800,0x5cc1,0x3009,0x184c,
	0x0002,0x0150,0x0002,0x0051,0x1fff,0xfe02,0x34e3,0x0024,
	0x3c01,0x3005,0x3010,0x0024,0x30f0,0x4024,0xf400,0x4041,
	0xac2a,0x4041,0x3100,0xc024,0x0002,0x0091,0x6836,0x0024,
	0xac3e,0x0024,0xcefe,0x0024,0xfee2,0x4041,0x48ba,0x0024,
	0xfe8e,0x0024,0x51ea,0x0406,0x0002,0x00d1,0x44ba,0x0024,
	0xb46c,0x0024,0xac26,0x4041,0xb628,0x0402,0x0002,0x0111,
	0x6824,0x0024,0xac26,0x0024,0xbcde,0x4041,0x3100,0x8024,
	0x6824,0x0024,0xac26,0x0024,0x6c92,0x0024,0xc624,0x2040,
	0xd424,0x23c1,0x0010,0x0001,0x4286,0x1004,0xb880,0x93c5,
	0xb884,0x3840,0x0000,0x4000,0xfe30,0x3841,0x48b2,0x3842,
	0x2900,0x15c0,0x3e00,0x0024,0x6498,0x9b0c,0x6802,0x4490,
	0x3a20,0x004c,0x2800,0x4fc0,0x3800,0x4024,0x3e10,0x0024,
	0x290c,0x7300,0x3e15,0x134c,0x2800,0x4880,0x36e3,0x0024,
	0x3e12,0xb817,0x3e12,0x7808,0x3e11,0xb811,0x3e15,0x7810,
	0x3e18,0xb823,0x3e18,0x3821,0x3e10,0x3801,0x48b2,0x0024,
	0x3e10,0x3801,0x3e11,0x3802,0x3009,0x3814,0x0030,0x0717,
	0x3f05,0xc024,0x0030,0x0351,0x3100,0x0024,0x4080,0x0024,
	0x0030,0x10d1,0x2800,0x6905,0x0001,0x800a,0x0006,0x6514,
	0x3111,0x8024,0x6894,0x13c1,0x6618,0x0024,0xfe44,0x1000,
	0x4cb2,0x0406,0x3c10,0x0024,0x3c50,0x4024,0x34f0,0x4024,
	0x661c,0x1040,0xfe64,0x0024,0x4cb2,0x0024,0x3cf0,0x4024,
	0xbc82,0x3080,0x0030,0x0351,0x3100,0x8024,0xfea8,0x0024,
	0x5ca2,0x0024,0x0000,0x0182,0xac22,0x0024,0xf7c8,0x0024,
	0x48b2,0x0024,0xac22,0x0024,0x2800,0x6c80,0xf7cc,0x1002,
	0x0030,0x0394,0x3400,0x4024,0x3100,0x184c,0x0006,0xc051,
	0x291e,0x8080,0x0006,0x6410,0x4088,0x1001,0x0030,0x1111,
	0x3100,0x184c,0x0006,0xc051,0x291e,0x8080,0x0006,0x6550,
	0x0006,0x6694,0x408c,0x1002,0xf224,0x0024,0x0006,0xa017,
	0x2800,0x7095,0x0000,0x0024,0x2800,0x7181,0x0006,0x6410,
	0x3050,0x0024,0x3000,0x4024,0x6014,0x0024,0x0000,0x0024,
	0x2800,0x6fd9,0x0000,0x0024,0xf400,0x4040,0x38b0,0x0024,
	0x2800,0x7180,0x3800,0x0024,0x2800,0x7141,0x0000,0x0024,
	0xf400,0x4106,0xf400,0x4184,0x0030,0x06d5,0x3d05,0x5bd4,
	0xf400,0x4115,0x36f1,0x1802,0x36f0,0x1801,0x2210,0x0000,
	0x36f0,0x1801,0x36f8,0x1821,0x36f8,0x9823,0x0006,0x9f57,
	0x0020,0xffd0,0x3009,0x1c11,0x3985,0x4024,0x3981,0x8024,
	0x3009,0x3c11,0x36f5,0x5810,0x36f1,0x9811,0x36f2,0x5808,
	0x3602,0x8024,0x0030,0x0717,0x2100,0x0000,0x3f05,0xdbd7,
	0x0007,0x0001, /*copy 1*/
	0x0800,
	0x0006,0x0005, /*copy 5*/
	0x0000,0x0009,0x002e,0x000d,0x0006,
	0x0007,0x0001, /*copy 1*/
	0x8023,
	0x0006,0x0002, /*copy 2*/
	0x2a00,0x5e0e,

};

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