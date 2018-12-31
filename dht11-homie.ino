#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Homie.h>  // homie-esp8266-2.0.0

HomieSetting<long> intervalSetting("interval", "The interval in seconds");
const int DEFAULT_INTERVAL = 300;
unsigned long lastSent = 0;

#define DHTPIN D7 // NodeMCU pin

#define DHTTYPE DHT11 // Adjust range if you change this
DHT dht(DHTPIN, DHTTYPE);

HomieNode temperatureNode("temperature", "temperature");
HomieNode humidityNode("humidity", "humidity");

void setupHandler() {
  temperatureNode.setProperty("unit").send("c");
  humidityNode.setProperty("unit").send("%");
}

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
  Homie.setSetupFunction(setupHandler);
  Homie.setLoopFunction(loopHandler);
  temperatureNode.advertise("unit");
  temperatureNode.advertise("degrees");
  temperatureNode.advertiseRange("degrees",0,50); // dht11 range
  humidityNode.advertise("unit");
  humidityNode.advertise("percent");
  humidityNode.advertiseRange("percent",20,95); // dht11 range
  
  intervalSetting.setDefaultValue(DEFAULT_INTERVAL).setValidator([] (long candidate) {
    return candidate > 0;
  });
  
  Homie.setup();
}

void loop() {
  Homie.loop();
}
