/*
 * Tron game simulation to be included in the server.
 *
 * This file is required to run the game. This is the section that has all the 
 * logic as to where the player moves and if they turn or lose. 
 *
 * Salomon Levy && Christian Aguilar
 * 9/Nov/2018
 */
#include "tron_simulation.h"

// Create and allocate board
board_t *create_board(int size_x, int size_y){
  srand (time(NULL));
  
  // Allocate pointer to stuct
  board_t* board = malloc(sizeof(board_t));
  
  // Assign x and y
  board->width = size_x;
  board->height = size_y;
  
  // Allocate index of the stucts pixel
  board->spaces = (int**)malloc(size_y * sizeof(int*));
  
  // Allocate all necesary spaces for the board stuct
  board->spaces[0] = (int*)calloc(size_x * size_y, sizeof(int));
  for(int i = 0; i < (board->height); i++){
    board->spaces[i] = board->spaces[0] + board->width * i;
  }
  
  // Initialize to zeros
  for(int i = 0; i < (size_x * size_y); i++){
    board->spaces[0][i] = 0;
  }
  return board;
}

// Free the data 
void free_board(board_t * board){
  free(board->spaces[0]);
  free(board->spaces);
  free(board);
}

// Read board from a file
board_t* board_from_file(char* filename){
  int size_x = 0, size_y = 0;
  FILE* file = fopen(filename, "r");
  if (file == NULL){
    printf("File can't open.\n");
    exit(EXIT_FAILURE);
  }
  // Scan size
  fscanf(file, "%i %i\n", &size_x, &size_y);
  
  // Create board of that size
  board_t* board = create_board(size_x, size_y);
  
  // Read file into board
  int buffer = 0;
  for (int i = 0; i < size_y; i++){
    for (int j = 0; j < size_x; j++){
      fscanf(file, "%i", &buffer);
      // 1 == Player1
      if(buffer == 1){
        board->spaces[i][j] = 1;
      }
      // 2 ==  Player2
      else if(buffer == 2){
        board->spaces[i][j] = 2;
      }
      // Any other number will be taken as EMPTY
      else
        board->spaces[i][j] = EMPTY;
    }
  }
  fclose(file);
  return board;
}

// Encoding to make the print pretty for debugs
char encode(int val){
    switch(val){
        case EMPTY:
            return ' ';
        case PLAYER_TRAIL:
            return 'X';
        case PLAYER:
            return '1';
        default:
            return val;
    }
}

// Print board to debug
void print_board(board_t *board){
    for(int i = 0; i < board->height; i++){
        for(int j = 0; j < board->width; j++){
            printf("%c|", encode(board->spaces[i][j]));
        }
        printf("\n");
    }
    printf("\n");
}

/*
Enumeration for the position
    1         UP
  4 P 2  LEFT P RIGHT
    3         DOWN
*/

void getNewCoordinates(player_status_t *player){
  if(player->current_direction == UP){
    player->coordinates.y_position = getCoord(player->coordinates.y_position - 1, BOARD_HEIGHT);
  }
  else if(player->current_direction == LEFT){
    player->coordinates.x_position = getCoord(player->coordinates.x_position - 1, BOARD_WIDTH);
  }
  else if(player->current_direction == DOWN){
    player->coordinates.y_position = getCoord(player->coordinates.y_position + 1, BOARD_HEIGHT);
  }
  else if(player->current_direction == RIGHT){
    player->coordinates.x_position = getCoord(player->coordinates.x_position + 1, BOARD_WIDTH);
  }
  else{
      //error
  }
}

int getCoord(int coord, int max) {
  if (coord < 0) {
    return max - 1;
  } else if (coord >= max) {
    return 0;
  }
  return coord;
}

// Actual game simulation
void game_simulation(board_t *board, player_status_t * players, int player_c) {
  for (int i = 0; i < player_c; i++) {
    board->spaces[players[i].coordinates.y_position][players[i].coordinates.x_position] = PLAYER_TRAIL;
    
    getNewCoordinates(&players[i]);

    if (board->spaces[players[i].coordinates.y_position][players[i].coordinates.x_position] != EMPTY) {
      printf("Game has ended\n");
      exit(EXIT_SUCCESS);
    }

    board->spaces[players[i].coordinates.y_position][players[i].coordinates.x_position] = PLAYER;
  }
}

player_coordinates_t getStartPosition(board_t * board, int player_n) {
  player_coordinates_t result;
  result.x_position = rand() % BOARD_WIDTH;
  result.y_position = rand() % BOARD_HEIGHT;
  for(int i = 0; i < board->height; i++){
    for(int j = 0; j < board->width; j++){
      if (board->spaces[i][j] == player_n) {
        result.y_position = i;
        result.x_position = j;
        return result;
      }
    }
  }
  return result;
}

direction_t getStartDirection() {
  direction_t start_direction = rand() % 3;
  return start_direction;
}

char * compressGame(game_t * game) {
  // Needed space (assuming max heigh/width 99):
  // - Per player
  //  + X coord 2 chars
  //  + . 1 char
  //  + Y coord 2 chars
  //  + . 1 char
  //  + Dir 1 chars
  //  + . 1 char
  // Equals 8 chars per player
  int player_size = 16;
  char * message = calloc(player_size * game->players->player_count, sizeof(*message));
  char * tmp_message = malloc(player_size * sizeof(*tmp_message));
  for (int i = 0; i < game->players->player_count; i++) {
    sprintf(tmp_message, "%d.%d.%d.", game->stati[i].coordinates.x_position,
      game->stati[i].coordinates.y_position, game->stati[i].current_direction);
    strcat(message, tmp_message);
  }
  free(tmp_message);
  //printf("Compressed: %s\n", message);
  return message;
}

void decompressGame(char * message, game_t * game) {
  int n;
  for (int i = 0; i < game->players->player_count; i++) {
    //printf("\t%s\n",message);
    sscanf(message, "%d.%d.%d.%n", &game->stati[i].coordinates.x_position,
      &game->stati[i].coordinates.y_position, &game->stati[i].current_direction, &n);
    message+=n;
  }
}

void printGame(game_t * game) {
  printf("Game\n\tConnected players: %d\n\tPlayer count: %d\n\tPlayers ready: %d\n",
    game->players->connected_players,
    game->players->player_count,
    game->players->players_ready);
}
