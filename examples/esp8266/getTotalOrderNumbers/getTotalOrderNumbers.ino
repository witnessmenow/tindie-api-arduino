/*******************************************************************
 *  Get Total Tindie Orders + unshipped order numbers
 *
 *  By Brian Lough
 *  https://www.youtube.com/brianlough
 *******************************************************************/

#include <TindieApi.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

// Currently needs to be V5.X.X (not V6)
#include <ArduinoJson.h> // This Sketch doesn't technically need this, but the library does so it must be installed.

//------- Replace the following! ------
char ssid[] = "SSID";         // your network SSID (name)
char password[] = "password"; // your network key

#define TINDIE_API_KEY "1234567890654rfscFfsdfdsffd"

#define TINDIE_USER "brianlough"

WiFiClientSecure client;
TindieApi tindie(client, TINDIE_USER, TINDIE_API_KEY);

unsigned long delayBetweenRequests = 60000; // Time between requests (1 minute)
unsigned long requestDueTime;               //time when request due

void setup()
{

    Serial.begin(115200);

    // kss._debug = true;

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