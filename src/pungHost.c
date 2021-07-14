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
#define iLose       0x20000000
#define iWin        0x30000000

// Static prototypes
static player   newPlayer(int xl, int xr, int yt, int yb, int s);
static void     checkTheirPlayer(player *Player, hostBall *ball);
static void     checkMyPlayer(player *Player, hostBall *ball);
static void     increaseyVelocity(hostBall *ball); 
static void     decreaseyVelocity(hostBall *ball);
static void     increasexVelocity(hostBall *ball); 
static void     decreasexVelocity(hostBall *ball);
static void     movePlayerRight(player *player);
static void     movePlayerLeft(player *player);
static void     resetBall(hostBall *theBall);
static int      moveBall(hostBall *theBall);
static void     drawBanner(bool iWon);

// Game
void pungHost(pungrameters *Params)
{
    // Check to run
    int checkRequired       = 0;

    // Timers
    uint64_t lastMoveTime   = esp_timer_get_time();
    uint64_t currentTime;

    // Network packets
    uint32_t myMove         = 0;
    uint32_t theirMove      = 0;

    // Player and ball peices
    player myPlayer         = newPlayer(53, 77, 220, 225, 2);
    player theirPlayer      = newPlayer(53, 77, 20,  25,  2);
    hostBall theBall;
    resetBall(&theBall);

    // Player lives
    myLives                 = 3;
    theirLives              = 3;

    // Intro screen
    cls                     (bgColour);
    print_xy                ("Starting", 47, 120);
    draw_rectangle          (myPlayer.xLeft, myPlayer.yTop, myPlayer.width, myPlayer.height, pColour);
    draw_rectangle          (theirPlayer.xLeft, theirPlayer.yTop, theirPlayer.width, theirPlayer.height, pColour);
    send_frame              ();
    for (int i = 0; i < 3; i++)
    {
        // Removing the dots
        draw_rectangle  (95, 120, 40, 10, bgColour);
        int pos         = 95;
        for (int y = 3; y > i; y--)
        {
            print_xy    (".", pos, 120);
            pos         += 6;
        }
        flip_frame      ();
        startSleep      ('n', 1000000);
    }

    // Game
    uint32_t startTime = esp_timer_get_time();
    while (true)
    {
        // Reset moves and peices
        theirMove       = 0;
        myMove          = 0;

        // Receive from their move queue
        if(xQueueReceive(*Params->recvQueue, &theirMove, 0))
        {
            switch ((theirMove & moveBits) >> 30)
            {
                case left:

                    // 11xxxxxxxxxx indicates a left on our screen
                    movePlayerLeft(&theirPlayer);
                    break;
                
                case right:
                    
                    // 10xxxxxxxxxx indicates a right on our screen
                    movePlayerRight(&theirPlayer);
                    break;
                
                default:
                    break;
            }
            startTime = esp_timer_get_time();
        }
        else
        {
            // If its been over three seconds throw an error and quit
            if (startTime - esp_timer_get_time() > 3000000)
            {
                displayBanner   (rgbToColour(255, 0, 0), "Comms error", "No client response", "");  
                flip_frame      ();
                break;
            }
        }
		 
        // Get our player move
        // Right button
        if (!gpio_get_level(35))
        {
            movePlayerRight     (&myPlayer);

            // Moving right on our screen is a left on their screen
            myMove              = movedLeft;
        }

        // Left button
        else if (!gpio_get_level(0))
        {
            movePlayerLeft      (&myPlayer);

            // Moving left on our screen is a right on their screen
            myMove              = movedRight;
        }

        // Stationary
        else
        {
            myPlayer.velocity   = 0;
        }

        // Check collisions
        if (checkRequired == 3)
        {
            checkMyPlayer       (&myPlayer, &theBall);
        }

        // If time greater than move delay move ball
        currentTime = esp_timer_get_time();
        if (currentTime - lastMoveTime >= 50000)
        {
            lastMoveTime        = currentTime;

            // Move ball
            checkRequired       = moveBall(&theBall);
            switch (checkRequired)
            {
                case 1:

                    // Decrease my lives and check for death
                    myLives--;
                    if (!myLives)
                    {
                        drawBanner  (false);
                        myMove      = iLose;
                        myMove      |= myLives << 2;
                        myMove      |= theirLives;
                        xQueueSend  (*Params->sendQueue, &myMove, 10);
                        goto END;
                    }
                    resetBall   (&theBall);
                    break;

                case 2:

                    // Decrease their lives and check for death
                    theirLives--;
                    if(!theirLives)
                    {
                        drawBanner  (true);
                        myMove      = iWin; 
                        myMove      |= myLives << 2;
                        myMove      |= theirLives;
                        xQueueSend  (*Params->sendQueue, &myMove, 10);
                        goto END;
                    }
                    resetBall       (&theBall);
                    break;

                case 3:

                    // Check if we hit the ball
                    checkMyPlayer   (&myPlayer, &theBall);
                    break;

                case 4:

                    // Check if they hit the ball
                    checkTheirPlayer(&theirPlayer, &theBall);
                    break;

                default:

                    // Do nothing
                    break;
            }
        }

        // Draw the board
        wait_frame          ();
        draw_rectangle      (0, 0, 135, 240, bgColour);
        draw_rectangle      (myPlayer.xLeft, myPlayer.yTop, myPlayer.width, myPlayer.height, pColour);
        draw_rectangle      (theirPlayer.xLeft, theirPlayer.yTop, theirPlayer.width, theirPlayer.height, pColour);
        draw_rectangle      (theBall.xLeft, theBall.yTop, theBall.width, theBall.height, bColour);
        drawLives           ();

        // Add ball to my move
        myMove |= theBall.xLeft         << 20;
        myMove |= theBall.yTop          << 12;

        // Add lives to my move 
        myMove |= myLives << 2;
        myMove |= theirLives;

        // Send my move to queue
        if (myMove)
        {   
			if(xQueueSend(*Params->sendQueue, &myMove, 0) == errQUEUE_FULL)
            {
                // Error on send
                displayBanner   (rgbToColour(255, 0, 0), "Queue is full!", "No recv response", "");
                flip_frame      ();
                break;
            }
        }
        vTaskDelay(10 / portTICK_RATE_MS);
    }     
    END:

    // Allow the game over move to be sent before shutting down the wifi
    while(uxQueueMessagesWaiting(*Params->sendQueue))
    {
        vTaskDelay(defaultDelay);
    }

    // Then shutdown the wifi
    Params->gameRunning = false;
    while (Params->commsTask)
    {
        vTaskDelay(10 / portTICK_RATE_MS);
    }
    vQueueDelete    (*Params->sendQueue);
    vQueueDelete    (*Params->recvQueue);
    stopAPWifi      ();

    // Give player time to read banner (press a button to skip)
    startSleep      ('b', 2500000);

    // Restart main menu
    cls             (0);
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

// Check player has hit ball (different physics to a ball hitting a player)
static void checkMyPlayer(player *Player, hostBall *ball)
{
    // Check in line Y axis
    if (((ball->yTop >= Player->yTop) && (ball->yTop <= Player->yBottom)) || ((ball->yTop < Player->yTop) && (ball->yBottom > Player->yTop) && (ball->yBottom <  Player->yBottom)))
    {
        // Currently in line X axis
        if (((ball->xLeft >= Player->xLeft) && (ball->xLeft <= Player->xRight)) || ((ball->xLeft < Player->xLeft) && (ball->xRight > Player->xLeft) && (ball->xRight < Player->xRight)))
        {
            // Collision
            ball->yVelocity *= -1;

            // Change velocity and redraw ball on paddle, depending on positioning
            if (ball->xLeft < (Player->xLeft + 3)) 
            {
                // Ball hit left quarter of paddle
                increaseyVelocity(ball);
                decreasexVelocity(ball);

                // If it was the edge-edge, invert the balls xVelocity
                if ((ball->xLeft <= (Player->xLeft + 2)) && (ball->xVelocity > 0))
                {
                    ball->xVelocity *= -1;
                }
            }
            else if (ball->xRight > (Player->xRight - 3))
            {
                // Ball hit right quarter of paddle
                increaseyVelocity(ball);
                decreasexVelocity(ball);

                // If it was the edge-edge, invert the balls xVelocity
                if ((ball->xRight >= (Player->xRight - 2)) && (ball->xVelocity < 0))
                {
                    ball->xVelocity *= -1;
                }
            } 
            else 
            {
                // Ball hit centre of paddle
                decreaseyVelocity(ball);
                increasexVelocity(ball);
            }

            // Redraw on top of the paddle              
            ball->yTop      = 215;
            ball->yBottom   = 220;
        }
    }
}

// Check ball has hit the player
static void checkTheirPlayer(player *Player, hostBall *ball)
{
    // Check in line Y axis
    if (((ball->yTop <= Player->yBottom) && (ball->yTop >= Player->yTop)) || ((ball->yTop < Player->yTop) && (ball->yBottom > Player->yTop) && (ball->yBottom <  Player->yBottom)))
    {
        // Currently in line X axis
        if (((ball->xLeft >= Player->xLeft) && (ball->xLeft <= Player->xRight)) || ((ball->xLeft < Player->xLeft) && (ball->xRight > Player->xLeft) && (ball->xRight < Player->xRight)))
        {
            // Collision
            ball->yVelocity *= -1;

            // Change velocity and redraw ball on paddle, depending on positioning
            if (ball->xLeft < (Player->xLeft + 3)) 
            {
                // Ball hit left quarter of paddle
                increaseyVelocity(ball);
                decreasexVelocity(ball);

                // If it was the edge-edge, invert the balls xVelocity
                if ((ball->xLeft <= (Player->xLeft + 2)) && (ball->xVelocity > 0))
                {
                    ball->xVelocity *= -1;
                }
            }
            else if (ball->xRight > (Player->xRight - 3))
            {
                // Ball hit right quarter of paddle
                // Ball hit left quarter of paddle
                increaseyVelocity(ball);
                decreasexVelocity(ball);

                // If it was the edge-edge, invert the balls xVelocity
                if ((ball->xRight >= (Player->xRight - 2)) && (ball->xVelocity < 0))
                {
                    ball->xVelocity *= -1;
                }
            } 
            else 
            {
                // Ball hit centre of paddle
                decreaseyVelocity(ball);
                increasexVelocity(ball);
            }

            // Redraw on top of the paddle              
            ball->yTop      = 25;
            ball->yBottom   = 30;
        }
    }
}

// Required for collisions above
static void increaseyVelocity(hostBall *ball)
{
    if ((ball->yVelocity) > 0 && (ball->yVelocity < 6))
    {
        ball->yVelocity += 1;
    } 
    else if ((ball->yVelocity) < 0 && (ball->yVelocity > -6))
    {
        ball->yVelocity -= 1;
    }
}

static void decreaseyVelocity(hostBall *ball) 
{
    if ((ball->yVelocity - 1) > 2) 
    {
        ball->yVelocity -= 1;
    }
    else if ((ball->yVelocity + 1) < -2) 
    {
        ball->yVelocity += 1;
    }
}

static void increasexVelocity(hostBall *ball)
{
    if ((ball->xVelocity > 0) && (ball->xVelocity < 6)) 
    {
        ball->xVelocity += 1;
    } 
    else if ((ball->xVelocity < 0) && (ball->xVelocity > -6))
    {
        ball->xVelocity -= 1;
    }
}

static void decreasexVelocity(hostBall *ball)
{
    if ((ball->xVelocity - 1) > 2) 
    {
        ball->xVelocity -= 1;
    }
    else if ((ball->xVelocity + 1) < -2) 
    {
        ball->xVelocity += 1;
    }
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
        player->xRight  += player->speed;
        player->xLeft   += player->speed;
    }
    player->velocity     = player->speed;
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
        player->xLeft   -= player->speed;
        player->xRight  -= player->speed;
    }
    player->velocity     = player->speed * -1;
}

static int moveBall(hostBall *theBall)
{
    // Move X
    if (theBall->xRight + theBall->xVelocity >= 135) 
    {
        // Bounce off the right hand wall
        theBall->xRight      = 135;
        theBall->xLeft       = theBall->xRight - theBall->width;
        theBall->xVelocity   *= -1;
    }
    else if (theBall->xLeft + theBall->xVelocity <= 0)
    {
        // Bounce off the left hand wall
        theBall->xLeft       = 0;
        theBall->xRight      = theBall->xLeft + theBall->width;
        theBall->xVelocity   *= -1;
    }
    else
    {
        // Normal shift
        theBall->xRight     += theBall->xVelocity;
        theBall->xLeft      += theBall->xVelocity;
    }

    // Move Y
    if (theBall->yBottom + theBall->yVelocity >= 240)
    {
        // My player loses a life
        return 1;
    }
    else if (theBall->yTop + theBall->yVelocity <= 0)
    {
        // Their player loses a life
        return 2;
    }
    else
    {
        // Normal shift
        theBall->yBottom    += theBall->yVelocity;
        theBall->yTop       += theBall->yVelocity;
    }

    // Flag checks
    if (theBall->yBottom >= 210)
    {
        return 3;
    }        
    else if (theBall->yTop <= 30)
    {
        return 4;
    }

    return 0;
}

// Reset the ball
static void resetBall(hostBall *theBall)
{
    theBall->width           = 5;
    theBall->height          = 5;
    theBall->xLeft           = 55;
    theBall->xRight          = theBall->xLeft + theBall->width;
    theBall->yTop            = 120;
    theBall->yBottom         = theBall->yTop + theBall->height;
    theBall->xVelocity       = limitedRand(-3, 3);
    while (theBall->xVelocity == 0)
    {
        // Having 0 x velocity is boring
        theBall->xVelocity       = limitedRand(-3, 3);
    }
    theBall->yVelocity       = 2;
}

// Draw end banner
static void drawBanner(bool iWon)
{
    setFont             (FONT_UBUNTU16);
    setFontColour       (0, 0, 0);
    if(iWon)
    {
        displayBanner   (rgbToColour(0, 255, 0), "You won", ":D", "");
        flip_frame      ();   
    }
    else
    {
        displayBanner   (rgbToColour(255, 0, 0), "They won", ":(", "");
        flip_frame      ();
    }                   
    drawLives           ();
    setFont             (FONT_SMALL);
    setFontColour       (tColour.r, tColour.g, tColour.b);
}