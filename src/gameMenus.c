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

// File specific definition
#define sleepDelay   100

// Prototypes
static void showHelp();
static void setName();

// Selects game mode
int selectGameMode()
{
    int select = 85;
    while (true)
    {
        // Server
        cls(bgColour);
        setFontColour(tHighlight.r, tHighlight.g, tHighlight.b);
        setFont(FONT_DEJAVU24);
        print_xy("Server", CENTER, 40);
        
        // Options
        setFont(FONT_DEJAVU18);
        setFontColour(tColour.r, tColour.g, tColour.b);
        print_xy("Host", 50, 80);
        print_xy("Join", 50, 110);
        print_xy("Exit", 50, 140);  
        
        // Buttons
        setFont(FONT_SMALL);
        print_xy("Cycle",   6, 230);
        print_xy("Select", 88, 230);
        draw_rectangle(36, select, 4, 4, pColour);        
        flip_frame(); 

        // Left button
        if (!gpio_get_level(0))
        {
            switch (select)
            {
                case 85:
                    select = 115;
                    break;
                
                case 115:
                    select = 145;
                    break;
                
                case 145: 
                    select = 85;
                    break;
            }
        }

        // Right button
        if (!gpio_get_level(35))
        {
            switch (select)
            {
                case 85:

                    // Let button release
                    startSleep('n', defaultSleep);
                    return 1;
                
                case 115:

                    // Let button release
                    startSleep('n', defaultSleep);
                    return 2;
                
                case 145: 

                    // Let button release
                    startSleep('n', defaultSleep);
                    return 0;
            }
        }

        // Let button release
        startSleep('n', defaultSleep);
    }
}

// Options menu
void options()
{
    // Local variables
    bool    options             = true;
    int     scheme              = 1;
    int     select              = 52;
    char    socketString[10];
    while (options)
    {
        // Draw options menu
        cls                         (bgColour);
		setFont                     (FONT_DEJAVU18);
        print_xy                    ("Options", CENTER, 10);
		setFont                     (FONT_SMALL);
        draw_rectangle              (5, select, 4, 4, pColour);             // Selection rectangle
        print_xy                    ("Name:",           15, 50);
        if (name[0])
        {                                                                   // Set name
            print_xy                (name,              70, 50);            // Current name
        }
        else
        {
            setFontColour           (255, 0, 0);
            print_xy                ("Not Set.",        70, 50);
            setFontColour           (tColour.r, tColour.g, tColour.b);      // Name not set
        }
        print_xy                    ("Socket:",         15, 70);
        itoa                        (gameSocket, socketString, 10);
        print_xy                    (socketString,      70, 70);
        print_xy                    ("Color scheme:",   15, 90);            // Toggle colour schemes
        draw_rectangle              (65, 120, 7, 7, bColour);               // Ball colour
        draw_rectangle              (58, 140, 20, 5, pColour);              // Player colour
        print_xy                    ("Help",            15, 160);
        print_xy                    ("Save",            15, 180);
        print_xy                    ("Cycle",            0, 230);
        print_xy                    ("Select",          99, 230);
        flip_frame();

        // Get control inputs and update menu/options
        if (!gpio_get_level(0))
        {
            // Incriment select
            switch (select)
            {
                // Option one -> two
                case 52:
                    select = 72;
                    break;

                // Option two -> three
                case 72:
                    select = 92;
                    break;

                // Option three -> four
                case 92:
                    select = 162;
                    break;

                // Option four -> five
                case 162:
                    select = 182;
                    break;

                // Option five -> one
                case 182:
                    select = 52;
                    break;
            }
        }

        else if (!gpio_get_level(35))
        {
            // Select option occurs
            switch (select)
            {
                // Enter player name
                case 52:

                    startSleep('n', defaultSleep);
                    setName();
                    break;

                // Game socket
                case 72:
                    if (gameSocket < 30)
                    {
                        gameSocket++;
                    }
                    else
                    {
                        gameSocket = 20;
                    }
                    break;

                // Color scheme
                case 92:

                    // Toggle colour scheme then change it
                    if (scheme == 4)
                    {
                        scheme = 1;
                    }
                    else
                    {
                        scheme++;
                    }
                    setColours(scheme);

                    // Let button release
                    startSleep('n', defaultSleep);
                    break;

                // Show help
                case 162:

                    // Let button release
                    startSleep('n', defaultSleep);
                    showHelp();
                    break;

                // Save options
                case 182:

                    options = false;
                    break;
            }
        }
        vTaskDelay(defaultDelay / portTICK_PERIOD_MS);
    }
}

// Option One: Get name
void setName()
{
    // This is a bit convoluted to minimise checking
    char input[5] = {'A', '\0'};
    int nIndex = strlen(name);
    bool setting = true;
    while (setting)
    {
        displayBanner(0, "Input name", name, input);
        flip_frame();

        // Left button toggles A -> Z -> Save -> Del -> Back to A
        if (!gpio_get_level(0))
        {
            input[0]++;
            if (input[0] == 91)
            {
                strcpy(input, "SAVE");
            }
            else if (input[1] == 'A')
            {
                strcpy(input, "DEL");
            }
            else if (input[1] == 'E')
            {
                input[0] = 'A';
                input[1] = '\0';
            }
        }

        // Right button accepts what is displayed
        if (!gpio_get_level(35))
        {
            if ((!input[1]) && (nIndex < 6))
            {
                name[nIndex] = input[0];
                nIndex++;
            }
            else if (input[1] == 'A')
            {
                setFontColour(tColour.r, tColour.g, tColour.b);
                setting = false;
            }
            else if ((input[1] == 'E') && (nIndex > 0))
            {
                name[nIndex - 1] = '\0';
                nIndex--;
            }
        }

        // Let button release
        vTaskDelay(defaultDelay / portTICK_PERIOD_MS);
    }
}

// Display game information
void showHelp()
{
    // Draw options menu
    cls             (bgColour);
    setFontColour   (tHighlight.r, tHighlight.g, tHighlight.b);
    setFont         (FONT_DEJAVU18);
    print_xy        ("Gameplay", 5, 10);
        
    // Help print out
    setFontColour   (tColour.r, tColour.g, tColour.b);
    draw_rectangle  (5, 30, 130, 1, rgbToColour(tColour.r, tColour.g, tColour.b));
    setFont         (FONT_SMALL);
    print_xy        ("Use the left and right",  7, 37);
    print_xy        ("buttons to move your",    7, LASTY+10);
    print_xy        ("paddle to hit the ball",  7, LASTY+10);
    print_xy        ("back to your opponent. ", 7, LASTY+10);
    print_xy        ("You have 3 lives. If",    7, LASTY+10);
    print_xy        ("you fail to hit the",     7, LASTY+10);
    print_xy        ("ball back, you will",     7, LASTY+10);
    print_xy        ("lose a life.",            7, LASTY+10);
    print_xy        ("The game ends when a",    7, LASTY+10);
    print_xy        ("player has no lives.",    7, LASTY+10);

    setFontColour   (tHighlight.r, tHighlight.g, tHighlight.b);
    setFont         (FONT_DEJAVU18);
    print_xy        ("Modes", 5, LASTY+15);
    setFontColour   (tColour.r, tColour.g, tColour.b);
    draw_rectangle  (5, 162, 130, 1, rgbToColour(tColour.r, tColour.g, tColour.b));
    setFont         (FONT_SMALL);
    print_xy        ("[Host] - Start a new",    7, 168);
    print_xy        ("         wireless game.", 7, LASTY+10);
    print_xy        ("[Join] - Scan for",       7, LASTY+20);
    print_xy        ("         games to join.", 7, LASTY+10);

    print_xy        ("Back", 0, 230);
    flip_frame      (); 

    while (true)
    {
        // Let button drops us out of the help printout
        if (!gpio_get_level(0))
        {
            // Let button release
            startSleep('n', defaultSleep);
            break;
        }
		 // Let button release
        vTaskDelay(defaultDelay / portTICK_PERIOD_MS);
    }
}

// Set the colour scheme in the game
void setColours(int scheme)
{
    switch (scheme)
    {
        case 1:
            // Background:  Black
            bgColour        = rgbToColour(0, 0, 0);

            // Player:      White
            pColour         = rgbToColour(255, 255, 255);

            // Text:        White
            tColour.r       = 255;
            tColour.g       = 255;
            tColour.b       = 255;

            // Highlight    Blue
            tHighlight.r    = 0;
            tHighlight.g    = 0;
            tHighlight.b    = 255;

            // Ball:        Dull yellow
            bColour         = rgbToColour(125, 125, 0);

            // Power Ups    Green
            pupColour       = rgbToColour(0, 255, 0);

            // Banner       Baby blue
            banner          = rgbToColour(207, 238, 250);
            break;

        case 2:
            // Background:  White
            bgColour        = rgbToColour(255, 255, 255);

            // Player:      Black
            pColour         = rgbToColour(0, 0, 0);

            // Text:        Black
            tColour.r       = 0;
            tColour.g       = 0;
            tColour.b       = 0;

            // Highlight    Red
            tHighlight.r    = 255;
            tHighlight.g    = 0;
            tHighlight.b    = 0;

            // Ball:        Dull blueish
            bColour         = rgbToColour(0, 125, 125);

            // Power Ups    Red
            pupColour       = rgbToColour(255, 0, 0);

            // Banner       Grey
            banner          = rgbToColour(220, 220, 220);
            break;

        case 3:
            // Background:  Pink
            bgColour        = rgbToColour(255, 182, 193);

            // Player:      Black
            pColour         = rgbToColour(0, 0, 0);

            // Text:        Black
            tColour.r       = 0;
            tColour.g       = 0;
            tColour.b       = 0;

            // Highlight    Yellow
            tHighlight.r    = 255;
            tHighlight.g    = 255;
            tHighlight.b    = 0;

            // Ball:        Bright Yellow
            bColour         = rgbToColour(255, 255, 0);

            // Power Ups    Blue
            pupColour       = rgbToColour(0, 0, 255);

            // Banner       Red
            banner          = rgbToColour(255, 0, 0);
            break;

        case 4:
             // Background: This
            bgColour        = rgbToColour(limitedRand(0, 255), limitedRand(0, 255), limitedRand(0, 255));

            // Player:      Colour
            pColour         = rgbToColour(limitedRand(0, 255), limitedRand(0, 255), limitedRand(0, 255));

            // Text:        Scheme
            tColour.r       = limitedRand(0, 255);
            tColour.g       = limitedRand(0, 255);
            tColour.b       = limitedRand(0, 255);

            // Highlight    Is
            tHighlight.r    = limitedRand(0, 255);
            tHighlight.g    = limitedRand(0, 255);
            tHighlight.b    = limitedRand(0, 255);

            // Ball:        Cancer
            bColour         = rgbToColour(limitedRand(0, 255), limitedRand(0, 255), limitedRand(0, 255));

            // Power Ups    
            pupColour       = rgbToColour(limitedRand(0, 255), limitedRand(0, 255), limitedRand(0, 255));

            // Banner       Grey
            banner          = rgbToColour(limitedRand(0, 255), limitedRand(0, 255), limitedRand(0, 255));
            break;
    }
    setFontColour(tColour.r, tColour.g, tColour.b);
}

// Start an interuptable sleep timer
bool startSleep(char mode, uint64_t forThisLong)
{
	uint64_t startTime;
	startTime = esp_timer_get_time();
    
    // Mode denotes which buttons can interupt the sleep
	switch(mode)
	{
		case 'n':

            // Neither button
			while(esp_timer_get_time() - startTime < forThisLong)
			{
				vTaskDelay(sleepDelay / portTICK_PERIOD_MS);
			}
			break;
			
		case 'l':

            // Left button interupt
			while(esp_timer_get_time() - startTime < forThisLong)
			{
				if (!gpio_get_level(0))
				{
					startSleep('n', defaultSleep);
					return true;
				}

				vTaskDelay(sleepDelay / portTICK_PERIOD_MS);
			}
			break;
			
		case 'r':

            // Right button
			while(esp_timer_get_time() - startTime < forThisLong)
			{
				if (!gpio_get_level(35))
				{ 
					startSleep('n', defaultSleep);
					return true;
				}
                
				vTaskDelay(sleepDelay / portTICK_PERIOD_MS);
			}
			break;
			
		case 'b':

            // Either button
			while(esp_timer_get_time() - startTime < forThisLong)
			{
				if ((!gpio_get_level(0)) || (!gpio_get_level(35)))
				{
					startSleep('n', defaultSleep);
					return true;
				}
                
				vTaskDelay(sleepDelay / portTICK_PERIOD_MS);
			}
			break;
	}

    // If interupted it returns true to denote interuption, if not it returns false
	return false;
}

// Get a random number
int limitedRand(int min, int max)
{
    return (rand() % (max + 1 - min) + min);
}

// Display a banner
void displayBanner(uint32_t overRide, char *stringOne, char *stringTwo, char *stringThree)
{
    draw_rectangle(0, 88, 135, 2, 0);
    if (overRide)
    {
        draw_rectangle(0, 90, 135, 62, overRide);
    }
    else
    {
        draw_rectangle(0, 90, 135, 62, banner);
    }
    draw_rectangle(0, 152, 135, 2, 0);

    setFontColour   (0, 0, 0);
    print_xy        (stringOne, CENTER, 100);
    print_xy        (stringTwo, CENTER, 130);

    if (stringThree[0])
    {
        setFontColour   (tHighlight.r, tHighlight.g, tHighlight.b);
        print_xy        (stringThree, CENTER, 115);
    }
    setFontColour   (tColour.r, tColour.g, tColour.b);
}

// Global function	
void drawLives() {	

    // Their lives	
    draw_rectangle(112, 7, 22, 8, bColour);	
    draw_rectangle(113, 8, 20, 6, bgColour);	
    for (int i = 0; i < theirLives; i++) 
    {	
        draw_rectangle(125 -(i*6)+3, 9, 3, 3, pColour);	
    }	

    // My lives	
    draw_rectangle(1, 230,  22, 8, bColour);	
    draw_rectangle(2, 231, 20, 6, bgColour);	
    for (int i = 0; i < 3; i++) 
    {	
        if (i < myLives) 
        {	
            draw_rectangle(((i+1)*6)-2, 232, 3, 3, rgbToColour(tHighlight.r, tHighlight.g, tHighlight.b));	
        
        } else 
        {	
            draw_rectangle(((i+1)*6)-2, 232, 3, 3, pColour);	
        }	
    }	
    send_frame();	
}	
