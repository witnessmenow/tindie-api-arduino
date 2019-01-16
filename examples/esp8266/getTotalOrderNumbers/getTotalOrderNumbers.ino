/*******************************************************************
    Get Total Tindie Orders + unshipped order numbers
 
    By Brian Lough
    YouTube: https://www.youtube.com/brianlough
    Tindie: https://www.twitch.tv/brianlough
    Twitter: https://twitter.com/witnessmenow
 *******************************************************************/

#include <TindieApi.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

#include <ArduinoJson.h>
// Library used for parsing Json from the API responses
// NOTE: There is a breaking change in the 6.x.x version,
// install the 5.x.x version instead
// Search for "Arduino Json" in the Arduino Library manager
// https://github.com/bblanchon/ArduinoJson

//------- Replace the following! ------

char ssid[] = "SSID";         // your network SSID (name)
char password[] = "password"; // your network password

#define TINDIE_API_KEY "1234567890654rfscFfsdfdsffd"
// API Key can be retrieved from here
// https://www.tindie.com/profiles/update/
//
// NOTE: Please be very careful with this API key as people will be
// able to retrieve customer information (address, phone numbers, emails etc)
// if they get access to it

#define TINDIE_USER "brianlough"

//------- ---------------------- ------

WiFiClientSecure client;
TindieApi tindie(client, TINDIE_USER, TINDIE_API_KEY);

unsigned long delayBetweenRequests = 60000; // Time between requests (1 minute)
unsigned long requestDueTime;               //time when request due

void setup()
{

    Serial.begin(115200);

    // Set WiFi to station mode and disconnect from an AP if it was Previously
    // connected
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    // Attempt to connect to Wifi network:
    Serial.print("Connecting Wifi: ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(500);
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    IPAddress ip = WiFi.localIP();
    Serial.println(ip);

    // Only avaible in ESP8266 V2.5 RC1 and above
    // I would consider this very important for this library
    client.setFingerprint(TINDIE_FINGERPRINT);

    // If you want to enable some extra debugging
    //tindie._debug = true;
}

void loop()
{

    if (millis() > requestDueTime)
    {
        Serial.print("Free Heap: ");
        Serial.println(ESP.getFreeHeap());

        Serial.print("Getting number of Tindie Orders for:");
        Serial.println(TINDIE_USER);
        int totalTindieOrders = tindie.getOrderCount();
        delay(100);
        int unshippedTindieOrders = tindie.getOrderCount(false);

        Serial.println("------------------");

        Serial.print("Total Orders: ");
        if (totalTindieOrders >= 0)
        {
            Serial.println(totalTindieOrders);
        }
        else
        {
            Serial.println("error");
        }

        Serial.print("Un-shipped Orders: ");
        if (unshippedTindieOrders >= 0)
        {
            Serial.println(unshippedTindieOrders);
        }
        else
        {
            Serial.println("error");
        }

        Serial.println("------------------------");

        requestDueTime = millis() + delayBetweenRequests;
    }
}