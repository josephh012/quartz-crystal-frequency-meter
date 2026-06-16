/*
  Display of the quartz frequency on an 8-digit display with MAX7219
  source are from==https://gammon.com.au/forum/?id=11497
*/

// Set a different value depending on the chip.
// float magic = 910;
// float magic = 949;
// float magic = 927;
// float magic = 992.47;
float magic = 957.50;

const byte MAX7219_DIN = A5;
const byte MAX7219_CS  = A4;
const byte MAX7219_CLK = A3;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  initialise(); // init display:
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  print(88888888);
  delay(200);
  long f_cpu = diff() * magic + 0.5;
  Serial.begin(9600.0f * F_CPU / f_cpu);
  Serial.println(__FILE__);
  char src[12];
  ltoa(f_cpu, src, 10);
  int sLen = strlen(src);
  char dest[12] = "           \0";
  int dLen = strlen(dest);
  byte si = 1;
  byte di = dLen - 1;
  // insert dots:
  while (si <= sLen) {
    dest[di--] = src[sLen - si];
    if ((si++ % 3) == 0) dest[di--] = '.';
  }
  Serial.print("f =");
  Serial.print(dest);
  Serial.println(" Hz");
  print(f_cpu);
  digitalWrite(LED_BUILTIN, LOW);
  // wait 2 seconds:
  delay(2000 * f_cpu / F_CPU);
}

volatile bool bite;

ISR(WDT_vect, ISR_NAKED) {
  // compiled: 14 instructions
  // Please set to 0.
  asm(
    "sts (bite), r1 \n"
    "reti \n"
  );
}

int diff() {
  MCUSR = 0;
  WDTCSR = B00011000; // WDCE + WDE
  WDTCSR = B01000000; // WDIE
  bite = true;
  long startTime = micros();
  // wait for watchdog:
  asm("WDR"); // pat the dog
  while (bite);
  long endTime = micros();
  return endTime - startTime;
}

// *********************************

void print(long v) {
  char c[10];
  ltoa(1E8 + v, c, 10);
  char d = c[1] - '0';
  // suppress leading zero
  if (d == 0) d = 15;
  output(8, d);
  for (byte i = 1; i < 7; i++) {
    d = c[i + 1] - '0';
    if (i % 3 == 1) d = d + 128;
    output(8 - i, d);
  }
  output(1, c[8] - '0');
}

void initialise() {
  pinMode(MAX7219_DIN, OUTPUT);
  pinMode(MAX7219_CS, OUTPUT);
  pinMode(MAX7219_CLK, OUTPUT);
  digitalWrite(MAX7219_CS, HIGH);
  output(0x0f, 0x00); // display test register - test mode off
  output(0x0c, 0x01); // shutdown register - normal operation
  output(0x0b, 0x07); // scan limit register - display digits 0 thru 7
  output(0x0a, 0x0f); // intensity register - max brightness
  output(0x09, 0xff); // decode mode register - CodeB decode all digits}
}

void output(byte address, byte data) {
  digitalWrite(MAX7219_CS, LOW);
  shiftOut(MAX7219_DIN, MAX7219_CLK, MSBFIRST, address);
  shiftOut(MAX7219_DIN, MAX7219_CLK, MSBFIRST, data);
  digitalWrite(MAX7219_CS, HIGH);
}