#include <cmath>
#include <chrono>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <conio.h>
#include <windows.h>

#include "sound.h"

typedef enum Color {
    BACKGROUND =        10,

    DEFAULT =           39,

    BLACK =             30,
    RED =               31,
    GREEN =             32,
    YELLOW =            33,
    BLUE =              34,
    MAGENTA =           35,
    CYAN =              36,
    WHITE =             37,
    
    BRIGHT_BLACK =      90,
    BRIGHT_RED =        91,
    BRIGHT_GREEN =      92,
    BRIGHT_YELLOW =     93,
    BRIGHT_BLUE =       94,
    BRIGHT_MAGENTA =    95,
    BRIGHT_CYAN =       96,
    BRIGHT_WHITE =      97,
} Color;

typedef struct Block {
    bool cellOccupied = false;
    bool isStationary = false;
    // Color color = DEFAULT;
    int color = 255;
} Block;

typedef struct Piece {
    Block block;
    int blockMask[4][4];

    // Divided by two
    int pivotX = 2;
    int pivotY = 2;
} Piece;

const Piece pieces[7] = {
    { // O
        {true, false, 220},
        
        {{0,1,1,0},
         {0,1,1,0},
         {0,0,0,0}},
        
        3, 1
    },

    { // I
        {true, false, 123},
        
        {{1,1,1,1}},
        
        1, 2
    },

    { // S
        {true, false, 196},
        
        {{0,1,1,0},
         {1,1,0,0},
         {0,0,0,0}},
        
        2, 2
    },

    { // Z
        {true, false, 46},
        
        {{1,1,0,0},
         {0,1,1,0},
         {0,0,0,0}},
        
        2, 2
    },

    { // L
        {true, false, 208},
        
        {{0,0,1,0},
         {1,1,1,0},
         {0,0,0,0}},
        
        2, 2
    },

    { // J
        {true, false, 199},
        
        {{1,0,0,0},
         {1,1,1,0},
         {0,0,0,0}},
        
        2, 2
    },

    { // T
        {true, false, 129},
        
        {{0,1,0,0},
         {1,1,1,0},
         {0,0,0,0}},
        
        2, 2
    }
};

int countDigits(int number) {
    if (number == 0) return 1;
    
    int count = 0;
    long long n = std::llabs(static_cast<long long>(number)); 
    
    while (n > 0) {
        n /= 10;
        count++;
    }
    return count;
}