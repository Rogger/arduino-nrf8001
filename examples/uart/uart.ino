#define NRF_DEBUG 1
#include <SPI.h>
#include <nRF8001.h>

#define RX_PIPE 7
#define TX_PIPE 8
#define BATTERY_PIPE 3

// change nRF8001 reset pin to -1 if it's not connected
// Redbear BLE Shield users: to my knowledge reset pin is not connected so use -1!
// NOTE: if you choose -1, youll need to manually reset your device after powerup!!
#define RESET_PIN 7
#define REQN_PIN 9
#define RDYN_PIN 3

nRF8001 *nrf;

uint8_t pipeStatusReceived, dataSent;
unsigned long lastSent;

// Generic event handler, here it's just for debugging all received events
void eventHandler(nRFEvent *event)
{
  Serial.println("============ generic event handler ============");
  nrf->debugEvent(event);
  Serial.println("");
}

void dataReceivedHandler(nRFPipe servicePipeNo, uint8_t *data)
{
  Serial.println("============ data received handler ============");
  char *t = (char*)data;
  Serial.println(t);
}

void setup() {
  pipeStatusReceived = 0;
  lastSent = 0;
  
  Serial.begin(115200);
  Serial.println("Starting...");
  
  // nRF8001 class initialized with pin numbers
  nrf = new nRF8001(RESET_PIN, REQN_PIN, RDYN_PIN);

  // Register event handles
  nrf->setEventHandler(&eventHandler);
  nrf->setDataReceivedHandler(&dataReceivedHandler);

  if ((nrf->setup()) == cmdSuccess) {
    Serial.println("SUCCESS");
  } else {
    Serial.println("FAIL");
    while (1);
  }
  
  // These functions merely request device address,
  // actual responses are asynchronous. They'll return error codes
  // if somehow the request itself failed, for example because
  // the device is not ready for these commands.
  nrf->getDeviceAddress();
  nrf->poll();
  nrf->getDeviceVersion();
  nrf->poll();
  nrf->getBatteryLevel();
  nrf->poll();
  
  nrf->connect(0, 32);
}

void loop() {
  
  Serial.println("polling");
  // Polling will block - times out after 2 seconds
  nrf->poll(2000);
  
  if (nrf->isPipeOpen(TX_PIPE) && (millis() - lastSent) > 5000 && nrf->isConnected()) {
    
    char *hello = "hello G\0"; 
    Serial.print("sending ");
    Serial.println(hello);
   
    nrf->sendData(TX_PIPE, strlen(hello) , (uint8_t *)hello);
    
    lastSent = millis();
    
    
    //uint8_t bat = 78;
    // If battery pipe is open
    /*if (nrf->isPipeOpen(BATTERY_PIPE) && nrf->creditsAvailable()) {
      nrf->sendData(BATTERY_PIPE, 1, &bat);
    }*/
    
  } else if (nrf->getConnectionStatus() == Disconnected) {
    Serial.println("Reconnecting");
    dataSent = 0;
    nrf->connect(0, 32);
  }
}

