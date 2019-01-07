#include <DHTNew.h>
#include <Homie.h>  // homie-esp8266-3.0.1

HomieSetting<long> intervalSetting("interval", "The interval in seconds");
const int DEFAULT_INTERVAL = 60;
unsigned long lastSent = 0;
unsigned long dhtErrors = 0;
unsigned long dhtReadings = 0;
#define DHTPIN D7 // NodeMCU pin

#define DHTTYPE DHT_MODEL_DHT11 // Adjust range if you change this
DHT dht(DHTPIN, DHTTYPE);

HomieNode temperatureNode("temperature", "Temperature", "temperature");
HomieNode humidityNode("humidity", "Humidity", "humidity");
HomieNode errorNode("errors", "Errors", "errors");
HomieNode readingsNode("readings", "Readings", "readings");

void loopHandler() {
  if (! (millis() - lastSent >= intervalSetting.get() * 1000UL || lastSent == 0)) {
    return;
  }

  if(dht.read()) {
    dhtReadings++;
    readingsNode.setProperty("count").send(String(dhtReadings));

    if(dht.getError() != DHT_ERROR_NONE) {
      dhtErrors++;
      errorNode.setProperty("count").send(String(dhtErrors));
      Homie.getLogger().println( dht.getErrorString());
      // wait for the minimum sampling period
      delay(dht.getMinimumSamplingPeriod());
      return;
    }

    // Read temperature as Celsius (the default)
    float temperature = dht.getTemperature();    
    float humidity = dht.getHumidity();
	
    Homie.getLogger() << "Temperature: " << temperature << " °C" << endl;
    temperatureNode.setProperty("degrees").send(String(temperature));
    Homie.getLogger() << "Humidity: " << humidity << " %" << endl;
    humidityNode.setProperty("percent").send(String(humidity));
    
    lastSent = millis();
  }
}

void setup() {
  Serial.begin(115200);
  Serial << endl << endl;
  Homie_setFirmware("homie-dht11", "1.0.0");  
  Homie.setLoopFunction(loopHandler);
  temperatureNode.advertise("degrees").setName("Degrees").setDatatype("float").setUnit("°C");
  humidityNode.advertise("percent").setName("Percent").setDatatype("float").setUnit("%");
  errorNode.advertise("count").setName("Errors").setDatatype("float");
  readingsNode.advertise("count").setName("Readings").setDatatype("float");

  intervalSetting.setDefaultValue(DEFAULT_INTERVAL).setValidator([] (long candidate) {
    return candidate > 0;
  });
  
  Homie.setup();
}

void loop() {
  Homie.loop();
}
