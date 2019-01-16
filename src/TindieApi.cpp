/*
   Copyright (c) 2018 Brian Lough. All right reserved.

   TindieApi - An Arduino library to wrap the Tindie API

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "TindieApi.h"

TindieApi::TindieApi(Client &client, char *userName, char *apiKey)
{
    this->client = &client;
    this->_apiKey = apiKey;
    this->_userName = userName;
}

bool TindieApi::makeGetRequest(char *command)
{
    client->flush();
    client->setTimeout(TINDIE_TIMEOUT);
    if (!client->connect(TINDIE_HOST, portNumber))
    {
        Serial.println(F("Connection failed"));
        return false;
    }

    // give the esp a breather
    yield();

    // Send HTTP request
    client->print(F("GET "));
    client->print(command);
    client->println(F(" HTTP/1.1"));

    //Headers
    client->print(F("Host: "));
    client->println(TINDIE_HOST);

    client->println(F("Cache-Control: no-cache"));

    if (client->println() == 0)
    {
        Serial.println(F("Failed to send request"));
        return false;
    }

    // Check HTTP status
    char status[32] = {0};
    client->readBytesUntil('\r', status, sizeof(status));
    if (strcmp(status, "HTTP/1.1 200 OK") != 0)
    {
        Serial.print(F("Unexpected response: "));
        Serial.println(status);
        return false;
    }

    // Skip HTTP headers
    char endOfHeaders[] = "\r\n\r\n";
    if (!client->find(endOfHeaders))
    {
        Serial.println(F("Invalid response"));
        return false;
    }

    // Was getting stray characters between the headers and the body
    // This should toss them away
    while (client->available() && client->peek() != '{')
    {
        char c = 0;
        client->readBytes(&c, 1);
        if (_debug)
        {
            Serial.print("Tossing an unexpected character: ");
            Serial.println(c);
        }
    }

    // Let the caller of this method parse the JSon from the client
    return true;
}

int TindieApi::getOrderCount(int shipped)
{
    char command[200] = TINDIE_ORDER_END_POINT;
    strcat(command, "&username=");
    strcat(command, _userName);
    strcat(command, "&api_key=");
    strcat(command, _apiKey);
    if (shipped > -1)
    {
        strcat(command, "&shipped=");
        if (shipped == 0)
        {
            strcat(command, "false");
        }
        else
        {
            strcat(command, "true");
        }
    }
    if (_debug)
    {
        Serial.println(command);
    }

    // Use arduinojson.org/assistant to compute the capacity.
    const size_t bufferSize = JSON_ARRAY_SIZE(1) + JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(5) + 5 * JSON_OBJECT_SIZE(9) + JSON_OBJECT_SIZE(30) + 500;
    int orderCount = -1;
    if (makeGetRequest(command))
    {
        // Allocate JsonBuffer
        DynamicJsonBuffer jsonBuffer(bufferSize);

        // Parse JSON object
        JsonObject &root = jsonBuffer.parseObject(*client);
        if (root.success())
        {
            JsonObject &meta = root["meta"];
            orderCount = meta["total_count"];
        }
        else
        {
            Serial.println(F("Parsing failed!"));
        }
    }
    closeClient();
    return orderCount;
}

int TindieApi::getOrderCount(bool shipped)
{
    if (shipped)
    {
        return getOrderCount(1);
    }
    else
    {
        return getOrderCount(0);
    }
}

OrderInfo TindieApi::getOrderInfo(int offset, int shipped)
{
    char command[200] = TINDIE_ORDER_END_POINT;
    strcat(command, "&username=");
    strcat(command, _userName);
    strcat(command, "&api_key=");
    strcat(command, _apiKey);
    if (shipped > -1)
    {
        strcat(command, "&shipped=");
        if (shipped == 0)
        {
            strcat(command, "false");
        }
        else
        {
            strcat(command, "true");
        }
    }
    if (offset > -1)
    {
        char str[25] = {0};
        std::sprintf(str, "&offset=%d", offset);
        strcat(command, str);
    }

    if (_debug)
    {
        Serial.println(command);
    }

    // Get from https://arduinojson.org/v5/assistant/
    const size_t bufferSize = JSON_ARRAY_SIZE(1) + JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(5) + 5 * JSON_OBJECT_SIZE(9) + JSON_OBJECT_SIZE(30);
    OrderInfo orderInfo;
    // This flag will get cleared if all goes well
    orderInfo.error = true;
    if (makeGetRequest(command))
    {
        // Allocate JsonBuffer
        DynamicJsonBuffer jsonBuffer(bufferSize);

        // Parse JSON object
        JsonObject &root = jsonBuffer.parseObject(*client);
        if (root.success())
        {
            JsonObject &order = root["orders"][0];
            orderInfo.shipping_country = (char *)order["shipping_country"].as<char *>();
            orderInfo.date = (char *)order["date"].as<char *>();
            orderInfo.number = order["number"].as<long>();
            orderInfo.shipped = order["shipped"].as<bool>();
            orderInfo.total_seller = order["total_seller"].as<float>();
            orderInfo.total_subtotal = order["total_subtotal"].as<float>();

            JsonArray &items = order["items"];
            orderInfo.number_of_products = items.size();
            int maxCount = (orderInfo.number_of_products < TINDIE_MAX_PRODUCTS_IN_ORDER) ? orderInfo.number_of_products : TINDIE_MAX_PRODUCTS_IN_ORDER;
            for (int i = 0; i < orderInfo.number_of_products; i++)
            {
                orderInfo.products[i].model_number = (char *)order["items"][i]["model_number"].as<char *>();
                orderInfo.products[i].options = (char *)order["items"][i]["options"].as<char *>();
                orderInfo.products[i].price_total = order["items"][i]["price_total"].as<float>();
                orderInfo.products[i].price_unit = order["items"][i]["price_unit"].as<float>();
                orderInfo.products[i].product = (char *)order["items"][i]["product"].as<char *>();
                orderInfo.products[i].quantity = order["items"][i]["quantity"].as<int>();
                orderInfo.products[i].sku = (char *)order["items"][i]["sku"].as<char *>();
                orderInfo.products[i].error = false;
            }

            orderInfo.error = false;
        }
        else
        {
            Serial.println(F("Parsing failed!"));
        }
    }
    closeClient();
    return orderInfo;
}

void TindieApi::closeClient()
{
    if (client->connected())
    {
        if (_debug)
        {
            Serial.println(F("Closing client"));
        }
        client->stop();
    }
}