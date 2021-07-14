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

// File specific functions and even handlers
static void             join_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
static void             startJoiner();
static wifi_ap_record_t *scanGames();

// Local variables
static EventGroupHandle_t event_group;
const int STA_CONNECTED_BIT     = BIT0;
const int STA_DISCONNECTED_BIT  = BIT1;
const int STA_DONT_SET_BIT      = BIT2;

// Scan wifi and display games to join
int gameSelect()
{
    // Initialize the wifi scanner
    startJoiner();
    wifi_scan_config_t scan_config = {
            .ssid           = 0,
            .bssid          = 0,
            .channel        = 3,
            .show_hidden    = true
    };
	 
	 // And config file
	 wifi_config_t wifi_config = {
        .sta = {
            .ssid           = "",
            .password       = "",
        },
    };

    // Drop into the scanner menu
    bool scanning = true;
    while (scanning)
    {
        // Banner
        displayBanner   (0, "Finding networks", "", "");
        flip_frame      ();

        // Scanner
        esp_wifi_scan_start         (&scan_config, true);
        wifi_ap_record_t *records   = scanGames();

        // Selection menu
        bool menu   = true;
        int select  = 62;
        while (menu)
        {
            // Banner
            cls             (bgColour);
            setFontColour   (tHighlight.r, tHighlight.g, tHighlight.b);
            print_xy        ("Possible servers", 17, 30);
            setFontColour   (tColour.r, tColour.g, tColour.b);

            // Identified networks printout
            int y = 60;
            for(int i = 0; i < 5; i++)
            {
                if (records[i].ssid[0])
                {
                    print_xy((char *)records[i].ssid, 30, y);
                }
                else
                {
                    print_xy("No record", 30, y);
                }
                y += 20;
            }

            draw_rectangle  (22, select, 4, 4, pColour);
            print_xy        ("Scan again",  30, 160);
            print_xy        ("Cancel",      30, 180);
            print_xy        ("Cycle",        6, 230);
            print_xy        ("Select",      88, 230);         
            flip_frame      (); 

            // Get inputs
            if (!gpio_get_level(0))
            {
                // Left button toggles selection
                switch (select)
                {
                    case 62:
                        select = 82;
                        break;

                    case 82:
                        select = 102;
                        break;
                    
                    case 102:
                        select = 122;
                        break;
                    
                    case 122: 
                        select = 142;
                        break;
                    
                    case 142:
                        select = 162;
                        break;

                    case 162:
                        select = 182;
                        break;

                    case 182:
                        select = 62;
                        break;  
                }
            }

            if (!gpio_get_level(35))
            {
                // Right button does the following
                switch (select)
                {
                    case 62:
                        if (records[0].ssid[0])
                        {
                            // Join to top network
                            strcpy              ((char *)wifi_config.sta.ssid, (char *)records[0].ssid);
                            esp_wifi_set_config (ESP_IF_WIFI_STA, &wifi_config);
                            if (esp_wifi_connect() != ESP_OK)
                            {
                                displayBanner   (rgbToColour(255, 0, 0), "Failed to connect", "", "");
                                flip_frame      ();
                                startSleep      ('b', 500000);
                                return 1;
                            }
                            else
                            {
                                scanning   = false;
                                menu       = false;
                                break;
                            }
                        }
                        else
                        {
                            // Or dont if it was empty
                            break;
                        }

                    case 82:
                        if (records[1].ssid[0])
                        {
                            // Same again for second network
                            strcpy              ((char *)wifi_config.sta.ssid, (char *)records[1].ssid);
                            esp_wifi_set_config (ESP_IF_WIFI_STA, &wifi_config);
                            if (esp_wifi_connect() != ESP_OK)
                            {
                                displayBanner   (rgbToColour(255, 0, 0), "Failed to connect", "", "");
                                flip_frame      ();
                                startSleep      ('b', 500000);
                                return 1;
                            }
                            else
                            {
                                scanning   = false;
                                menu       = false;
                                break;
                            }
                        }
                        else
                        {
                            break;
                        }
                    
                    case 102:
                        if (records[2].ssid[0])
                        {
                            // Same again for third network
                            strcpy              ((char *)wifi_config.sta.ssid, (char *)records[2].ssid);
                            esp_wifi_set_config (ESP_IF_WIFI_STA, &wifi_config);
                            if (esp_wifi_connect() != ESP_OK)
                            {
                                displayBanner   (rgbToColour(255, 0, 0), "Failed to connect", "", "");
                                flip_frame      ();
                                startSleep      ('b', 500000);
                                return 1;
                            }
                            else
                            {
                                scanning   = false;
                                menu       = false;
                                break;
                            }
                        }
                        else
                        {
                            break;
                        }
                    
                    case 122: 
                        if (records[3].ssid[0])
                        {
                            // Same again for fourth network
                            strcpy              ((char *)wifi_config.sta.ssid, (char *)records[3].ssid);
                            esp_wifi_set_config (ESP_IF_WIFI_STA, &wifi_config);
                            if (esp_wifi_connect() != ESP_OK)
                            {
                                displayBanner   (rgbToColour(255, 0, 0), "Failed to connect", "", "");
                                flip_frame      ();
                                startSleep      ('b', 500000);
                                return 1;
                            }
                            else
                            {
                                scanning   = false;
                                menu       = false;
                                break;
                            }
                        }
                        else
                        {
                            break;
                        }
                    
                    case 142:
                        if (records[4].ssid[0])
                        {
                            // And fifth network
                            strcpy              ((char *)wifi_config.sta.ssid, (char *)records[4].ssid);
                            esp_wifi_set_config (ESP_IF_WIFI_STA, &wifi_config);
                            if (esp_wifi_connect() != ESP_OK)
                            {
                                displayBanner   (rgbToColour(255, 0, 0), "Failed to connect", "", "");
                                flip_frame      ();
                                startSleep      ('b', 500000);
                                return 1;
                            }
                            else
                            {
                                scanning   = false;
                                menu       = false;
                                break;
                            }
                        }
                        else
                        {
                            break;
                        }

                    case 162:
                        // Rescan drops us back into the first loop with the scan
                        menu = false;
                        break;

                    case 182:
                        // Cancel returns an integer
                        return 1;             
                }
            }
            // Let other tasks run
            vTaskDelay(defaultDelay / portTICK_PERIOD_MS);
        }
        // Let other tasks run
        vTaskDelay(defaultDelay / portTICK_PERIOD_MS);
    } 

    // Return back to main with 0 on successful connection
    cls         (0);
    flip_frame  ();
    return 0;
}

// Starts starts scanning for wifi signals
static void startJoiner()
{
    // Initialize TCPIP STACK with handler
	nvs_flash_init      ();
    tcpip_adapter_init  ();

    // Set the flags and event handler
    event_group         = xEventGroupCreate();
    esp_event_loop_init (join_handler, NULL);

    // Initialize wifi
    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init                       (&wifi_init_config);
    esp_wifi_set_storage                (WIFI_STORAGE_RAM);
    esp_wifi_set_mode                   (WIFI_MODE_STA);
    esp_wifi_start                      ();
}

// Returns list of wifi signals
static wifi_ap_record_t *scanGames()
{
    // Stop the scanner and list networks
    esp_wifi_scan_stop();
    uint16_t ap_num = 5;
    static wifi_ap_record_t ap_records[5];
    esp_wifi_scan_get_ap_records(&ap_num, ap_records);
    return ap_records;
}

// Wifi event handler for joining device
static void join_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) 
	{
        xEventGroupSetBits  (event_group, STA_DISCONNECTED_BIT);
        xEventGroupClearBits(event_group, STA_CONNECTED_BIT);
    } 
	else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) 
	{
        xEventGroupSetBits  (event_group, STA_CONNECTED_BIT);
        xEventGroupClearBits(event_group, STA_DISCONNECTED_BIT);
    }
}

// Socket client task
int socketClientTask(uint32_t *buffer) 
{
	// Create server address for connect()
    struct sockaddr_in serverAddress;
	serverAddress.sin_family    = AF_INET;
	serverAddress.sin_port      = htons(gameSocket);
	inet_pton                   (AF_INET, "130.120.8.1", &serverAddress.sin_addr);
   
	// Create a server send/client receive socket
	int sock            = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    uint32_t startTime  = esp_timer_get_time();  
    int conn            = 0;
    while (esp_timer_get_time() - startTime < 3000000)
    {
        // Connect it
        conn = connect(sock, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
        if (conn == 0)
        {
            //Return the connected socket
            return sock;
        }
        vTaskDelay(25 / portTICK_RATE_MS);
    }
    return -1;
}

// Client send/receive task
void clientComms(pungrameters *params)
{
	uint32_t moveBuffer;
    while (params->gameRunning != false)
    {
		// Get move from server
        moveBuffer  = 0;
		if (recv(params->gameSocket, &moveBuffer, sizeof(uint32_t), 0) != -1)
        {
            // Check they didn't send an empty message
            if (moveBuffer)
            {
                // And send it to the queue
                xQueueSend(*params->recvQueue, &moveBuffer, 0);
            }

            // Now we send our move to the host
            moveBuffer      = 0;
            xQueueReceive   (*params->sendQueue, &moveBuffer, 0);

            // Send move to the other side
            if (send(params->gameSocket, &moveBuffer, sizeof(uint32_t), 0) == -1)
            {	  
                // ERROR
                displayBanner   (rgbToColour(255, 0, 0), "Send error!", "Restart game", "");
                startSleep      ('n', defaultSleep);
                startSleep      ('b', 3000000);
                break;
            }
        }
        else
        {
            displayBanner   (rgbToColour(255, 0, 0), "Recv error!", "Restart game", "");
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
void stopSTAWifi()
{
    xEventGroupClearBits    (event_group, STA_CONNECTED_BIT);
    xEventGroupSetBits      (event_group, STA_DISCONNECTED_BIT);
    esp_wifi_disconnect     ();   
    esp_wifi_stop           ();
	esp_wifi_deinit         ();
}