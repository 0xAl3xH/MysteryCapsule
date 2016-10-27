#include "Board.h"

Board Board_newBlankBoard(void) {
    Board newBoard;
    for (uint8_t i = 0; i < 4; i++)
        for (uint8_t j = 0; j<4; j++)
            // Initialize values to 0, empty block 
            newBoard.grid[i][j] = (Block) {.value=0}; 
    return newBoard;
}

Board Board_newBoard(uint16_t grid[4][4]) {
    Board newBoard;
    for (uint8_t i = 0; i <4; i++)
        for (uint8_t j = 0; j < 4; j++) {
            newBoard.grid[i][j].value = grid[i][j];
        }
    return newBoard;
}

Boolean Board_equal(Board * board1, Board * board2) {
    Board boardA = *board1;
    Board boardB = *board2;
    for (uint8_t i = 0; i < 4; i++)
        for (uint8_t j = 0; j < 4; j++) {
           if (boardA.grid[i][j].value != boardB.grid[i][j].value)
              return FALSE; 
        }
    return TRUE;
}

void decrement (int8_t *counter) {
    (*counter)--;
}

void increment (int8_t *counter) {
    (*counter)++;
}

uint8_t greaterThan (int8_t *counter) {
    return (*counter) >= 0; 
}

uint8_t lessThan (int8_t *counter) {
    return (*counter) < 4;
}

Boolean Board_shift(Direction dir, Board *gameBoard) {
    // Make a copy of the original board
    Board board = * gameBoard;
    
    int8_t row, col;
    int8_t *outerLoop, *innerLoop;
    void (*outerIncFnPtr)(int8_t *);
    void (*innerIncFnPtr)(int8_t *);
    uint8_t (*outerConFnPtr)(int8_t *);
    uint8_t (*innerConFnPtr)(int8_t *);
    uint8_t outerReloadVal;
    uint8_t innerReloadVal;
    int8_t moveIndexReload;

    if (dir == LEFT) {
        outerReloadVal = 0;
        innerReloadVal = 0;
        outerLoop = &row;
        innerLoop = &col;
        outerIncFnPtr = &increment; 
        innerIncFnPtr = &increment;  
        outerConFnPtr = lessThan;
        innerConFnPtr = lessThan;
        moveIndexReload = 0;
    }

    if (dir == RIGHT) {
        outerReloadVal = 0;
        innerReloadVal = 3;
        outerLoop = &row;
        innerLoop = &col;
        outerIncFnPtr = &increment;
        innerIncFnPtr = &decrement;
        outerConFnPtr = lessThan;
        innerConFnPtr = greaterThan;
        moveIndexReload = 3;
    }

    if (dir == DOWN) {
        outerReloadVal = 0;
        innerReloadVal = 3;
        outerLoop = &col;
        innerLoop = &row;
        outerIncFnPtr = &increment;
        innerIncFnPtr = &decrement;
        outerConFnPtr = lessThan;
        innerConFnPtr = greaterThan;
        moveIndexReload = 3;
    }

    if (dir == UP) {
        outerReloadVal = 0;
        innerReloadVal = 0;
        outerLoop = &col;
        innerLoop = &row;
        outerIncFnPtr = &increment;
        innerIncFnPtr = &increment;
        outerConFnPtr = lessThan;
        innerConFnPtr = lessThan;
        moveIndexReload = 0;
    }

    for (*outerLoop = outerReloadVal; (*outerConFnPtr)(outerLoop); (*outerIncFnPtr)(outerLoop)) {
        // Keep track of value to find pair for merge
        uint16_t seenVal = 0;
        // Keep track of where to insert merges, or shift blocks
        int8_t moveIndex = moveIndexReload; 

        uint16_t newRow[4] = {0,0,0,0};

        // Start merging
        for (*innerLoop = innerReloadVal; (*innerConFnPtr)(innerLoop); (*innerIncFnPtr)(innerLoop)) {
            uint16_t boardVal = board.grid[row][col].value;
            //Found a non-empty block 
            if(boardVal) {
                //Matching blocks, combine and shift
                if (seenVal == boardVal) {
                    newRow[moveIndex] = 2 * seenVal;
                    (*innerIncFnPtr)(&moveIndex);
                    seenVal = 0; 
                } else {
                    //Not matching block, shift old block and look for new block's pair
                    if(seenVal) {
                        newRow[moveIndex] = seenVal;
                        (*innerIncFnPtr)(&moveIndex);
                    }
                    //Look for new pair
                    seenVal = boardVal;
                }
            }        
        }
        
        //Can't find a pair of matching blocks and at the end of col/row, shift block
        if (seenVal) {
            newRow[moveIndex] = seenVal;
            (*innerIncFnPtr)(&moveIndex);
        }

        for (*innerLoop = moveIndexReload; (*innerConFnPtr)(innerLoop); (*innerIncFnPtr)(innerLoop)) {
            gameBoard -> grid[row][col].value = newRow[(*innerLoop)];
        }
    }     
    return !Board_equal(gameBoard, &board);
}

Boolean Board_gameOver(Board *board) {
    Board gameBoard = *board;
    Direction allDirs[4] = {UP,DOWN,LEFT,RIGHT};
    for (int i = 0; i < 4; i++) {
        if(!Board_shift(allDirs[i], &gameBoard))
            return TRUE; 
    }
    return FALSE;
}
