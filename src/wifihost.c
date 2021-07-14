/**************************
*	Assignment 3		   
*	Damien DeCourcy 	   
*   19042551	 			   
*	&					   
*	Fletcher van Ameringen 
*	19023939			   
**************************/

// Includes
#include "a3.pung.h"
#include "a3.networking.h"

// File specific event handler
static esp_err_t host_handler(void *ctx, system_event_t *event);

// Event group
static EventGroupHandle_t event_group;
const int AP_CONNECTED_BIT      = BIT0;
const int AP_DISCONNECTED_BIT   = BIT1;

// Start the AP
void startHost()
{
    // Initilaise wifi with event handler
    nvs_flash_init              ();
    tcpip_adapter_init          ();
    tcpip_adapter_dhcps_stop    (TCPIP_ADAPTER_IF_AP);

    // Assign a static IP to the network interface
    tcpip_adapter_ip_info_t info;
    memset                      (&info, 0, sizeof(info));
    IP4_ADDR                    (&info.ip, 130, 120, 8, 1);
    IP4_ADDR                    (&info.gw, 130, 120, 8, 1);
    IP4_ADDR                    (&info.netmask, 255, 255, 255, 0);
    tcpip_adapter_set_ip_info   (TCPIP_ADAPTER_IF_AP, &info);
    tcpip_adapter_dhcps_start   (TCPIP_ADAPTER_IF_AP);

    // Set the event handler
    event_group                 = xEventGroupCreate();
    esp_event_loop_init         (host_handler, NULL);

    // Initialize wifi
    wifi_init_config_t wifiInitConfig           = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init                               (&wifiInitConfig);
    esp_wifi_set_storage                        (WIFI_STORAGE_RAM);
    esp_wifi_set_mode                           (WIFI_MODE_AP);

    wifi_config_t ap_config = {
        .ap = {
            .ssid               = "",
            .ssid_len           = 0,
            .channel            = 3,
            .authmode           = WIFI_AUTH_OPEN,
            .ssid_hidden        = false,
            .max_connection     = 1,
            .beacon_interval    = 100
        }
    };
    strcpy                  ((char *)ap_config.ap.ssid, name);
    ap_config.ap.ssid_len   = strlen(name);

    esp_wifi_set_config     (WIFI_IF_AP, &ap_config);
    esp_wifi_start          ();
}

// Wifi event handler for establish a device connection
static esp_err_t host_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id)
    {
        case SYSTEM_EVENT_AP_STACONNECTED:
            xEventGroupSetBits  (event_group, AP_CONNECTED_BIT);
            xEventGroupClearBits(event_group, AP_DISCONNECTED_BIT);
            break;

        case SYSTEM_EVENT_AP_STADISCONNECTED:
            xEventGroupSetBits  (event_group, AP_DISCONNECTED_BIT);
            xEventGroupClearBits(event_group, AP_CONNECTED_BIT);
            break;

        default:
            break;
    }

    return ESP_OK;
}

// Wait for player join event
int waitPlayer()
{
    // Timers init
    uint64_t startTime;
    uint64_t currentTime;
    int leftTime;
    char string[20];
    startTime   = esp_timer_get_time();
    currentTime = esp_timer_get_time();

    //  Create a connected stations list
    wifi_sta_list_t wifi_sta_list;
    tcpip_adapter_sta_list_t adapter_sta_list;
    memset(&wifi_sta_list, 0, sizeof(wifi_sta_list));
    memset(&adapter_sta_list, 0, sizeof(adapter_sta_list));

    // There is a 60 second timeout for devices to join
    while (currentTime - startTime < 60000000)
    {
        // Print the banner
        leftTime        = 60 - ((currentTime - startTime) / 1500000);
        itoa            (leftTime, string, 10);
        displayBanner   (0, "Await player...", "Buttons to cancel", string);
        flip_frame      ();

        // Once someone has joined, this finds them on the connected list and returns success
        esp_wifi_ap_get_sta_list    (&wifi_sta_list);
        tcpip_adapter_get_sta_list  (&wifi_sta_list, &adapter_sta_list);
        if (adapter_sta_list.num > 0)
        {
            return 0;
        }

        // Cancel or time out
        if (!gpio_get_level(35))
        {
            break;
        }
        else if (!gpio_get_level(0))
        {
            break;
        }
        vTaskDelay  (defaultDelay / portTICK_PERIOD_MS);
        currentTime = esp_timer_get_time();
    }
    vTaskDelay  (defaultDelay / portTICK_PERIOD_MS);
    return 1;
}

// Socket server task
int socketServerTask()
{
    struct sockaddr_in clientAddress;
    struct sockaddr_in serverAddress;
    int opt = 1;

    // Create the first socket port for listening on 
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // Bind our server listen socket to 130.120.8.1:22 then mark as reuseable
    serverAddress.sin_family    = AF_INET;
	serverAddress.sin_port      = htons(gameSocket);
	inet_pton                   (AF_INET, "130.120.8.1", &serverAddress.sin_addr);
    bind                        (sock, (struct sockaddr *)&serverAddress, sizeof(serverAddress));

    // Flag as listening for new connections
    listen  (sock, 5);

    // Listen for game socket connection with a 3 second time out
    socklen_t clientAddressLength   = sizeof(clientAddress);
    int conn                        = accept(sock, (struct sockaddr *)&clientAddress, &clientAddressLength);
    if (conn > 0)
    {
        // Close the listener and return the socket
        shutdown(sock, SHUT_RDWR);
        return conn;
    }
    return -1;
}

// Server send/receive task
void serverComms(pungrameters *params)
{
	uint32_t moveBuffer;
    while (params->gameRunning != false)
    {
        // Accept moves from queue regardless if there or not
        moveBuffer      = 0;
		xQueueReceive   (*params->sendQueue, &moveBuffer, 0);
		
        // Send move to the other side
		if (send(params->gameSocket, &moveBuffer, sizeof(uint32_t), 0) != -1)
        {
            // Now receive from the client
            moveBuffer  = 0;
            if (recv(params->gameSocket, &moveBuffer, sizeof(uint32_t), 0) == -1)
            {
                // ERROR
                displayBanner   (rgbToColour(255, 0, 0), "Recv error!", "Restart game", "");
                startSleep      ('n', defaultSleep);
                startSleep      ('b', 3000000);
                break;		  
            }

            // Check they didn't send an empty message
            if (moveBuffer)
            {
                xQueueSend(*params->recvQueue, &moveBuffer, 0);
            }
        }
        else
        {	  
            // ERROR
            displayBanner   (rgbToColour(255, 0, 0), "Send error!", "Restart game", "");
            startSleep      ('n', defaultSleep);
            startSleep      ('b', 3000000);
            break;
        }
    }
    // On error or game over
    shutdown            (params->gameSocket, SHUT_RDWR);
    close               (params->gameSocket);
    params->commsTask   = NULL;
    vTaskDelete         (NULL);
}

// Stop wifi gracefully
void stopAPWifi()
{
    xEventGroupClearBits    (event_group, AP_CONNECTED_BIT);
    xEventGroupSetBits      (event_group, AP_DISCONNECTED_BIT);
    esp_wifi_disconnect     ();   
    esp_wifi_stop           ();
	esp_wifi_deinit         ();
    nvs_flash_deinit        ();
}