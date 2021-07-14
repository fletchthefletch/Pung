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

// Load app
void app_main()
{
    // Set graphics
    graphics_init       ();
    set_orientation     (1);
    setColours          (1);

    // Set inputs
    gpio_set_direction  (0, GPIO_MODE_INPUT);
    gpio_set_direction  (35, GPIO_MODE_INPUT);

    // Declare player name
    for (int i = 0; i < 10; i++)
    {
        name[i] = '\0';
    }

    // Declare game socket
    gameSocket          = 20;
    oldSocket           = 0;

    // Start main task with handler for pausing
    mainHandle          = NULL;
    xTaskCreate         (mainTask, "mainTask", 3000, &mainHandle, 1, mainHandle);
}

// Main run in task to free up wifi between cycles
void mainTask(TaskHandle_t *MainTask)
{
    // Intital print
    cls             (bgColour);
    setFontColour   (tHighlight.r, tHighlight.g, tHighlight.b);
    setFont         (FONT_DEJAVU24);
    print_xy        ("Pung", CENTER, 80);
    setFontColour   (tColour.r, tColour.g, tColour.b);
    setFont         (FONT_SMALL);
    print_xy        ("Play", 6, 230);
    print_xy        ("Options", 88, 226);
    send_frame      ();

    // Reset global game variables
    pungrameters    pungRameters;
                    pungRameters.sendQueue      = NULL;
                    pungRameters.recvQueue      = NULL;
                    pungRameters.MainTask       = NULL;
                    pungRameters.gameRunning    = false;
                    pungRameters.gameSocket     = 0;
    QueueHandle_t   recvQueue                   = NULL;
	QueueHandle_t   sendQueue                   = NULL;
    TaskHandle_t   	pungHandle                  = NULL;

    while (true)
    {
        // Menu
        bool menu = true;
        while (menu)
        {
            if (!gpio_get_level(0))
            {
                // Let button release
                startSleep          ('n', defaultSleep);

                // Enter game menu
                if (!name[0])
                {
                    displayBanner   (rgbToColour(255, 0, 0), "Name not set", "Define in options", "");
                    flip_frame      ();
                    startSleep      ('b', 5000000);
                    break;
                }

                // Check socket
                if (oldSocket == gameSocket)
                {
                    displayBanner   (rgbToColour(255, 0, 0), "Change socket", "See read me", "");
                    flip_frame      ();
                    startSleep      ('b', 5000000);
                    break;
                }

                switch (selectGameMode())
                {
                    case 1:
                        // Loading
                        displayBanner   (0, "Setting up server", "", "");
                        flip_frame      ();
                        startSleep      ('b', defaultSleep);

                        // Set up host server
                        startHost       ();

                        // Wait for opponent
                        if (waitPlayer())
                        {
                            break;
                        }

                        displayBanner   (0, "Starting server", "", "");
                        flip_frame      ();

                        // Connect to sockets
                        pungRameters.gameSocket = socketServerTask();
                        if (pungRameters.gameSocket == -1)
                        {
                            displayBanner   (rgbToColour(255,0,0), "Socket error", "", "");
                            flip_frame      ();
                            break;
                        }

                        // Create move queues and game switch
                        sendQueue                   = xQueueCreate(100, sizeof(uint32_t));
                        recvQueue                   = xQueueCreate(100, sizeof(uint32_t));

                        // Start comms task
                        pungRameters.gameRunning    = true;
                        pungRameters.sendQueue      = &sendQueue;
                        pungRameters.recvQueue      = &recvQueue;
                        xTaskCreate                 (serverComms, "serverComms", 2048, &pungRameters, 5, &pungRameters.commsTask);

                        displayBanner               (0, "Starting Pung", "", "");
                        flip_frame                  ();

                        // Play pong and pass queues
                        pungRameters.MainTask       = MainTask;
                        oldSocket                   = gameSocket;
                        xTaskCreate                 (pungHost, "pungHost", 4084, &pungRameters, 10, pungHandle);
                        vTaskDelete                 (NULL);
                        break;

                    case 2:
                        // Loading
                        displayBanner               (0, "Initializing wifi", "", "");
                        flip_frame                  ();
                        startSleep                  ('b', 500000);

                        // Scan for game
                        if (gameSelect(NULL))
                        {
                            break;
                        }

                        displayBanner               (0, "Connect sockets", "", "");
                        flip_frame                  ();

                        // Connect to sockets
                        pungRameters.gameSocket     = socketClientTask();
                        if (pungRameters.gameSocket == -1)
                        {
                            displayBanner   (rgbToColour(255, 0, 0), "Socket error", "", "");
                            flip_frame      ();
                            break;
                        }

                        displayBanner               (0, "Creating queues", "", "");
                        flip_frame                  ();

                        // Create move queues and game switch
                        sendQueue                   = xQueueCreate(100, sizeof(uint32_t));
                        recvQueue                   = xQueueCreate(100, sizeof(uint32_t));
                        
                        displayBanner               (0, "Creating chatter", "", "");
                        flip_frame                  ();

                        // Start comms task
                        pungRameters.gameRunning    = true;
                        pungRameters.sendQueue      = &sendQueue;
						pungRameters.recvQueue      = &recvQueue;
                        xTaskCreate                 (clientComms, "clientComms", 2048, &pungRameters, 5, &pungRameters.commsTask);

                        displayBanner               (0, "Starting Pung", "", "");
                        flip_frame                  ();

                        // Play pung
                        pungRameters.MainTask       = MainTask;
                        oldSocket                   = gameSocket;
                        xTaskCreate                 (pungJoin, "pungJoin", 4084, &pungRameters, 10, pungHandle);
                        vTaskDelete                 (NULL);
                        break;

                    default:
                        break;
                }
                menu = false;
            }

            else if (!gpio_get_level(35))
            {
                // Let button release
                startSleep('n', defaultSleep);

                // Enter options
                options();
                menu = false;
            }
            vTaskDelay(defaultDelay / portTICK_PERIOD_MS);
        }

        // Redraw main screen after options/game
        cls             (bgColour);
        setFontColour   (tHighlight.r, tHighlight.g, tHighlight.b);
        setFont         (FONT_DEJAVU24);
        print_xy        ("Pung", CENTER, 80);
        setFont         (FONT_SMALL);
        setFontColour   (tColour.r, tColour.g, tColour.b);
        print_xy        ("Play", 6, 230);
        print_xy        ("Options", 88, 226);
        flip_frame      ();

        vTaskDelay      (defaultDelay / portTICK_PERIOD_MS);
    }
}