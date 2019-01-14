#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <conio.h>
#include "structs.c"

#define wall_marker "#"
#define mob_marker "\x1b[31mY\x1b[0m"
#define player_marker "\x1b[32mX\x1b[0m"
#define fight_marker "\x1b[33mF\x1b[0m"


// delay implementation using time.h;
void delay(unsigned int mseconds)
{
    clock_t goal = mseconds + clock();
    while (goal > clock());
}

// Allows for cross-platform implementation of cls (Terminal clears differently on different OS's);
void clearScreen()
{
    system("cls");
}

// Vanity implementation of slow, RPG-like print function;
void dialogue(char *text)
{
    int i = 0;
    delay(500);
    while(text[i] != '\0')
    {
        printf("%c", text[i]);
        delay(5);
        i++;
    }
    printf("\n");
}

// implementation of pause mechanism;
int decision(char key)
{
    fflush(stdin);
    char decision = getchar();
    if(decision == key || decision == toupper(key))
    {
        return 1;
    }
    return 0;
}

// Basic world structure initialization;
World letThereBeLight()
{
    World world;
    world.player = loadPlayer();
    world.map = loadMap();
    world.mobs = loadMobs();
    //Place player at the beginning of the map.
    world.player.x = world.map.start_x;
    world.player.y = world.map.start_y;
    return world;
}

// returns rooms around x,y
Room getRoom(Map map, int x, int y)
{
    Room room;
    // - '0' transformes (int)char into proper int
    room.N = map.rooms[x + map.x*(y - 1)] - '0';
    room.S = map.rooms[x + map.x*(y + 1)] - '0';
    room.W = map.rooms[(x - 1) + map.x*y] - '0';
    room.E = map.rooms[(x + 1) + map.x*y] - '0';
    return room;
}

// Map drawing function. Translates 1D array to 2D map and paints player, mob markers;
// the map is re-rendered every loop, could probably eliminate that and speed things up a little bit
World drawMapOfThe(World world)
{
    //for all the elements in 1D array
    for(int i = 0; i<world.map.x*world.map.y; i++)
    {
        //get cartesian coordinates
        int x = i%world.map.x;
        int y = i/world.map.x;

        // if player stands in anything thats not a corridor or a wall - it must be a mob
        // display it and/or fight it
        if (world.player.x  == x && world.player.y == y && (world.map.rooms[i] - '0') > 1)
        {
            // Yellow Fight Indicator
            printf(fight_marker);
            if(world.mobs[world.map.rooms[i] - 50].alive)
                // pass the mob id to player object for use later
                world.player.infight = world.map.rooms[i] - 50;
        }
        // if player stands at this position, draw green X
        else if (world.player.x  == x && world.player.y == y )
        {
            // Green Player indicator
            printf(player_marker);
        }
        // else draw walls, corridors and mobs
        else
            switch(world.map.rooms[i])
            {
            case '0':
                printf(wall_marker);
                break;
            case '1':
                printf(" ");
                break;
            default:
                // Mob Red Y's
                printf(mob_marker);
            }

        // Wrap the map to width
        if(i%world.map.x == world.map.x-1)
            printf("\n");
    }
    return world;
}

// Fight routine, mutates the World object
World fight(World world)
{
    Mob mob = world.mobs[world.player.infight];
    if(mob.alive)
    {
        int power = mob.lvl - world.player.lvl;
        printf("A fight between %s (lvl %i) and %s (lvl %i) started", world.player.nick, world.player.lvl+1, mob.name, mob.lvl + 1);
        if(power > 0)
        {
            dialogue("\n The monster is stonger than you and you faint during the fight. \n The last thing you feel before passing out is someone... or something dragging you out of the dungeon...");
            world.mobs[world.player.infight].lvl--;
            world.player.x = world.map.start_x;
            world.player.y = world.map.start_y;
        }
        else if(power <= 0)
        {
            dialogue("\n The monster fought bravely, but you managed to end up on top. No pesky monster will stand in your way!");
            world.player.lvl++;
            world.mobs[world.player.infight].alive = false;
        }
        // Interrupt execution
        printf(" Press Enter to continue");
        decision('\n');
    }
    else
    {
        printf("You see a dead body of a %s \n", mob.name);
    }
    // set player fight status to undefined
    world.player.infight = -1;
    return world;
}

int main()
{
    // too small console window wraps the map and makes the game literally unplayable
    printf("\n\n\n\n\n     Maximize the console window for the best experience.  \n\n     WSAD to walk, Enter to skip dialogues \n\n\n");
    decision('\n');

    // World initialization
    World world = letThereBeLight();
    //loads map, mobs, and player.

    // Into

    clearScreen();
    dialogue("\n You wake up in a dark place... How the hell did you end up in this situation...");
    dialogue(" Right, what's your name?");
    printf("  ");
    scanf("%s", &world.player.nick);
    printf(" %s", world.player.nick);
    dialogue(", that's a weird name, well... I'm not here to judge. Let's get you out of here...");
    printf("\n Press Enter to proceed;");
    decision('\n');

    // Game loop
    while(world.player.alive)
    {
        clearScreen();
        printf("%51c Dungeon Keeper\n Reach the end of dungeon by finding your way around and killing mobs. \n", ' ');

        // Check for exit condition
        if(world.player.x == world.map.exit_x && world.player.y == world.map.exit_y)
        {
            dialogue("You've reached the end of the Dungeon! Start over again? Y/n \n");
            if(decision('y'))
            {
                // Reinitialize the world, but keep player stats;
                world.mobs = loadMobs();
                world.player.x = world.map.start_x;
                world.player.y = world.map.start_y;
                //skip the loop
                continue;
            }
            return 1;
        }


        // draw the map
        world = drawMapOfThe(world);

        if(world.player.infight > -1 && world.mobs[world.player.infight].alive)
        {
            // delegate the world object to another function
            world = fight(world);
            continue;
        }

        // get info about surrounding rooms
        Room currentRoom = getRoom(world.map, world.player.x, world.player.y);

        printf("[%i,%i] %s lvl %i", world.player.x, world.player.y, world.player.nick, world.player.lvl + 1);
        printf("\n Available directions: ");
        currentRoom.N ? printf( "[W] North " ) : 0;
        currentRoom.S ? printf( "[S] South " ) : 0;
        currentRoom.W ? printf( "[A] West " ) : 0;
        currentRoom.E ? printf( "[D] East " ) : 0;

        // technically not correct, but allows the code to wait for proper input from user
        while(1)
        {
            // flush stdin to prevent spamming queue
            fflush(stdin);
            // toipper allows for playing with capslock on
            char decision = toupper(getch());

            if(currentRoom.N && decision == 'W')
            {
                world.player.y--;
                break;
            }

            if(currentRoom.S && decision == 'S')
            {
                world.player.y++;
                break;
            }

            if(currentRoom.W && decision == 'A')
            {
                world.player.x--;
                break;
            }

            if(currentRoom.E && decision == 'D')
            {
                world.player.x++;
                break;
            }

            // the entire logic preventing user from walking into walls sits ^there.
        }
    }

    return 0;
}


