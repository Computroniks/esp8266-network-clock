// SPDX-FileCopyrightText: 2023 Matthew Nickson <mnickson@sidingsmedia.com>
// SPDX-License-Identifier: MIT

#include "wifi_init.hpp"

#include <string.h>

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "mdns.h"
#include "wifi_provisioning/manager.h"
#include "wifi_provisioning/scheme_softap.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "nvs.h"
#include "nvs_flash.h"

const int WIFI_CONNECTED_EVENT = BIT0;
EventGroupHandle_t wifi_event_group;

void wifi_init_station() {
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start())
}

void wifi_get_ssid(char* ssid, int max_len) {
    unsigned char mac[6];
    esp_wifi_get_mac(WIFI_IF_STA, mac);

    // Format SSID prefix + MAC: <PREFIX><XX><XX><XX>
    snprintf(
        ssid, max_len,
        "%s%02x%02x%02x",
        CONFIG_SOFTAP_SSID_PREFIX, mac[3], mac[4], mac[5]
    );
}

void event_handler_wifi_prov(
    void* arg,
    esp_event_base_t base,
    int id,
    void* data
) {
    const char TAG[] = "WIFI_PROVISIONING";
    switch (id) {
    case WIFI_PROV_START:
        ESP_LOGI(TAG, "Starting WiFi provisioning");
        break;
    case WIFI_PROV_CRED_RECV:
    {
        wifi_sta_config_t* station_config = (wifi_sta_config_t*)data;
        ESP_LOGI(
            TAG,
            "Recieved WiFi credentials\n\tSSID: %s\n\tPassword: %s",
            (const char*)station_config->ssid,
            (const char*)station_config->password
        );
        break;
    }
    case WIFI_PROV_CRED_FAIL:
    {
        wifi_prov_sta_fail_reason_t* err = (wifi_prov_sta_fail_reason_t*)data;
        ESP_LOGE(TAG, "Failed to provision device");
        if (*err == WIFI_PROV_STA_AUTH_ERROR) {
            ESP_LOGE(
                TAG,
                "Authentication error. Failed to authenticate with station"
            );
        }
        else {
            ESP_LOGE(
                TAG,
                "Could not connect to access point. Access point not found"
            );
        }
        break;
    }
    case WIFI_PROV_CRED_SUCCESS:
        ESP_LOGI(TAG, "Provisioning successful");
        break;
    case WIFI_PROV_END:
        ESP_LOGD(TAG, "Deinitialising provisioning manager");
        wifi_prov_mgr_deinit();
        break;
    default:
        ESP_LOGD(TAG, "Got unrecognised event. ID: %d", id);
        break;
    }
}

void event_handler_wifi(
    void* arg,
    esp_event_base_t base,
    int id,
    void* data
) {
    const char TAG[] = "WIFI";

    switch (id) {
    case WIFI_EVENT_STA_START:
        ESP_LOGD(TAG, "Got WIFI_EVENT_STA_START");
        esp_wifi_connect();
        break;
    case WIFI_EVENT_STA_DISCONNECTED:
        ESP_LOGI(TAG, "Disconnected. Attempting to reconnect");
        esp_wifi_connect();
        break;
    case WIFI_EVENT_AP_STACONNECTED:
        ESP_LOGI(TAG, "Station connected to SoftAP");
        break;
    case WIFI_EVENT_AP_STADISCONNECTED:
        ESP_LOGI(TAG, "Station disconnected from SoftAP");
        break;
    default:
        ESP_LOGD(TAG, "Got unrecognised event. ID: %d", id);
        break;
    }
}

void event_handler_ip(void* arg,
    esp_event_base_t base,
    int id,
    void* data
) {
    const char TAG[] = "IP";
    if (id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*)data;
        ESP_LOGI(
            TAG,
            "Connected with settings:\n\tIP: " IPSTR
            "\n\tGateway: " IPSTR
            "\n\tMask: " IPSTR,
            IP2STR(&event->ip_info.ip),
            IP2STR(&event->ip_info.netmask),
            IP2STR(&event->ip_info.gw)
        );

        // Tell the rest of the program to continue
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_EVENT);
    }
}

void wifi_init_events() {
    ESP_LOGD("NETWORK", "Registering events");

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(
        esp_event_handler_register(
        WIFI_PROV_EVENT,
        ESP_EVENT_ANY_ID,
        &event_handler_wifi_prov,
        NULL
    ));

    ESP_ERROR_CHECK(
        esp_event_handler_register(
        WIFI_EVENT,
        ESP_EVENT_ANY_ID,
        &event_handler_wifi,
        NULL
    ));

    ESP_ERROR_CHECK(
        esp_event_handler_register(
        IP_EVENT,
        ESP_EVENT_ANY_ID,
        &event_handler_ip,
        NULL
    ));
}

void wifi_init_net() {
    tcpip_adapter_init();
    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    // Do we need to do something with interfaces here?
    ESP_ERROR_CHECK(esp_wifi_init(&config));
}

void wifi_init_mdns() {
    esp_err_t err = mdns_init();
    if (err) {
        ESP_LOGE("MDNS", "Failed to init MDNS: %d", err);
    }
    else {
        mdns_hostname_set(CONFIG_MDNS_HOSTNAME);
        mdns_instance_name_set(CONFIG_MDNS_INTANCE_NAME);
    }

}

void wifi_init_provisioning() {
    const char TAG[] = "WIFI_PROVISIONING";

    wifi_prov_mgr_config_t config = {
        .scheme = wifi_prov_scheme_softap,
        .scheme_event_handler = WIFI_PROV_EVENT_HANDLER_NONE,
        .app_event_handler = WIFI_PROV_EVENT_HANDLER_NONE
    };

    ESP_ERROR_CHECK(wifi_prov_mgr_init(config));

    bool provisioned = false;

    ESP_ERROR_CHECK(wifi_prov_mgr_is_provisioned(&provisioned));

    if (!provisioned) {
        ESP_LOGI(TAG, "Starting provisioning service");
        // Last 6 characters of MAC + teminator
        char ssid[sizeof(CONFIG_SOFTAP_SSID_PREFIX) + 6 * sizeof(char) + 1];
        wifi_get_ssid(ssid, sizeof(ssid));

        // Set security level
        // Level 1 with no proof of posession
        wifi_prov_security_t sec_level = WIFI_PROV_SECURITY_1;
        ESP_ERROR_CHECK(
            wifi_prov_mgr_start_provisioning(sec_level, NULL, ssid, NULL)
        );
    }
    else {
        ESP_LOGI(TAG, "Device already provisioned. Starting station");
        wifi_prov_mgr_deinit();
        wifi_init_station();
    }
}

void init_non_volatile_storage() {
    const char TAG[] = "NVS";
    ESP_LOGI(TAG, "Initialising NVS");
    esp_err_t err = nvs_flash_init();

    if (err == ESP_ERR_NVS_NO_FREE_PAGES
        || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGI(TAG, "Partition truncated. Erasing.");

        // Partition has been truncated and needs to be cleared
        ESP_ERROR_CHECK(nvs_flash_erase());

        // Retry init
        ESP_ERROR_CHECK(nvs_flash_init());
    }
}

void network_init() {
    const char TAG[] = "NETWORK_INIT";
    ESP_LOGI(TAG, "Starting network configuration");

    init_non_volatile_storage();
    wifi_init_events();  // Initialize event handlers
    wifi_init_net();  // Initialize networking
    wifi_init_mdns();  // Initialize mDNS
    wifi_init_provisioning();  // Initialize and start provisioning as required

    // Wait for connection
    xEventGroupWaitBits(
        wifi_event_group,
        WIFI_CONNECTED_EVENT,
        true,
        true,
        portMAX_DELAY
    );

    ESP_LOGI(TAG, "Finished network configuration");
}
