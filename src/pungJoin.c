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

// File specific value definitions
#define iLose           0x00000003
#define iWin            0x00000002

#define myLivesBits     0x00000003
#define theirLivesBits  0x0000000C

// Static prototypes
static player   newPlayer(int xl, int xr, int yt, int yb, int s);
static void     movePlayerRight(player *player);
static void     movePlayerLeft(player *player);
static int      parseMove(uint32_t move, player *theirPlayer, joinerBall *theBall, pungrameters *Params);

// Game
void pungJoin(pungrameters *Params)
{
    // Network packets
    uint32_t myMove         = 0;
    uint32_t theirMove      = 0;

    // Player peices
    player myPlayer         = newPlayer(53, 77, 220, 225, 2);
    player theirPlayer      = newPlayer(53, 77, 20,  25,  2);
    myLives                 = 3;
    theirLives              = 3;

    // The ball object
    joinerBall theBall;
    theBall.width           = 5;
    theBall.height          = 5;
    theBall.xLeft           = 55;
    theBall.yTop            = 120;

    // Intro screen
    cls             (bgColour);
    print_xy        ("Starting", 47, 120);
    draw_rectangle  (myPlayer.xLeft, myPlayer.yTop, myPlayer.width, myPlayer.height, pColour);
    draw_rectangle  (theirPlayer.xLeft, theirPlayer.yTop, theirPlayer.width, theirPlayer.height, pColour);
    send_frame      ();
    for (int i = 0; i < 3; i++)
    {
        // Removing the dots
        draw_rectangle      (95, 120, 40, 10, bgColour);
        int pos = 95;
        for (int y = 3; y > i; y--)
        {
            print_xy    (".", pos, 120);
            pos         += 6;
        }
        flip_frame          ();
        startSleep          ('n', 1000000);
    }

    // Wait for host to start game
    uint32_t startTime = esp_timer_get_time();
    while (!theirMove)
    {
        if (esp_timer_get_time() - startTime > 3000000)
        {
            displayBanner   (rgbToColour(255, 0, 0), "Comms error", "No host response", "");       
            flip_frame      ();
            goto END;
        }

        if(xQueueReceive(*Params->recvQueue, &theirMove, 0))
        {
            parseMove(theirMove, &theirPlayer, &theBall, &Params);
        }
    }

    // Game on!
    startTime = esp_timer_get_time();
    while (true)
    {
        // Reset moves and background
        theirMove   = 0;
        myMove      = 0;

        // Receive from their move queue
        if(xQueueReceive(*Params->recvQueue, &theirMove, 0))
        {
            if(parseMove(theirMove, &theirPlayer, &theBall, &Params))
            {
                goto END;
            }
            startTime = esp_timer_get_time();
        }
        else
        {
            // If its been over three seconds throw an error and quit
            if (startTime - esp_timer_get_time() > 3000000)
            {
                displayBanner   (rgbToColour(255, 0, 0), "Comms error", "No host response", "");
                flip_frame      ();
                goto END;
            }
        }
		 
        // Get our player move
        // Right button
        if (!gpio_get_level(35))
        {
            movePlayerRight(&myPlayer);

            // Moving right on our screen is a left on their screen
            myMove = movedLeft;
        }

        // Left button
        else if (!gpio_get_level(0))
        {
            movePlayerLeft(&myPlayer);

            // Moving left on our screen is a right on their screen
            myMove = movedRight;
        }

        // Draw the board
        wait_frame      ();
        draw_rectangle  (0, 0, 135, 240, bgColour);
        draw_rectangle  (myPlayer.xLeft, myPlayer.yTop, myPlayer.width, myPlayer.height, pColour);
        draw_rectangle  (theirPlayer.xLeft, theirPlayer.yTop, theirPlayer.width, theirPlayer.height, pColour);
        draw_rectangle  (theBall.xLeft, theBall.yTop, theBall.width, theBall.height, bColour);
        drawLives       ();

        // Send my move to queue
        if (myMove)
        {
			if(xQueueSend(*Params->sendQueue, &myMove, 0) == errQUEUE_FULL)
            {
                // Error on send
                displayBanner   (rgbToColour(255, 0, 0), "Queue is full!", "No recv response", "");
                flip_frame      ();
                goto END;
            }
        }
        vTaskDelay(10 / portTICK_RATE_MS);
    }    
    END: 

    // Break down all the wifi stuff
    Params->gameRunning = false;
    while (Params->commsTask)
    {
        vTaskDelay(10 / portTICK_RATE_MS);
    }
    vQueueDelete    (*Params->sendQueue);
    vQueueDelete    (*Params->recvQueue);
    stopSTAWifi     ();

    // Give player time to read banner or press a button to skip
    startSleep('b', 3000000);

    // Restart main menu
    mainHandle      = NULL;
    xTaskCreate     (mainTask, "mainTask", 3000, &Params->MainTask, 1, mainHandle);
    vTaskDelete     (NULL);
}

// Create a new player block
static player newPlayer(int xl, int xr, int yt, int yb, int s)
{
    player Player = {
        .xLeft      = xl,
        .xRight     = xr,
        .yTop       = yt,
        .yBottom    = yb,
        .speed      = s,
        .width      = xr - xl,
        .height     = yb - yt,
    };

    return Player;
}

// Movement functions
static void movePlayerRight(player *player)
{
    if (player->xRight + player->speed >= 135)
    {
        player->xRight   = 135;
        player->xLeft    = player->xRight - player->width; 
    }
    else
    {
        player->xRight   += player->speed;
        player->xLeft    += player->speed;
    }
}

static void movePlayerLeft(player *player)
{
    if (player->xLeft - player->speed <= 0)
    {
        player->xLeft    = 0;
        player->xRight   = player->width; 
    }
    else
    {
        player->xLeft    -= player->speed;
        player->xRight   -= player->speed;
    }
}

static int parseMove(uint32_t move, player *theirPlayer, joinerBall *theBall, pungrameters *Params)
{
    // Check the player bits
    switch (((move & moveBits) >> 30))
    {
        case left:

            // 11xxxxxxxxxx indicates a left on our screen
            movePlayerLeft(theirPlayer);
            break;
        
        case right:
            
            // 10xxxxxxxxxx indicates a right on our screen
            movePlayerRight(theirPlayer);
            break;
        
        default:
            // Do nothing
            break;
    }

    // Update ball xy
    // xxxx00000000xxxxxxxxxxxxxxxxxxxx these bits
    theBall->xLeft      = 130 - ((move & (uint32_t)267386880)  >> 20);

    // xxxxxxxxxxxx00000000xxxxxxxxxxxx these bits
    theBall->yTop       = 240 - ((move & (uint32_t)1044480)    >> 12);

    // Update lives
    // xxxxxxxxxxxxxxxxxxxxxxxxxxxx0000 these bits
    myLives     = (move & myLivesBits);
    theirLives  = (move & theirLivesBits) >> 2;

    // Check the game over bits
    // xx00xxxxxxxxxxxxxxxxxxxxxxxxxxxx these bits
    switch (((move & winLoseBits) >> 28))
    {
        case iLose:

            // xx11xxxxxxxx indicates we lost
            setFont         (FONT_UBUNTU16);
            displayBanner   (rgbToColour(255, 0, 0), "They won", ":(", "");
            flip_frame      ();
            return 1;

        case iWin:

            // xx10xxxxxxxx indicates we won
            setFont         (FONT_UBUNTU16);
            displayBanner   (rgbToColour(0, 255, 0), "You won", ":)", "");
            flip_frame      ();
            return 1;
        
        default:
            // Do nothing
            break;
    }
    return 0;
}