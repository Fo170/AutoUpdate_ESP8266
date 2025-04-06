// #define FIRMWARE_VERSION "1.5.0"
const char* FirmwareVersion = FIRMWARE_VERSION;

// #define DEBUG_AUTOUPDATE_ESP8266

// #define FIRMWARE_URL_BIN   "http://url/firmware.bin"
// #define FIRMWARE_URL_TXT   "http://url/firmware.txt"
const char *firmwareURL = FIRMWARE_URL_BIN;
const char *firmwareTXT = FIRMWARE_URL_TXT;

int etat_update_firmware = 0;
String lecture_fichier_distant(void);

int UpDateOrNot(void); 

void update_started(void);
 
void update_finished(void);
 
void update_progress(int cur, int total);
 
void update_error(int err);

void AutoUpdate(void);

#include <ESP8266httpUpdate.h> // Include the HTTPUpdate library for ESP8266
ESP8266HTTPUpdate ESP_httpUpdate; // Use the correct object from the library
	
String lecture_fichier_distant(void) 
{
 String payload = "";
 WiFiClient client; // Create a WiFiClient object
    HTTPClient http;

    // Use the new API with WiFiClient and URL
    if (http.begin(client, FIRMWARE_URL_TXT)) {
        int httpCode = http.GET();

        if (httpCode > 0) {
            if (httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
                http.end();
                return payload;
            } else {
                Serial.printf("HTTP GET failed, code: %d\n", httpCode);
            }
        } else {
            Serial.printf("HTTP GET failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();
    } else {
        Serial.println("Unable to connect to the server.");
    }

    return ""; // Return an empty string if the request fails
}

int UpDateOrNot(void) 
{
  int etat_update = 0;
  Serial.print("version actuel : ");
  Serial.println(FirmwareVersion);
  // Téléchargement du fichier du firmware.txt
    String payload_txt = lecture_fichier_distant();
	Serial.print("payload_txt : ");
    Serial.println(payload_txt);
	
    // Extraction de la version du firmware
    char* str = (char*) malloc(payload_txt.length() + 1);
    payload_txt.toCharArray(str, payload_txt.length() + 1);
    char* token = strtok(str, "\n");
    while (token != NULL) 
    {
      if (strncmp(token, "FW_VERSION:", 11) == 0) 
      {
        char* version_en_ligne = token + 11;
        // Comparaison de la version du firmware
        Serial.print("version_en_ligne : ");
        Serial.println(version_en_ligne);
        
        if (strcmp(version_en_ligne, FirmwareVersion) > 0) 
        {
          // Mise à jour du firmware
          Serial.println("Mise à jour du firmware vers la nouvelle version");
          
          etat_update = 1;
        }
      }
      token = strtok(NULL, "\n");
    }
    free(str);

  return etat_update;
}

void update_started(void) 
{
  Serial.println("CALLBACK:  HTTP update process started");
}
 
void update_finished(void) 
{
  Serial.println("CALLBACK:  HTTP update process finished");
}
 
void update_progress(int cur, int total) 
{
  Serial.printf("CALLBACK:  HTTP update process at %d of %d bytes...\n", cur, total);
}
 
void update_error(int err) 
{
  Serial.printf("CALLBACK:  HTTP update fatal error code %d\n", err);
}

void AutoUpdate(void)
{
	WiFiClient client_update;
  
    etat_update_firmware = UpDateOrNot();
    Serial.print("etat_update_firmware : ");
    Serial.println(etat_update_firmware);
    
    // WiFiClient client;
    // The line below is optional. It can be used to blink the LED on the board during flashing
    // The LED will be on during download of one buffer of data from the network. The LED will
    // be off during writing that buffer to flash
    // On a good connection the LED should flash regularly. On a bad connection the LED will be
    // on much longer than it will be off. Other pins than LED_BUILTIN may be used. The second
    // value is used to put the LED on. If the LED is on with HIGH, that value should be passed
	  
    ESP_httpUpdate.setLedPin(LED_BUILTIN, LOW);
	
    // Add optional callback notifiers
    ESP_httpUpdate.onStart(update_started);
    ESP_httpUpdate.onEnd(update_finished);
    ESP_httpUpdate.onProgress(update_progress);
    ESP_httpUpdate.onError(update_error);
    ESP_httpUpdate.rebootOnUpdate(false); // remove automatic update

    if( etat_update_firmware )
    {
      Serial.println(F("Update start now!"));
     
      t_httpUpdate_return ret = ESP_httpUpdate.update(client_update, firmwareURL);
  
      switch (ret) 
      {
        case HTTP_UPDATE_FAILED:
        Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s --> %s\n", ESP_httpUpdate.getLastError(), ESP_httpUpdate.getLastErrorString().c_str(), firmwareURL);
        Serial.println(F("Retry in 10secs!"));
        delay(10000); // Wait 10secs
        break;
 
        case HTTP_UPDATE_NO_UPDATES:
        Serial.println("HTTP_UPDATE_NO_UPDATES");
        Serial.println("Your code is up to date!");
          delay(10000); // Wait 10secs
        break;
 
        case HTTP_UPDATE_OK:
        Serial.println("HTTP_UPDATE_OK");
        delay(1000); // Wait a second and restart
        ESP.restart();
        break;
      }
    }
}
