#include <Arduino.h>
#include <U8x8lib.h>
#include <AM2302-Sensor.h>

//screen setup
U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);

//temperature sensor setup
constexpr unsigned int SENSOR_PIN {7U};
AM2302::AM2302_Sensor am2302{SENSOR_PIN};

// Returns actual value of Vcc (x 100)
int getBandgap(void) {
   // For 168/328 boards
   const long InternalReferenceVoltage = 1056L;  // Adjust this value to your boards specific internal BG voltage x1000 by Mouse probably should be a little bit hire for my specific board
   // REFS1 REFS0          --> 0 1, AVcc internal ref. -Selects AVcc external reference
   // MUX3 MUX2 MUX1 MUX0  --> 1110 1.1V (VBG)         -Selects channel 14, bandgap voltage, to measure
   ADMUX = (0<<REFS1) | (1<<REFS0) | (0<<ADLAR) | (1<<MUX3) | (1<<MUX2) | (1<<MUX1) | (0<<MUX0);

   delay(50);  // Let mux settle a little to get a more stable A/D conversion
   // Start a conversion  
   ADCSRA |= _BV( ADSC );
   // Wait for it to complete
   while( ( (ADCSRA & (1<<ADSC)) != 0 ) );
   // Scale the value
   int results = (((InternalReferenceVoltage * 1024L) / ADC) + 5L) / 10L; // calculates for straight line value 
   return results;
}


void setup(void) {
   //screen setup
   u8x8.begin();
   u8x8.setPowerSave(0);
   pinMode(LED_BUILTIN, OUTPUT);
}

void loop(void) {
   u8x8.setFont(u8x8_font_chroma48medium8_r);
   u8x8.drawString(4,2,"Im Spark!");

   auto status = am2302.read();

   float temperature = am2302.get_Temperature();
   float humidity = am2302.get_Humidity();

   char buffer[128];
   char value[128];

   dtostrf(temperature, 0, 1, value);
   snprintf(buffer, sizeof(buffer), "%s%s%s", "Temp: ", value, "c");
   u8x8.drawString(0, 3, buffer);

   dtostrf(humidity, 0, 1, value);
   snprintf(buffer, sizeof(buffer), "%s%s%s", "Humi: ", value, "%");
   u8x8.drawString(0, 4, buffer);

   int Vcc = getBandgap();
   int Battery = map(analogRead(0), 0, 1023, 0, Vcc) + 15;
   float fBattery = Battery / 100.f;
   dtostrf(fBattery, 0, 2, value);
   snprintf(buffer, sizeof(buffer), "%s%s%s", "Batt: ", value, "v");
   u8x8.drawString(0, 5, buffer);

  digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(1000);                      // wait for a second
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
  delay(1000);                      // wait for a second
}
