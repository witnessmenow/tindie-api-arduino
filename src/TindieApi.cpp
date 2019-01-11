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

    client->setTimeout(5000);
    if (!client->connect(TINDIE_HOST, portNumber))
    {
        Serial.println(F("Connection failed"));
        return false;
    }

    //Serial.println(F("Connected!"));

    // Default client doesnt have a verify, need to figure something else out.
    // if (_checkFingerPrint && !client->verify(TINDIE_FINGERPRINT, TINDIE_HOST))
    // {
    //     Serial.println(F("certificate doesn't match"));
    //     return false;
    // }

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
    const size_t bufferSize = JSON_ARRAY_SIZE(1) + JSON_ARRAY_SIZE(5) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(5) + 2 * JSON_OBJECT_SIZE(9) + JSON_OBJECT_SIZE(30) + 500;
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