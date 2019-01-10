/*
 * Tron game simulation to be included in the server.
 *
 * This file is required to run the game. This is the section that has all the 
 * logic as to where the player moves and if they turn or lose. 
 *
 * Salomon Levy && Christian Aguilar
 * 9/Nov/2018
 */
 
/*
[0][0] [0][1] [0][2] [0][3] [0][4]
[1][0] [1][1] [1][2] [1][3] [1][4]
[2][0] [2][1] [2][2] [2][3] [2][4]
[3][0] [3][1] [3][2] [3][3] [3][4]
[4][0] [4][1] [4][2] [4][3] [4][4]
[5][0] [5][1] [5][2] [5][3] [5][4]
*/

/*
8 10
1 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 2
*/

#ifndef TRON_SIMULATION_H
#define TRON_SIMULATION_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "codes.h"

#define BOARD_WIDTH 80
#define BOARD_HEIGHT 80

typedef struct board_struct{
    int height;
    int width;
    int **spaces;
} board_t;

typedef struct player_coordinates{
    int x_position;
    int y_position;
}player_coordinates_t;

typedef struct player_status_struct{
    int player_number;
    int current_direction;
    int status;
    player_coordinates_t coordinates;
}player_status_t;

typedef struct player_struct {
  // How many players are expected
  int player_count;
  // How many players are currently connected
  int connected_players;
  // Used as barrier to ensure every player is ready at every frame
  // Protected by count_mutex
  int players_ready;
} player_t;

typedef struct game_struct {
  // Current state of board
  board_t * board;
  // 0 when not started
  int status;
  // Connected players info
  player_t * players;
  // Players status
  player_status_t * stati;
} game_t;

board_t *create_board(int size_x, int size_y);

void free_board(board_t * board);

board_t *board_from_file(char* filename);

char encode(int val);

void print_board(board_t *board);

void game_simulation(board_t *board, player_status_t * players, int player_c);

player_coordinates_t getStartPosition(board_t * board, int player_n);

direction_t getStartDirection();

void getNewCoordinates(player_status_t *player);

int getCoord(int coord, int max);

char * compressGame(game_t * game);

void decompressGame(char * message, game_t * game);

void printGame(game_t * game);

#endif
