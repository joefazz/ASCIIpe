//
//  main.cpp
//  ASCIIpe
//
//  Created by Joe Fazzino on 08/03/2016.
//  Copyright © 2016 Joe Fazzino. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <ncurses.h>
#include <vector>
#include <chrono>
#include <random>

using namespace std;

// Constants for generator
const int NORTH = 0;
const int SOUTH = 1;
const int EAST = 2;
const int WEST = 3;
const int MAZEWIDTH = 80;
const int MAZEHEIGHT = 24;

// Prototypes
int IntroMenu();
void GameLoop(int col, int row);
void MazeGeneration(int startingX, int startingY, int endingX, int endingY, int grid[][MAZEHEIGHT]);
void ReadHighScores();

// Struct for the main character with values of x and y coordinates and the appearance
struct MAINCHAR {
    int posX;
    int posY;
    
    char mainChar = '@';
};

int myGrid[MAZEWIDTH][MAZEHEIGHT];


// Struct for the wall. Probably temporary
struct WALL {
    char wallChar = '#';
};

struct GOAL {
    int posX;
    int posY;
    
    char goalChar = '$';
};

// Global Variables
vector<int> stack;
int nx, ny;
int goalPositionX, goalPositionY;
int directions[] = { NORTH, SOUTH, EAST, WEST };

// Main function
int main(int argc, const char * argv[]) {
    
    // Message to show that debugging in the terminal is active
    for (int argi = 1; argi < argc; argi++)
    {
        if (strcmp(argv[argi], "--debug-in-terminal") == 0)
        {
            printf("Debugging in terminal enabled\n");
            getchar(); // Without this call debugging will be skipped
            break;
        }
    }
    
    // Initialisation for the ncurses screen
    initscr();
    raw();
    noecho();
    
    // Initialise the keypad for inputs
    keypad(stdscr, TRUE);
    
    // Hide the cursor
    curs_set(0);
    
    if (IntroMenu() == 1) {
        GameLoop(MAZEHEIGHT, MAZEWIDTH);
    }
    
    
    wclear(stdscr);
    endwin();
    
    return 0;
}

// Function to display the Main Menu
int IntroMenu() {
    wclear(stdscr);
    int choice; // Menu Choice Selector

    
    string menuChoice[] = { // Menu Items in Array
        " PLAY GAME ",
    //    "HIGH SCORES",
        " EXIT GAME "
    };
    char item[2]; // Scroller for menu items
    
    printw("       d8888  .d8888b.   .d8888b. 8888888 8888888                   \n");//         A

    printw("      d88888 d88P  Y88b d88P  Y88b  888     888                     \n");//         S
    
    printw("     d88P888 Y88b.      888    888  888     888                     \n");//         C
    
    printw("    d88P 888  \"Y888b.   888         888     888   88888b.   .d88b.  \n");//        I
    
    printw("   d88P  888     \"Y88b. 888         888     888   888 \"88b d8P  Y8b \n");//       I
    
    printw("  d88P   888       \"888 888    888  888     888   888  888 88888888 \n");//        A
    
    printw(" d8888888888 Y88b  d88P Y88b  d88P  888     888   888 d88P Y8b.     \n");//         R
    
    printw("d88P     888 \"Y8888P\"    \"Y8888P\" 8888888 8888888 88888P\"   \"Y8888  \n");//   T
    
    printw("                                                  888               \n");//         !
    
    printw("                                                  888               \n");//         !
    
    printw("                                                  888               \n");//         !
    
    // For Loop so that the menu items are printed to the screen in the correct area and the 1st item starts highlighted
    for (int i = 0; i < 2; i++) { // 3 menu items so i < 3
        if (i == 0) {
            wattron(stdscr, A_STANDOUT); // Highlighted attribute on
        }
        else {
            wattroff(stdscr, A_STANDOUT); // Unhighlight the rest
        }
        sprintf(item, "%-2s", menuChoice[i].c_str()); // Print the items
        mvwprintw(stdscr, 13+i, 30, item); // Tell the console where the items should be printed, start at the 13th row and increment with every loop but stay in the 30th column
    }
    
    
    int i = 0; // For the while loop
    do {
        choice = wgetch(stdscr);
        
        sprintf(item, "%-3s",  menuChoice[i].c_str()); // Remove highlighter attribute when the key is pressed
        mvwprintw(stdscr, 13+i, 30, "%s", item ); // What that said
        
        switch (choice) {
            case KEY_UP: // When the up arrow key is pressed
                i--; // Go up in the rows
                i = (i < 0) ? 1 : i; // Shorthand if statement, if true then 2 if false then i
                break;
            case KEY_DOWN: // When the down arrow key is pressed
                i++; // Go down in rows
                i = (i > 1) ? 0 : i; // As before
                break;
        }
        wattron(stdscr, A_STANDOUT); // Highlighter attribute on
        
        sprintf(item, "%-3s", menuChoice[i].c_str()); //Add the highligher to the object it switches to
        mvwprintw(stdscr, 13+i, 30, "%s", item);
        
        wattroff(stdscr, A_STANDOUT); // Highlighter attribute off
        
        if (choice == 10) {
            switch (i) {
                case 0:
                    return 1;
                case 1:
                    //ReadHighScores();
                    break;
                case 2:
                    i = 4;
                    break;
            }
            
        }
        
    }
    while (i != 4);
    return 0;
}

// Function which runs the game loop
void GameLoop(int row, int col) {
    //Clear the screen
    wclear(stdscr);
    
    // Make a border around the window
    border(0, 0, 0, 0, 0, 0, 0, 0);
    
    // Get the maximum x and y coordinates that the cursor can go to
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    maxX -= 2;
    maxY -= 2;
    
    // Initialise a MainChar called AtMan
    MAINCHAR AtMan;
    WALL TestWall;
    GOAL Goal;
    
    // Int value to be returned from keypresses
    int keyPress;
    
    // Target is used to make sure the next character that is moved to is a legal move
    chtype target;
    
    // y and x are the coordinates of the cursor at the point of the getyx() function call
    int y, x;
    
    // Position the main character in the middle of the window
    AtMan.posY = row/2-1;
    AtMan.posX = 3;
    
    Goal.posY = row/2-1;
    Goal.posX = maxX - 1;
    
    // Generate Maze
    MazeGeneration(AtMan.posX, AtMan.posY, goalPositionX, goalPositionY, myGrid);
    
    // Display the main character on the screen
    mvaddch(AtMan.posY, AtMan.posX, AtMan.mainChar);
    mvaddch(Goal.posY, Goal.posX, Goal.goalChar);
    
    // Do While loop so key inputs can be reacted to correctly
    do {
        keyPress = wgetch(stdscr);
        getyx(stdscr, y, x); // Assign y and x to the current y and x values of the cursor
        
        // Collision for border of the window
        x = (x < 2) ? 2 : x;
        y = (y < 1) ? 1 : y;
        x = (x <= maxX) ? x : x-1;
        y = (y <= maxY) ? y : y-1;
        
        switch (keyPress) {
            case KEY_UP:
                
                if ((target = mvinch(AtMan.posY - 1, AtMan.posX)) != TestWall.wallChar && y != 1 && target != '$') {
                    mvaddch(AtMan.posY, AtMan.posX, ' '); // Replace old position with a space
                    AtMan.posY--;
                    mvaddch(AtMan.posY, AtMan.posX, AtMan.mainChar); // Move to the new position
                }
                else if (target == '$') {
                    MazeGeneration(3, row/2-1, Goal.posX, Goal.posY, myGrid);
                    mvaddch(AtMan.posY, AtMan.posX, ' ');
                    AtMan.posY = row/2-1;
                    AtMan.posX = 3;
                    mvaddch(AtMan.posY, AtMan.posX, AtMan.mainChar);
                    mvaddch(Goal.posY, Goal.posX, Goal.goalChar);
                    
                }
            
                break;
            case KEY_DOWN:
                
                if ((target = mvinch(AtMan.posY + 1, AtMan.posX)) != TestWall.wallChar && y != maxY && target != '$') {
                    mvaddch(AtMan.posY, AtMan.posX, ' ');
                    AtMan.posY++;
                    mvaddch(AtMan.posY, AtMan.posX, AtMan.mainChar);
                }
                else if (target == '$') {
                    MazeGeneration(3, row/2-1, goalPositionX, goalPositionY, myGrid);
                    mvaddch(AtMan.posY, AtMan.posX, ' ');
                    AtMan.posY = row/2-1;
                    AtMan.posX = 3;
                    mvaddch(AtMan.posY, AtMan.posX, AtMan.mainChar);
                    mvaddch(Goal.posY, Goal.posX, Goal.goalChar);
                }
                
                break;
            case KEY_RIGHT:
                
                if ((target = mvinch(AtMan.posY, AtMan.posX + 1)) != TestWall.wallChar && x != maxX && target != '$') {
                    mvaddch(AtMan.posY, AtMan.posX, ' ');
                    AtMan.posX++;
                    mvaddch(AtMan.posY, AtMan.posX, AtMan.mainChar);
                }
                else if (target == '$') {
                    MazeGeneration(3, row/2-1, goalPositionX, goalPositionY, myGrid);
                    mvaddch(AtMan.posY, AtMan.posX, ' ');
                    AtMan.posY = row/2-1;
                    AtMan.posX = 3;
                    mvaddch(AtMan.posY, AtMan.posX, AtMan.mainChar);
                    mvaddch(Goal.posY, Goal.posX, Goal.goalChar);
                    
                }
                
                break;
            case KEY_LEFT:
                
                if ((target = mvinch(AtMan.posY, AtMan.posX - 1)) != TestWall.wallChar && x != 2 && target != '$') {
                    mvaddch(AtMan.posY, AtMan.posX, ' ');
                    AtMan.posX--;
                    mvaddch(AtMan.posY, AtMan.posX, AtMan.mainChar);
                }
                else if (target == '$') {
                    MazeGeneration(3, row/2-1, goalPositionX, goalPositionY, myGrid);
                    mvaddch(AtMan.posY, AtMan.posX, ' ');
                    AtMan.posY = row/2-1;
                    AtMan.posX = 3;
                    mvaddch(AtMan.posY, AtMan.posX, AtMan.mainChar);
                    mvaddch(Goal.posY, Goal.posX, Goal.goalChar);
                }
                
                break;
            case 'r':
                GameLoop(MAZEHEIGHT, MAZEWIDTH);
                break;
        }
        
    }
    while (keyPress != 27); // 27 is esc key
    IntroMenu();
}

void MazeGeneration(int startingX, int startingY, int endingX, int endingY, int grid[MAZEWIDTH][MAZEHEIGHT]) {
    
    
    stack.clear();
    
    nx = startingX;
    ny = startingY;
    
    for (int i = 2; i < MAZEWIDTH - 1; i++) {
        for (int j = 1; j < MAZEHEIGHT - 1; j++) {
            if (grid[i][j] == 1 || grid[i][j] == 2) {
                grid[i][j] = 0;
            }
        }
    }

    
    for (int i = 0; i < 6000; i++) {
        
        // Make sure there is no bias in the direction that the algorithm generates the maze        
        unsigned int seed = chrono::system_clock::now().time_since_epoch().count();
        srand(seed);
        int uBias = directions[rand() % 3];
        
        // Check to see if directions are legal, if they are go in the direction and add what happened tothe stack
        if (nx == goalPositionX && ny == goalPositionY) {
            break;
        }
        else if (directions[uBias] == NORTH && ny - 1 != 1  && grid[nx][ny-1] < 1) {
            ny -= 1;
            grid[nx][ny] = 1;
            stack.push_back(NORTH);
        }
        else if (directions[uBias] == SOUTH && ny + 1 != MAZEHEIGHT - 1 && grid[nx][ny+1] < 1) {
            ny += 1;
            grid[nx][ny] = 1;
            stack.push_back(SOUTH);
        }
        else if (directions[uBias] == EAST && nx + 1 != MAZEWIDTH - 1  && grid[nx+1][ny] < 1) {
            nx += 1;
            grid[nx][ny] = 1;
            stack.push_back(EAST);
        }
        else if (directions[uBias] == WEST && nx - 1 != 1 && grid[nx-1][ny] != 1) {
            nx -= 1;
            grid[nx][ny] = 1;
            stack.push_back(WEST);
        }
        else {
            if (stack.back() == NORTH) {
                grid[nx][ny] = 2;
                ny += 1;
                grid[nx][ny] = 2;
                stack.pop_back();
            }
            else if (stack.back() == SOUTH) {
                grid[nx][ny] = 2;
                ny -= 1;
                grid[nx][ny] = 2;
                stack.pop_back();
            }
            else if (stack.back() == EAST) {
                grid[nx][ny] = 2;
                nx -= 1;
                grid[nx][ny] = 2;
                stack.pop_back();
            }
            else if (stack.back() == WEST) {
                grid[nx][ny] = 2;
                nx += 1;
                grid[nx][ny] = 2;
                stack.pop_back();
            }
        }
    }
    
    for (int i = 2; i < MAZEWIDTH - 1; i++) {
        
        for (int j = 1; j < MAZEHEIGHT - 1; j++) {
            if (grid[i][j] == 2) {
                mvaddch(j, i, ' ');
            }
            else {
                mvaddch(j, i, '#');
            }
        }
    }
    
}
/*
TODO: SORT OUT HIGH SCORES
void ReadHighScores() {
    wclear(stdscr);
    printw("High Scores");
    char firstScore[50];
    char secondScore[50];
    char thirdScore[50];
    char fourthScore[50];
    ifstream HighScores;
    HighScores.open("../HighScores.txt", ios::in);
    
    if (HighScores.is_open()) {
        HighScores.getline(firstScore, 50);
        HighScores.getline(secondScore, 50);
        HighScores.getline(thirdScore, 50);
        HighScores.getline(fourthScore, 50);
        
    }
    
    HighScores.close();
    
}
*/