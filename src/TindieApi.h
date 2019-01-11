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

#ifndef TindieApi_h
#define TindieApi_h

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Client.h>

#define TINDIE_HOST "www.tindie.com"
// Fingerprint correct as of Jan 9th 2019
#define TINDIE_FINGERPRINT "BC 73 A5 9C 6E EE 38 43 A6 37 FC 32 CF 08 16 DC CF F1 5A 66"
#define TINDIE_TIMEOUT 1500

#define TINDIE_ORDER_END_POINT "/api/v1/order/?format=json&limit=1"

struct OrderInfo
{
    int number_of_products;
    char *shipping_country;
    char *date;
    bool shipped;
    double total_seller;
    double total_subtotal;
    bool error;
};

struct ProductInfo
{
    char *model_number;
    char *options;
    //bool pre_order;
    double price_total;
    double price_unit;
    char *product;
    int quantity;
    char *sku;
    //char *status;
    bool error;
};

typedef bool (*FingerPrintMatches)();

class TindieApi
{
  public:
    TindieApi(Client &client, char *userName, char *apiKey);
    bool makeGetRequest(char *command);
    int getOrderCount(int shipped = -1); //-1 is ignore, 0 is false, 1 is true
    int getOrderCount(bool shipped);

    int portNumber = 443;
    //bool _checkFingerPrint = true; //Fail request if fingerprint doesnt match
    bool _debug = false;
    Client *client;

  private:
    char *_apiKey;
    char *_userName;
    void closeClient();
};

#endif