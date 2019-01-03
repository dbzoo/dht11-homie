#include <DHT.h>
#include <Homie.h>  // homie-esp8266-3.0.1

HomieSetting<long> intervalSetting("interval", "The interval in seconds");
const int DEFAULT_INTERVAL = 300;
unsigned long lastSent = 0;

#define DHTPIN D7 // NodeMCU pin

#define DHTTYPE DHT11 // Adjust range if you change this
DHT dht(DHTPIN, DHTTYPE);

HomieNode temperatureNode("temperature", "Temperature", "temperature");
HomieNode humidityNode("humidity", "Humidity", "humidity");

void loopHandler() {
  if (millis() - lastSent >= intervalSetting.get() * 1000UL || lastSent == 0) {
    // Read temperature as Celsius (the default)
    float temperature = dht.readTemperature();    
    float humidity = dht.readHumidity();
	
    // Check if any reads failed and exit early.
    if (isnan(humidity) || isnan(temperature)) {
      Homie.getLogger().println("Failed to read from DHT sensor!");
      return;
    }
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
    
  intervalSetting.setDefaultValue(DEFAULT_INTERVAL).setValidator([] (long candidate) {
    return candidate > 0;
  });
  
  Homie.setup();
}

void loop() {
  Homie.loop();
}
