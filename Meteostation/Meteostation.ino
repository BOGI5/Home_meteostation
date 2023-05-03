#include <U8glib.h>
#include <Wire.h>
#include <DHT.h>
#include <Adafruit_BMP280.h>
#include <iarduino_RTC.h>  

int pressed = 0;
int prev_day_status;
bool just_started = true;
float data[3][3];
char data_name[3][2][12] = {{"Temperature", "C"}, {"Pressure", "HPa"}, {"Humidity", "%"}}; 
char weekdays[7][9] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
int coordinates[3][2] = {{20, 85}, {30, 93}, {30, 85}};

Adafruit_BMP280 bmp;
iarduino_RTC time(RTC_DS1307); 
U8GLIB_ST7920_128X64_4X u8g(13, 11, 10, U8G_PIN_NONE );
DHT dht(2, DHT11);

void setup() {            
  pinMode(A0, INPUT);
  time.begin(); 
  dht.begin();  
  u8g.begin();         
  u8g.setFont(u8g_font_unifont);
  u8g.setFontPosTop();
  Serial.begin(9600);
  while ( !Serial ) delay(100);
  if (!bmp.begin()) {
    u8g.firstPage();  
    do 
    {
      u8g.drawStr(0, 0, "Error: bmp280");
      u8g.drawStr(12, 13, "Check");
      u8g.drawStr(6, 26, "wiring or i2c");
      u8g.drawStr(6, 39, "address");
    }
    while(u8g.nextPage());
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                      "try a different address!"));
    while (1) delay(10);
  }

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
  
}

void collect_data()
{
  data[0][1] = bmp.readTemperature();
  data[1][1] = bmp.readPressure() / 100;
  data[2][1] = dht.readHumidity();
  for(int i = 0; i < 3; i++)
  {
    if(data[i][0] > data[i][1] || just_started)data[i][0] = data[i][1];
    if(data[i][2] < data[i][1] || just_started)data[i][2] = data[i][1]; 
  }
  just_started = false;
}

void print_data(int data_index)
{
  if(!data_index)
  {
    main_page();
    return;
  }
  data_index--;
  u8g.drawStr(coordinates[data_index][0], -1, data_name[data_index][0]);
  u8g.drawHLine(5, 12, 118);
  
  u8g.drawStr(10, 14, "Min: ");
  u8g.setPrintPos(45, 14);
  u8g.print(data[data_index][0]);
  u8g.drawStr(coordinates[data_index][1], 14, data_name[data_index][1]);

  u8g.drawStr(10, 30, "Now: ");
  u8g.setPrintPos(45, 30);
  u8g.print(data[data_index][1]);
  u8g.drawStr(coordinates[data_index][1], 30, data_name[data_index][1]);

  u8g.drawStr(10, 46, "Max: ");
  u8g.setPrintPos(45, 46);
  u8g.print(data[data_index][2]);
  u8g.drawStr(coordinates[data_index][1], 46, data_name[data_index][1]);
}

void main_page()
{
  u8g.setFont(u8g_font_6x10);
  u8g.setFontPosTop();
  u8g.drawStr(15, 0, time.gettime("d.m.Y"));
  u8g.drawStr(79, 0, weekdays[time.weekday - 1]);
  u8g.setFont(u8g_font_unifont);
  u8g.setFontPosTop();
  u8g.drawStr(30, 14, time.gettime("H:i:s"));
  u8g.drawHLine(5, 31, 118);
  u8g.setPrintPos(0, 50);
  u8g.print(data[0][1]);
  u8g.drawStr(coordinates[0][1] - 45, 50, data_name[0][1]);
  u8g.setPrintPos(26, 35);
  u8g.print(data[1][1]);
  u8g.drawStr(coordinates[1][1] - 19, 35, data_name[1][1]);
  u8g.setPrintPos(79, 50);
  u8g.print(data[2][1]);
  u8g.drawStr(coordinates[2][1] + 34, 50, data_name[2][1]);
}

void loop() {
  if(prev_day_status != time.day)just_started = true;
  prev_day_status = time.day;
  collect_data();
  if(digitalRead(A0))
  {
    pressed++;
    delay(150);
  } 
  u8g.firstPage();  
  u8g.setRot180();
  do 
  {
    if(pressed == 4)pressed = 0;
    print_data(pressed);
  }
  while(u8g.nextPage()); 
}
