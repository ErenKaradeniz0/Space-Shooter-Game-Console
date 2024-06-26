#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h> // Include for usleep

// Screen Constants
#define MAX_X 24
#define MAX_Y 80
#define SIDE_BAR_WIDTH 20

// Size of Rocket
#define ROCKET_WIDTH 4
#define ROCKET_HEIGHT 3

// Size of Spaceship
#define SPACE_SHIP_HEIGHT 3
#define SPACE_SHIP_WIDTH 6

//Bullet Constants
#define BULLET_SPEED 1
#define MAX_BULLETS 30
#define BULLET_MOVE_DELAY 8

//Rocket Constants
#define ROCKET_SPEED 1
#define MAX_ROCKETS 4
#define ROCKET_MOVE_DELAY 80

//Max Value Generated by randRocketAxis()
#define RAND_MAX 112

typedef struct
{
    int x;
    int y;
    int active;  // Flag to bullet is active or not
    int avaible; // Flag to bullet is avaible to shot
} Bullet;

typedef struct
{
    int x;
    int y;
    int active; // Flag to rocket is active or not
} Rocket;

Bullet bullets[MAX_BULLETS];
Rocket rockets[MAX_ROCKETS];

//Delay Counters
int rocketMoveCounter = 0; // Counter to control rocket movement speed
int bulletMoveCounter = 0; // Counter to control bullet movement speed

//Flags for status of pause and quit
int quit_flag = 0;
int pause_flag = 0;

//hold current key
char current_key = '1';

int bullet_count = MAX_BULLETS;

//coordinates of spaceship
int ship_x;
int ship_y;

int score = 0;

//string arrays for printing score and bullets count on screen
char score_str[3];   // maximum number of digits is 3
char bullets_str[2]; // maximum number of digits is 2

//Draw Boundaries
void drawBoundaries()
{
    for (int i = 0; i < MAX_X; i++)
    {
        mvprintw(i, 0, "#");              // Swap parameters
        mvprintw(i, SIDE_BAR_WIDTH, "#"); // Swap parameters
        mvprintw(i, MAX_Y - 1, "#");
    }

    for (int i = 0; i < MAX_Y; i++)
    {
        mvprintw(0, i, "#");
        mvprintw(MAX_X - 1, i, "#");
    }
}

// Convert an integer to its string representation
int intToString(int num, char *buffer)
{
    int i = 0;
    int digits = 0; // Variable to store the number of digits

    if (num == 0)
    {
        buffer[i++] = '0';
        digits = 1; // If the number is zero, it has one digit
    }
    else
    {
        // Calculate the number of digits
        int temp = num;
        while (temp != 0)
        {
            digits++;
            temp /= 10;
        }

        // Convert each digit to character and store in the buffer
        while (num != 0)
        {
            int digit = num % 10;
            buffer[i++] = '0' + digit;
            num /= 10;
        }
    }
    buffer[i] = '\0';

    // Reverse the string
    int start = 0;
    int end = i - 1;
    while (start < end)
    {
        char temp = buffer[start];
        buffer[start] = buffer[end];
        buffer[end] = temp;
        start++;
        end--;
    }

    return digits; // Return the number of digits
}

//Print Score on Screen
void printScore(int x, int y)
{
    int num_digits = intToString(score, score_str);
    mvprintw(y, x, "%s", score_str);
}

//Count how many bullet left
void bulletCounter()
{
    bullet_count = 0;
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (bullets[i].avaible)
        {
            bullet_count += 1;
        }
    }
}

//Print how many bullets left
void printBulletCount(int x, int y)
{

    int num_digits = intToString(bullet_count, bullets_str);
    mvprintw(y, x, "%s", bullets_str);
    if (bullet_count < 10)
        mvprintw(y, x + 1, " ");
}

//Write Game Info
void info()
{
    // Display the welcome message and instructions
    mvprintw(1, 2, "Welcome!");          // Swap parameters
    mvprintw(2, 2, "Save the World!");   // Swap parameters
    mvprintw(3, 2, "by Eren Karadeniz"); // Swap parameters
    mvprintw(4, 2, "200101070");         // Swap parameters

    mvprintw(6, 2, "Keys");               // Swap parameters
    mvprintw(7, 2, "A to move left");     // Swap parameters
    mvprintw(8, 2, "D to move right");    // Swap parameters
    mvprintw(9, 2, "Space to Shot");      // Swap parameters
    mvprintw(10, 2, "Q to quit game");    // Swap parameters
    mvprintw(11, 2, "R to restart game"); // Swap parameters
    mvprintw(12, 2, "P to pause game");
    mvprintw(14, 2, "Win after reach"); // Swap parameters
    mvprintw(15, 2, "25 Score");        // Swap parameters
}

// Initialize game screen
void intro()
{
    // Clear the screen and draw boundaries
    clear();
    drawBoundaries();

    info();

    mvprintw(17, 2, "Bullets:");
    printBulletCount(11, 17);

    mvprintw(18, 2, "Score:");
    printScore(10, 18);
}

//Draws Spaceship
void drawSpaceship()
{
    mvprintw(ship_y, ship_x, "A  I  A");
    mvprintw(ship_y + 1, ship_x, "A /-\\ A");
    mvprintw(ship_y + 2, ship_x, "/o o o\\");
}

// Clears the old position of the spaceship
void clearSpaceship()
{
    // Clear the old position of the spaceship
    mvprintw(ship_y, ship_x, "       ");
    mvprintw(ship_y + 1, ship_x, "       ");
    mvprintw(ship_y + 2, ship_x, "       ");
}

//Move bullet
void moveBullet(int index)
{
    if (bulletMoveCounter % BULLET_MOVE_DELAY == 0)
    {
        mvprintw(bullets[index].y, bullets[index].x, " "); // Clear previous bullet position
        bullets[index].y -= BULLET_SPEED;                  // Move the bullet upwards
        if (bullets[index].y > 0)
        {
            mvprintw(bullets[index].y, bullets[index].x, "^"); // Draw the bullet
        }
        else
            bullets[index].active = 0;
    }
}

//Control movement of all bullets
void moveBullets()
{

    // Move all active bullets
    for (int index = 0; index < MAX_BULLETS; index++)
    {
        if (!pause_flag)
        {
            if (bullets[index].active && !bullets[index].avaible)
            {
                mvprintw(bullets[index].y, bullets[index].x, "^");
                moveBullet(index);
            }
        }
    }
    // Increment the bullet move counter
    bulletMoveCounter++;
    // Reset the counter to prevent overflow
    if (bulletMoveCounter >= BULLET_MOVE_DELAY)
        bulletMoveCounter = 0;
}

void shotBullet(Bullet *bullet)
{
    bullet->active = 1;
    bullet->avaible = 0;
    bullet->x = ship_x + 3; // Adjust bullet position to appear from spaceship's center
    bullet->y = ship_y - 1;
}

//Draw Rocket
void drawRocket(int x, int y)
{
    mvprintw(y, x, "\\||/");
    mvprintw(y + 1, x, "|oo|");
    mvprintw(y + 2, x, " \\/");
}

//ClearRocket
void clearRocket(int x, int y)
{
    mvprintw(y, x, "    ");
    mvprintw(y + 1, x, "    ");
    mvprintw(y + 2, x, "   ");
}

//Get System Timer for random
unsigned int getSystemTimerValue()
{
    unsigned int val;
    // Read the value of the system timer (assuming x86 architecture)
    asm volatile("rdtsc" : "=a"(val));
    return val;
}

// Define some global variables for the random number generator
static unsigned long next;

//Generate a pseudo-random integer
int rand(void)
{
    next = getSystemTimerValue();
    next = next * 1103515245 + 12345;
    return (unsigned int)(next / 65536) % RAND_MAX;
}

//Randomize X axis of Rockets due to block collision of rockets
int randRocketAxis()
{
    int min_x = SIDE_BAR_WIDTH + 1;
    int max_x = MAX_Y - ROCKET_WIDTH - 1;
    int x = rand();
    while (min_x > x || x > max_x)
    {
        x = rand();
    }
    return x;
}

//Generate a single rocket from passive rocket
void generateRocket(Rocket *rocket)
{
    int newRocketX, newRocketY;
    int collisionDetected;

    do
    {
        newRocketX = randRocketAxis(); // Generate random position for the new rocket
        newRocketY = 1;

        // Check for collision with existing rockets based on X position only
        collisionDetected = 0;
        for (int j = 0; j < MAX_ROCKETS; j++)
        {
            if (rockets[j].active &&
                (newRocketX >= rockets[j].x - ROCKET_WIDTH && newRocketX <= rockets[j].x + ROCKET_WIDTH)) // Check only X position
            {
                collisionDetected = 1;
                break;
            }
        }
    } while (collisionDetected);

    // Set the position of the new rocket
    rocket->x = newRocketX;
    rocket->y = newRocketY;
    rocket->active = 1;
}

//Generate Rockets
void generateRockets()
{
    // Generate new rockets if there are inactive rockets
    for (int i = 0; i < MAX_ROCKETS; i++)
    {
        if (!rockets[i].active)
        {
            generateRocket(&rockets[i]);
        }
    }
}

//Move single rocket
void moveRocket(int index)
{
    if (rocketMoveCounter % ROCKET_MOVE_DELAY == 0)
    {                                                    // Move the rocket every ROCKET_MOVE_DELAY frames
        clearRocket(rockets[index].x, rockets[index].y); // Clear previous rocket position
        rockets[index].y += ROCKET_SPEED;                // Move the rocket downwards
        drawRocket(rockets[index].x, rockets[index].y);
    }
}

//Control movement of rockets
void moveRockets()
{
    // Draw and move the rocket
    for (int i = 0; i < MAX_ROCKETS; i++)
    {
        if (!pause_flag)
        {
            if (rockets[i].active)
            {
                drawRocket(rockets[i].x, rockets[i].y);
                moveRocket(i);
            }
        }
    }

    // Increment the rocket move counter
    rocketMoveCounter++;
    // Reset the counter to prevent overflow
    if (rocketMoveCounter >= ROCKET_MOVE_DELAY)
        rocketMoveCounter = 0;
    if (current_key != 'p')
    {
        generateRockets();
    }
}

//Init all bullets
void initBullets()
{
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        bullets[i].x = 1;
        bullets[i].y = 1;
        bullets[i].active = 0;
        bullets[i].avaible = 1;
    }
}

//Init all rockets
void initRockets()
{
    for (int i = 0; i < MAX_ROCKETS; i++)
    {
        int newRocketX, newRocketY;
        int collisionDetected;

        do
        {
            // Generate random position for the new rocket
            newRocketX = randRocketAxis(); // Adjust range to prevent overflow
            newRocketY = 1;                // Adjust range as needed

            // Check for collision with existing rockets based on X position only
            collisionDetected = 0;
            for (int j = 0; j < i; j++)
            {
                if (rockets[j].active &&
                    (newRocketX >= rockets[j].x - ROCKET_WIDTH && newRocketX <= rockets[j].x + ROCKET_WIDTH)) // Check only X position
                {
                    collisionDetected = 1;
                    i = 0;
                    break;
                }
            }
        } while (collisionDetected);

        // Set the position of the new rocket
        rockets[i].x = newRocketX;
        rockets[i].y = newRocketY;
        rockets[i].active = 1;
    }
}

//Check for collision between bullet and rocket
int collisionBullet()
{
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (bullets[i].active)
        {
            for (int j = 0; j < MAX_ROCKETS; j++)
            {
                if (rockets[j].active &&
                    bullets[i].x >= rockets[j].x && bullets[i].x < rockets[j].x + 7 &&
                    bullets[i].y >= rockets[j].y && bullets[i].y < rockets[j].y + 5)
                {
                    score += 1;

                    printScore(10, 18);
                    bullets[i].active = 0; // Deactivate bullet
                    rockets[j].active = 0; // Deactivate rocket
                    mvprintw(bullets[i].y, bullets[i].x, " ");
                    clearRocket(rockets[j].x, rockets[j].y);
                    break;
                }
            }
        }
    }
}

//Game Over when player loses
void gameOver()
{
    clear();
    drawBoundaries();
    info();
    mvprintw(12, 35, "You lost, Press R for Play Again");
    mvprintw(13, 46, "Score: ");
    mvprintw(13, 54, "%s", score_str);
}

//Check for collision between rocket and spaceship
void collisionSpaceShip()
{
    for (int i = 0; i < MAX_ROCKETS; i++)
    {
        // Check if any of the edges of the rocket box lie outside the spaceship box

        if (ship_x <= rockets[i].x + ROCKET_WIDTH - 1 && ship_x + SPACE_SHIP_WIDTH - 1 >= rockets[i].x && rockets[i].y + ROCKET_HEIGHT >= ship_y)
        {
            quit_flag = 1;
            gameOver();
            mvprintw(11, 36, "Spaceship destroyed by rocket");
        }
    }
}

//Init Game
void init()
{

    initBullets();
    initRockets();
    intro();

    ship_x = (MAX_Y - SPACE_SHIP_WIDTH + SIDE_BAR_WIDTH) / 2; // Starting position for spaceship
    ship_y = MAX_X - SPACE_SHIP_HEIGHT - 1;                             // Adjusted starting position for the spaceship
}

//Quit game when pressed quit
void quitGame()
{
    clear();
    drawBoundaries();
    info();
    mvprintw(12, 35, "Press R for Play Again");
}

//Restart game when pressed restart
void restartGame()
{

    init(); // Initialize the game
}

//Handle how the program runs based on which key is pressed
void handleUserInput(char current_key, Bullet bullets[MAX_BULLETS])
{
    if (!pause_flag)
    {

        switch (current_key)
        {
        case 'a':
            if (ship_x - 1 > SIDE_BAR_WIDTH)
            {
                clearSpaceship(ship_x, ship_y);
                (ship_x)--;
            }
            break;
        case 'd':
            if (ship_x + 1 < MAX_Y - 7)
            {
                clearSpaceship(ship_x, ship_y);
                (ship_x)++;
            }
            break;
        case ' ':
            for (int i = 0; i < MAX_BULLETS; i++)
            {
                if (!bullets[i].active && bullets[i].avaible)
                {
                    shotBullet(&bullets[i]);
                    bulletCounter();
                    printBulletCount(11, 17);
                    break;
                }
            }
            break;
        case 'q':
            score = 0;
            quitGame();
            bullet_count = MAX_BULLETS;
            quit_flag = 1;
            break;
        case 'r':
            score = 0;
            quit_flag = 0;
            bullet_count = MAX_BULLETS;
            restartGame(); // Restart the game
            break;
        case 'p':
            pause_flag = !pause_flag; // Toggle pause_flag
            if (pause_flag)
            {
                mvprintw(10, 35, "Paused, Press p to continue");
                usleep(50000);
            }
            break;
        }
    }
    else
    {
        if (current_key == 'p')
        {
            pause_flag = 0;
            mvprintw(10, 35, "                                 ");
        }
    }
}

//Win condition
void winGame()
{
    clear();
    drawBoundaries();
    info();
    mvprintw(12, 44, "You Win");
    mvprintw(13, 37, "Press R for Play Again");
}

//Continue Game unless win or lose or pause
int continueGame()
{
    // Check if all rockets have reached the bottom of the screen

    int rocketReachedBottom = 0;
    for (int i = 0; i < MAX_ROCKETS; i++)
    {
        if (rockets[i].y >= MAX_X)
        {
            rocketReachedBottom = 1;
            if (rocketReachedBottom)
            {
                quit_flag = 1;
                gameOver();
                return 0;
            }
        }
    }

    if (score == 25)
    {
        quit_flag = 1;
        winGame();
        score = 0;
        return 0;
    }

    return 1;
}

//main function
void main(void)
{
    // Initialize the curses library
    initscr();

    resize_term(25, 80);

    // Don't display typed characters on the screen
    noecho();

    // Enable special keyboard input (e.g., arrow keys)
    keypad(stdscr, TRUE);

    // Hide the cursor
    curs_set(0);

    // Make getch a non-blocking call
    nodelay(stdscr, TRUE);

    // Print a message
    printw("Press arrow keys to move the spaceship. Press spacebar to fire bullets. Press q to quit.");

    init();

    // game loop
    while (1)
    {
        // Get keyboard input without waiting for Enter
        char current_key = getch();
        // Make getch a non-blocking call
        nodelay(stdscr, TRUE);
        while (quit_flag == 0 && continueGame())
        {
            // Get keyboard input without waiting for Enter
            current_key = getch();
            // Make getch a non-blocking call
            nodelay(stdscr, TRUE);

            handleUserInput(current_key, bullets);
            if (current_key == 'q')
            {
                break;
            }

            drawSpaceship(ship_x, ship_y);
            moveBullets();
            moveRockets();
            // Check for collision between bullets and rockets
            collisionBullet();
            collisionSpaceShip();
            refresh();
            napms(10); // Introduce a small delay
        }

        if (current_key == 'r')
        {
            quit_flag = 0;
            bullet_count = MAX_BULLETS;
            restartGame(); // Restart the game
        }
    }

    // End curses mode
    endwin();
}