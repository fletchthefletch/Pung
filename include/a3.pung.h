/**************************
*	Assignment 3		   
*	Damien DeCourcy 	   
*   19042551	 			   
*	&					   
*	Fletcher van Ameringen 
*	19023939			   
**************************/

// Includes
#include <driver/adc.h>
#include <driver/gpio.h>
#include <esp_adc_cal.h>
#include <esp_system.h>
#include <esp_sntp.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/task.h>
#include "fonts.h"
#include "graphics.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// General code definitions
#define defaultSleep 150000
#define defaultDelay 180

// Game definitions
#define left            0x00000003
#define movedLeft       0xC0000000
#define right           0x00000002
#define movedRight      0x80000000
#define moveBits        0xC0000000
#define winLoseBits     0x30000000

// Structs
typedef struct hostBall
{
    int xLeft;
    int xRight;
    int yTop;
    int yBottom;
    int xVelocity;
    int yVelocity;
    int width;
    int height;
} hostBall;

typedef struct joinerBall
{
    int xLeft;
    int yTop;
    int width;
    int height;
} joinerBall;

typedef struct player
{
    int xLeft;
    int xRight;
    int yTop;
    int yBottom;
    int speed;
    int height;
    int width;
    int velocity;
} player;

typedef struct colour
{
    int r;
    int g;
    int b;
} colour;

typedef struct pungrameters
{
	QueueHandle_t *sendQueue;
	QueueHandle_t *recvQueue;
	TaskHandle_t  *MainTask;
    TaskHandle_t  *commsTask;
    int           gameSocket;
    bool          gameRunning;
} pungrameters;

// Global Functions
void                displayBanner(uint32_t overRide, char *stringOne, char *stringTwo, char *stringThree);
bool                startSleep(char mode, uint64_t forThisLong);
void 		        mainTask(TaskHandle_t *MainTask);  
void                pungJoin(pungrameters *Params);
void                pungHost(pungrameters *Params);       
int                 limitedRand(int min, int max);
void                setColours(int scheme);
int                 selectGameMode();
void                drawLives();
void                options();

// Global Variables
TaskHandle_t    mainHandle;
uint16_t        bgColour;
uint16_t        pColour;
uint16_t        bColour;
uint16_t        banner;
uint16_t        pupColour;
colour          tColour;
colour          tHighlight;
char            name[10];
int theirLives;
int myLives;