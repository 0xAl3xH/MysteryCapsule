#ifndef BOARD_H
#define BOARD_H
/*
 * This is an implementation of a standard 2048 game board, which has 4x4 = 16 cells.
 * The grid is represented by a 2D array that contains Blocks
 * Additionally, board should implement functions that allow for directional shifts
 * and a funciton that checks the game over condition.
 * Users should not interact with the grid member of Board directly and should instead 
 * rely on the public functions as defined in Board.h
 */

#include <stdint.h>
#include "Block.h"

typedef struct {
    Block grid[4][4];
} Board;

/*
 * Direction of shift 
 */
typedef enum {
    LEFT,
    RIGHT,
    UP,
    DOWN
} Direction;

typedef enum {
    FALSE,
    TRUE
} Boolean;

/*
 * Constructor function for initializing a new board. Blocks in this board 
 * will have a default value of 0, meaning that the Block is an empty block  
 */
Board Board_newBlankBoard(void);

/*
 * Constructor function for initializing a new board, which takes a 2D array
 * with dimensions of 4x4 that represents the value of each block on the board
 */
Board Board_newBoard(uint16_t grid[4][4]);

/*
 * Check if two boards are equal, takes pointer to two Boards as parameters
 */
Boolean Board_equal(Board * board1, Board * board2);

/*
 * Puts a tile, either 2 or 4 as specified by the parameter, into 
 * a random, empty spot. Note that the user is responsible for 
 * ensuring distribution of twos and fours abide by 2048 rules 
 */ 
void Board_putRandom(Board * board, uint32_t randomNum, Boolean two);

/*
 * Takes a direction dir and shifts all the blocks in the board according to 
 * 2048 rules, mutating gameBoard:
 *   - Blocks of equal values are merged, but no greedy merging (merged once)
 *   - Merging starts from the same direction as shift (eg. left shift, start 
 *     merging from the left)
 *   - If a block is not merged, it is simply shifted in the direction specified
 *     if there is space
 * The function returns a boolean value of TRUE if a shift or merge occured  
 */
Boolean Board_shift(Direction dir, Board * gameBoard); 

/*
 * Checks to see if the game is over according to 2048 rules:
 *   - A shift in any direction will not result in any merges
 *   - A shift in any direction will not result in any blocks moving to new 
 *     positions
 * The function returns a boolean value of TRUE if the game is indeed over 
 */
Boolean Board_gameOver(Board *board); 

/*
 * Checks to see if the player has won the game by getting a 2048 block
 */
Boolean Board_gameWon(Board *board);

#endif
