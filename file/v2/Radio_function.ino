
//---- Ritorno al loop
void software_Reboot() {
  start();
  boot = 1;
  vmenu = 10; part = 10;
  old_frequency = -1;
  old_stereo = 0;
  stereo = 1;
  old_mute = 1;
  old_signal_level = 1;
  signal_level = 0;
  p = 1;
  c = 0;
  part = 0;
  step = 0;
  TEA5767_read_data();


  if (frequency != old_frequency && p == 1) {
    set_text(old_frequency >= 10000 ? 6 : 14, 17, value_to_string(old_frequency), WHITE, 2);
    set_text(frequency >= 10000 ? 6 : 14, 17, value_to_string(frequency), BLACK, 2);
    old_frequency = frequency;
  }

  if (old_stereo != stereo && p == 1) {

    set_text(old_stereo ? 22 : 28, 39, old_stereo ? "Stereo" : "Mono", WHITE, 1);
    set_text(stereo ? 22 : 28, 39, stereo ? "Stereo" : "Mono", BLACK, 1);
    old_stereo = stereo;
  }

  if (old_signal_level != signal_level) {
    set_text(old_signal_level < 10 ? 76 : 70, 39, String((int)old_signal_level), WHITE, 1);
    set_text(signal_level < 10 ? 76 : 70, 39, String((int)signal_level), BLACK, 1);
    old_signal_level = signal_level;
    show_signal_level(signal_level);
  }

  if (old_mute != mute) {
    set_text(1, 39, old_mute ? "M" : "S", WHITE, 1);
    set_text(1, 39, mute ? "M" : "S", BLACK, 1);
    old_mute = mute;
  }
}

//----Stampa numero stazione
void station () {
  lcd.display();
  set_text(old_frequency >= 10000 ? 45 : 1, 1, value_to_string(old_frequency), BLACK, 0);
  set_text(frequency >= 10000 ? 45 : 1, 1, value_to_string(frequency), WHITE, 0);
}
//----Stampa valore che si ripete
void print() {

  digitalWrite(lcd_luminosity, LOW);
  lcd.clearDisplay();
  lcd.display();
  lcd.drawBitmap(0, 0,  menu, 84, 48, 1);
  blink();
  set_text(old_frequency >= 10000 ? 45 : 1, 1, value_to_string(old_frequency), BLACK, 0);
  set_text(frequency >= 10000 ? 45 : 1, 1, value_to_string(frequency), WHITE, 0);
  old_frequency = frequency;
  set_text( 57, 10, interruttore, BLACK, 1);
  set_text( 57, 20, (space + contr), BLACK, 1);
}


void blink () {

  if (digitalRead(lcd_luminosity) == LOW) {
    acceso = "|On";
  }
  else if (digitalRead(lcd_luminosity) == HIGH) {
    acceso = "|Off";
  }
}

void start() {

  Wire.begin();
  TEA5767_mute();
  TEA5767_set_frequency();
  lcd.begin();
  lcd.setContrast(contr);
  if (boot == 0) {
    bootanimation();
  }
  TEA5767_mute();

}
//----Boot iniziale
void bootanimation() {
  lcd.clearDisplay();
  lcd.drawBitmap(0, 0,  boot1, 84, 48, 1);
  lcd.display();
  delay(1250);
  lcd.clearDisplay();
  lcd.drawBitmap(0, 0,  boot2, 84, 48, 1);
  lcd.display();
  delay(1250);
  lcd.clearDisplay();
  lcd.drawBitmap(0, 0,  boot3, 84, 48, 1);
  lcd.display();
  delay(2000);
  lcd.clearDisplay();
  boot = 1;
  lcd.setContrast(contr);
}

unsigned char frequencyH = 0;
unsigned char frequencyL = 0;
unsigned int frequencyB;
unsigned char TEA5767_buffer[5] = {0x00, 0x00, 0xB0, 0x10, 0x00};

void TEA5767_write_data(byte data_size) {

  delay(50);
  Wire.beginTransmission(0x60);

  for (byte i = 0; i < data_size; i++)
    Wire.write(TEA5767_buffer[i]);
  Wire.endTransmission();
  delay(50);
}

void TEA5767_mute() {

  if (!mute) {
    mute = 1;
    TEA5767_buffer[0] |= TEA5767_MUTE_FULL;
    TEA5767_write_data(2);
  }

  else {
    mute = 0;
    TEA5767_buffer[0] &= ~TEA5767_MUTE_FULL;
    TEA5767_write_data(2);
  }
}

void TEA5767_set_frequency() {
  frequencyB = 4 * (frequency * 10000 + 225000) / 32768;
  TEA5767_buffer[0] = frequencyB >> 8;
  if (mute)TEA5767_buffer[0] |= TEA5767_MUTE_FULL;
  TEA5767_buffer[1] = frequencyB & 0XFF;
  TEA5767_write_data(5);
}

int TEA5767_read_data() {
  orologio();
  button.tick();
  unsigned char buf[5];
  memset (buf, 0, 5);
  Wire.requestFrom (0x60, 5);
  if (Wire.available ()) {
    for (int i = 0; i < 5; i++) {
      buf[i] = Wire.read ();
    }
    stereo = (buf[2] & TEA5767_STEREO_MASK) ? 1 : 0;
    signal_level = ((buf[3] & TEA5767_ADC_LEVEL_MASK) >> 4);
    return 1;
  }
  else return 0;
}

void orologio() {
  DateTime now = RTC.now();
  String time1 =  String(now.hour(), DEC) + ':' + String(now.minute(), DEC) + '-' + String(now.day(), DEC) + '/' + String(now.month(), DEC);

  if (c == 0) {
    set_text(1, 1, time1, BLACK, 1);
    set_text(1, 1, time1, WHITE, 1);
  }
}
//----Mostra icona Onde Radio
void show_signal_level(int level) {
  byte xs = 68;
  byte ys = 7;
  for (int i = 0; i < 15; i++) {
    if (i % 2 != 0)
      lcd.drawLine(xs + i, ys, xs + i, ys - i / 2, level >= i ? BLACK : WHITE);
  }
}

//---Mostra numero Onde Radio
String value_to_string(int value) {
  String value_string = String(value / 100);
  value_string = value_string + '.' + ((value % 100 < 10) ? "0" : "") + (value % 100);
  return value_string;
}

//----Inizializzare lo schermo
void set_text(int x, int y, String text, int color, int textsize) {

  lcd.setTextSize(textsize);
  lcd.setTextColor(color);
  lcd.setCursor(x, y);
  lcd.println(text);
  lcd.display();


}
