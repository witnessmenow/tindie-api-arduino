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

void printProductInfoToSerial(ProductInfo productInfo)
{
    if (!productInfo.error)
    {
        Serial.println("--------- Product Info ---------");

        Serial.print("Product: ");
        Serial.println(productInfo.product);

        Serial.print("Quantity: ");
        Serial.println(productInfo.quantity);

        Serial.print("Model Number: ");
        Serial.println(productInfo.model_number);

        Serial.print("Options: ");
        Serial.println(productInfo.options);

        Serial.print("Price Total: $");
        Serial.println(productInfo.price_total);

        Serial.print("Price per unit: $");
        Serial.println(productInfo.price_unit);

        Serial.print("SKU: ");
        Serial.println(productInfo.sku);

        Serial.println("------------------------");
    }
}

void printOrderInfoToSerial(OrderInfo orderInfo)
{
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

        char url[50];
        sprintf(url, TINDIE_ORDER_URL_FORMAT, orderInfo.number);
        Serial.print("Order URL: ");
        Serial.println(url);

        Serial.println("Products Bought: ");
        int numProductInfo = orderInfo.number_of_products;
        if (orderInfo.number_of_products > TINDIE_MAX_PRODUCTS_IN_ORDER)
        {
            Serial.print("Note: Only printing the first 5 of ");
            Serial.println(TINDIE_MAX_PRODUCTS_IN_ORDER);
            numProductInfo = TINDIE_MAX_PRODUCTS_IN_ORDER;
        }
        for (int i = 0; i < numProductInfo; i++)
        {
            printProductInfoToSerial(orderInfo.products[i]);
        }

        Serial.println("------------------------");
    }
    else
    {
        Serial.print("Order Info had an error");
    }
}

void loop()
{

    if (millis() > requestDueTime)
    {
        Serial.print("Free Heap: ");
        Serial.println(ESP.getFreeHeap());

        Serial.print("Most recent order:");
        Serial.println(TINDIE_USER);
        OrderInfo orderInfo = tindie.getOrderInfo();

        // getOrderInfo() has two optional params
        // offset (int) - how far offset from the most recent order e.g. 1 would give the second most recent order
        // shipped (int) - 1 for shipped only, 0 for unshipped only. -1 for both (default)

        printOrderInfoToSerial(orderInfo);

        delay(100);
        //get second most recent order
        Serial.print("Second most recent order:");
        orderInfo = tindie.getOrderInfo(1);
        printOrderInfoToSerial(orderInfo);

        requestDueTime = millis() + delayBetweenRequests;
    }
}