// #define FIRMWARE_VERSION "0.8"
const char* FirmwareVersion = FIRMWARE_VERSION;

// #define DEBUG_AUTOUPDATE_ESP8266

// #define FIRMWARE_URL_BIN   "http://url/firmware.bin"
// #define FIRMWARE_URL_TXT   "http://url/firmware.txt"
const char *firmwareURL = FIRMWARE_URL_BIN;
const char *firmwareTXT = FIRMWARE_URL_TXT;

// Exemple pour un hébergement sur free.fr
// #define HOST_UPDATE  "site.free.fr"
// #define PATH_UPDATE "firmwares/"
// #define FILE_UPDATE_TXT "firmware.txt"
// #define FILE_UPDATE_BIN "firmware.bin"

#define SEPARATEUR_SIMPLE_CHR  "----------------"

int etat_update_firmware = 0;
String lecture_fichier_distant(void);

int UpDateOrNot(void); 

void update_started(void);
 
void update_finished(void);
 
void update_progress(int cur, int total);
 
void update_error(int err);

void AutoUpdate(void);

String lecture_fichier_distant(void) 
{
  WiFiClient client2;
  String payload = "";
  Serial.println(F(SEPARATEUR_SIMPLE_CHR));
  // Faire une requête HTTP GET
  if(client2.connect(HOST_UPDATE, 80)) 
  {
	client2.print(String("GET ") + String(PATH_UPDATE) + String(FILE_UPDATE_TXT) + String(" HTTP/1.1\r\n") + String("Host: ") + String(HOST_UPDATE) + String("\r\n Connection: close\r\n\r\n") );
    unsigned long timeout = millis();
    while(client2.available() == 0) 
    {
      if(millis() - timeout > 5000) 
      {
		
        Serial.println("Erreur dans la requête HTTP : timeout");
		
        client2.stop();
        return payload;
      }
    }
    // Lire le contenu du fichier
    while(client2.available()) 
    {
      String line = client2.readStringUntil('\n');
      payload = line;
    }
    /*
    while(client2.available()) // while (client2.connected()) 
    {
      String line = client2.readStringUntil('\n');
      payload += line;
      if (line == "\r") 
      {
        break;
      }
    }
*/
    #ifdef DEBUG_AUTOUPDATE_ESP8266
    Serial.println("Lecture du contenu du fichier distant :");
    Serial.println(payload);
	#endif
  }
  else {
	  
    Serial.println("Erreur dans la requête HTTP : échec de la connexion au serveur");
	   
  }

  return payload;
}

int UpDateOrNot(void) 
{
  int etat_update = 0;

  // Téléchargement du fichier du firmware.txt
    String payload_txt = lecture_fichier_distant();
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
		
        Serial.println(SEPARATEUR_SIMPLE_CHR);
        Serial.print("version actuel : ");
        Serial.print(FirmwareVersion);
        Serial.print(" / version_en_ligne : ");
        Serial.println(version_en_ligne);
        
        if (strcmp(version_en_ligne, FirmwareVersion) > 0) 
        {
          // Mise à jour du firmware
		  
          Serial.println(SEPARATEUR_SIMPLE_CHR);
          Serial.print("Mise à jour du firmware vers la version : ");
          Serial.println(version_en_ligne);
          
          etat_update = 1;
        }
      }
      token = strtok(NULL, "\n");
    }
    free(str);

  return etat_update;
}

void update_started(void) {
  Serial.println("CALLBACK:  HTTP update process started");
}
 
void update_finished(void) {
  Serial.println("CALLBACK:  HTTP update process finished");
}
 
void update_progress(int cur, int total) {
  Serial.printf("CALLBACK:  HTTP update process at %d of %d bytes...\n", cur, total);
}
 
void update_error(int err) {
  Serial.printf("CALLBACK:  HTTP update fatal error code %d\n", err);
}

void AutoUpdate(void)
{
	WiFiClient client_update;
    etat_update_firmware = UpDateOrNot();
    Serial.println(F(SEPARATEUR_SIMPLE_CHR));

    Serial.print("etat_update_firmware : ");
    Serial.println(etat_update_firmware);
    
    // WiFiClient client;
    // The line below is optional. It can be used to blink the LED on the board during flashing
    // The LED will be on during download of one buffer of data from the network. The LED will
    // be off during writing that buffer to flash
    // On a good connection the LED should flash regularly. On a bad connection the LED will be
    // on much longer than it will be off. Other pins than LED_BUILTIN may be used. The second
    // value is used to put the LED on. If the LED is on with HIGH, that value should be passed
    ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);
 
    // Add optional callback notifiers
    ESPhttpUpdate.onStart(update_started);
    ESPhttpUpdate.onEnd(update_finished);
    ESPhttpUpdate.onProgress(update_progress);
    ESPhttpUpdate.onError(update_error);
 
    ESPhttpUpdate.rebootOnUpdate(false); // remove automatic update
 
    if( etat_update_firmware )
    {
      Serial.println(F("Update start now!"));
     
      t_httpUpdate_return ret = ESPhttpUpdate.update(client_update, firmwareURL);
    
      switch (ret) 
      {
        case HTTP_UPDATE_FAILED:
        Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s --> %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str(), firmwareURL);
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

