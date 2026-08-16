#include <WiFi.h>
#include <esp_wifi.h>

// ================== CONFIG ==================
#define BUTTON_PIN 0  // Boot button on standard ESP32 boards (active LOW)
#define LED_PIN    2  // Onboard LED GPIO pin
const char* fakeSSIDs[100] = {
    "TellMyWiFiLoveHer", "Loading...", "Searching...", "WiFi_Goin_Down_For_Real", "GetYourOwnWiFi",
    "PrettyFlyForAWiFi", "HideYourKidsHideYourWiFi", "NoMoreMisterWiFi", "AbrahamLinksys", "JohnWilkesBooth",
    "BenjaminFranklinksys", "WinnebagoForCuties", "BillClinNet", "Router?IHardlyKnowHer!", "DropItLikeIt'sHotspot",
    "ItBurnsWhenIP", "WuTangLAN", "LAN_Solo", "ObiWanKenobi", "ThePingInTheNorth",
    "WinterIsConnecting", "HouseLANnister", "LordVoldemodem", "SilenceOfTheLANs", "KeepItOnTheDownload",
    "99ProblemsButWiFiAin'tOne", "EverydayImBuffering", "GirlsGoneWireless", "FormatC:", "Connecting...",
    "AccessDenied", "SearchingForNetwork...", "Wi-FiNotConnected", "NetworkNotFound", "IPFreely",
    "C:\\Wi-Fi", "404_WiFi_Not_Found", "Virus.exe", "TrojanHorse_Installer", "MalwareBytes_Test",
    "FBI_Surveillance_Van_4", "NSA_PRISM_Node_7", "CIA_Safehouse_WiFi", "MI6_Field_Office", "Area51_Alien_Uplink",
    "KGB_Listening_Post", "Interpol_Mobile_Unit", "SWAT_Command_Center", "SecretService_Hotspot", "Paranoid_Android",
    "GetOffMyLawn", "Yell_PooP_For_Password", "BringBeerToApt4B", "PayYourRentFirst", "StopStargazingAt3AM",
    "QuietDownNeighbors", "CleanYourRoomFirst", "WeCanHearYouSinging", "StopStealingMyPaper", "BarkingDogIn3C",
    "NachoWiFi", "TestingNetwork_DoNotJoin", "FreeWiFi_JustKidding", "BuyYourOwnRouter", "ClickHereForFreeMoney",
    "TotallyNotAWebTrap", "Unsecured_Public_WiFi", "Error_500_Internal_Server", "Rebooting_Please_Wait", "Out_Of_Bandwidth",
    "MomUseThisOne", "DadPressHereForInternet", "Grandma_Click_This", "LookMomNoWires", "NoFreeWiFiHere",
    "GoGoGadgetWiFi", "SuperFastUnicornNet", "PotatoPoweredRouter", "DialUp_56k_Sound", "IsThisTheRealLife",
    "OrIsThisJustFantasy", "CaughtInALANslide", "NoEscapeFromReality", "OpenYourAPIs", "LookUpToTheSkies",
    "ImJustABoorerBoy", "IGetanErrorValue", "EasyComeEasyGo", "LittleHighLittleLow", "AnyWayTheSignalBlows",
    "Doesn'tReallyMatterToMe", "Mama_JustKilledAWifi", "PutAModemToHisHead", "PulledMyTriggerNowHe'sDead", "WiFi_Life_Has_Just_Begun",
    "ButNowI'veGoneAndThrownItAllAway", "Mama_Ooh_Didn't_Mean_To_Make_You_Cry", "IfI'mNotConnectedThisTimeTomorrow", "CarryOn_CarryOn", "AsIfNothingReallyMatters"
};
const int numSSIDs = sizeof(fakeSSIDs) / sizeof(fakeSSIDs[0]);

void sendBeacon(const char* ssid);

void setup() {
  Serial.begin(115200);
  delay(1000);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  
  // Ensure LED is off during setup
  digitalWrite(LED_PIN, LOW);

  unsigned long pressStartTime = 0;

  // Clean start
  esp_wifi_stop();
  delay(100);
  WiFi.mode(WIFI_MODE_NULL);
  delay(100);

  WiFi.mode(WIFI_MODE_STA);
  esp_wifi_set_promiscuous(true);
  delay(200);

  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  Serial.printf("MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  while (true) {
    if (digitalRead(BUTTON_PIN) == LOW) { 
      if (pressStartTime == 0) {
        pressStartTime = millis(); // Start tracking press duration
      } else if (millis() - pressStartTime >= 2000) {
        break; // Successfully held for 2 seconds, exit loop
      }
    } else {
      pressStartTime = 0; // Reset timer if released before 2 seconds
    }
    
    delay(10); // Small yield delay to avoid rapid polling
  }

  // Turn LED on once the button check passes
  digitalWrite(LED_PIN, HIGH);
}

void loop() {
  for (int i = 0; i < numSSIDs; i++) {
    sendBeacon(fakeSSIDs[i]);
  }
}

void sendBeacon(const char* ssid) {
  int ssidLen = strlen(ssid);
  if (ssidLen > 32) ssidLen = 32;

  uint8_t packet[200];  // bigger buffer to be safe
  int offset = 0;

  // Fixed beacon header
  packet[offset++] = 0x80;  // Management, Beacon
  packet[offset++] = 0x00;
  packet[offset++] = 0x00;  // Duration
  packet[offset++] = 0x00;

  // Destination = broadcast
  for (int i = 0; i < 6; i++) packet[offset++] = 0xFF;

  // Source MAC + BSSID (randomized)
  uint8_t mac[6];
  for (int j = 0; j < 6; j++) mac[j] = random(0, 256);
  for (int i = 0; i < 6; i++) packet[offset++] = mac[i];  // Source
  for (int i = 0; i < 6; i++) packet[offset++] = mac[i];  // BSSID

  packet[offset++] = 0x00;  // Sequence control low
  packet[offset++] = 0x00;

  // Timestamp (8 zero bytes is fine)
  for (int i = 0; i < 8; i++) packet[offset++] = 0x00;

  packet[offset++] = 0x64;  // Beacon interval low
  packet[offset++] = 0x00;  // high

  packet[offset++] = 0x01;  // Capability low (open)
  packet[offset++] = 0x04;  // high

  // === SSID IE ===
  packet[offset++] = 0x00;           // Element ID: SSID
  packet[offset++] = ssidLen;
  memcpy(&packet[offset], ssid, ssidLen);
  offset += ssidLen;

  // Supported Rates
  packet[offset++] = 0x01;
  packet[offset++] = 0x08;
  uint8_t rates[8] = {0x82, 0x84, 0x8b, 0x96, 0x12, 0x24, 0x48, 0x6c};
  memcpy(&packet[offset], rates, 8);
  offset += 8;

  // DS Parameter Set (Channel 1)
  packet[offset++] = 0x03;
  packet[offset++] = 0x01;
  packet[offset++] = 1;   // Change to 6 or 11 if you want

  // Extended Supported Rates (helps a lot with modern phones)
  packet[offset++] = 0x32;
  packet[offset++] = 0x04;
  uint8_t ext_rates[4] = {0x0c, 0x18, 0x30, 0x60};
  memcpy(&packet[offset], ext_rates, 4);
  offset += 4;

  // Send it
  esp_err_t result = esp_wifi_80211_tx(WIFI_IF_STA, packet, offset, false);
  if (result != ESP_OK) {
    Serial.printf("Failed for '%s' (err %d)\n", ssid, result);
  }
}