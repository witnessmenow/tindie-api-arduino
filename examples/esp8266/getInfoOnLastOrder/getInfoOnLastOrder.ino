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

        Serial.print("Getting info on last Tindie Order for:");
        Serial.println(TINDIE_USER);
        OrderInfo orderInfo = tindie.getOrderInfo();

        // getOrderInfo() has two optional params
        // offset (int) - how far offset from the most recent order e.g. 1 would give the second most recent order
        // shipped (int) - 1 for shipped only, 0 for unshipped only. -1 for both (default)

        if (!orderInfo.error)
        {
            Serial.println("---------Order Info ---------");

            Serial.print("Number: ");
            Serial.println(orderInfo.number);

            Serial.print("Country: ");
            Serial.println(orderInfo.shipping_country);

            Serial.print("Date: ");
            Serial.println(orderInfo.date);

            Serial.print("Number of different products bought: ");
            Serial.println(orderInfo.number_of_products);

            Serial.print("Shipped: ");
            orderInfo.shipped ? Serial.println("true") : Serial.println("false");

            Serial.print("Total Price paid: ");
            Serial.println(orderInfo.total_subtotal);

            Serial.print("Total Price to seller: ");
            Serial.println(orderInfo.total_seller);

            Serial.println("------------------------");
        }

        requestDueTime = millis() + delayBetweenRequests;
    }
}