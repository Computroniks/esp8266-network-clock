// SPDX-FileCopyrightText: 2023 Matthew Nickson <mnickson@sidingsmedia.com>
// SPDX-License-Identifier: MIT

#ifndef MAIN_WIFI_INIT_H_
#define MAIN_WIFI_INIT_H_

#include "esp_event.h"

// initialise WiFi in station mode
void wifi_init_station();

// Get the SSID for the softAP
// Use the prefix PROV_ followed by the last three chunks of the
// devices MAC address
void wifi_get_ssid(char* ssid, int max_len);

// Handler for WiFi provisioning system events
void event_handler_wifi_prov(
    void* arg,
    esp_event_base_t base,
    int id,
    void* data
);

// Handler for WiFi system events
void event_handler_wifi(
    void* arg,
    esp_event_base_t base,
    int id,
    void* data
);

// Handler for IP system events
void event_handler_ip(
    void* arg,
    esp_event_base_t base,
    int id,
    void* data
);

// Create event loop and register events
void wifi_init_events();

// Initialise TCP/IP and WiFi interface
void wifi_init_net();

// Initialise the mDNS service
void wifi_init_mdns();

// Initialise provisioning and check if we actually need to do anything
void wifi_init_provisioning();

// Init the default NVS partition for key value storage
void init_non_volatile_storage();

// Provision this device
void network_init();

#endif // MAIN_WIFI_INIT_H_
