/**************************
*	Assignment 3		   
*	Damien DeCourcy 	   
*   19042551	 			   
*	&					   
*	Fletcher van Ameringen 
*	19023939			   
**************************/

// Network includes
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_task_wdt.h"
#include "nvs_flash.h"
#include "lwip/sockets.h"

// Global prototypes
void 		clientComms(pungrameters *params);
void 		serverComms(pungrameters *params);
int         socketClientTask();
int			socketServerTask(); 
int         gameSelect();
int         waitPlayer();
void        stopSTAWifi();
void        stopAPWifi();
void        startHost();

// Game socket
int gameSocket;
int oldSocket;