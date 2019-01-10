#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
// Sockets libraries
#include <netdb.h>
#include <arpa/inet.h>
// Ncurses for game visualization
#include <ncurses.h>
// Custom libraries
#include "codes.h"
#include "sockets.h"
#include "fatal_error.h"
#include "tron_simulation.h"

#define BUFFER_SIZE 1024

///// FUNCTION DECLARATIONS
void usage(char * program);
void startGame(int connection_fd, game_t * game);
void update(int connection_fd, game_t * game, direction_t move, int time);
// Thread to catch keyboard strokes
void * threadEntry (void * arg);

///// MAIN FUNCTION
int main(int argc, char * argv[]) {
  // Check the correct arguments
  if (argc != 4) {
      usage(argv[0]);
  }

  int * ch = malloc(sizeof(*ch));
  pthread_t tid;

  initscr();
  noecho();
  curs_set(FALSE);
  cbreak();	/* Line buffering disabled. pass on everything */
  keypad(stdscr, TRUE);

  int status = pthread_create(&tid, NULL, &threadEntry, ch);
  if (status) {
    fprintf(stderr, "ERROR: pthread_create %d\n", status);
    exit(EXIT_FAILURE);
  }

  int connection_fd;
  game_t * game;
  direction_t direction = RIGHT;

  // Start the server
  connection_fd = connectSocket(argv[1], argv[2]);
  // Start the game
  game = malloc(sizeof *game);
  startGame(connection_fd, game);

  int counter = 0;
  int max_y = 0, max_x = 0;

  while(1) {
    // Global var `stdscr` is created by the call to `initscr()`
    getmaxyx(stdscr, max_y, max_x);
    if (counter == 100) {
      counter = 0;  
    } else {
      counter++;
    }

    switch(*ch) {
      case KEY_LEFT:
        if (direction != RIGHT) direction = LEFT;
        *ch = 0;
        break;
      case KEY_RIGHT:
        if (direction != LEFT) direction = RIGHT;
        *ch = 0;
        break;
      case KEY_UP:
        if (direction != DOWN) direction = UP;
        *ch = 0;
        break;
      case KEY_DOWN:
        if (direction != UP) direction = DOWN;
        *ch = 0;
        break;
      default:
        break;
    }

    for (int i = 0; i < game->players->player_count; i++) {
      // Get actual coordinates for current window
      int new_y = max_y * game->stati[i].coordinates.y_position / game->board->height;
      int new_x = max_x * game->stati[i].coordinates.x_position / game->board->width;
      mvprintw(new_y, new_x, "o");
    }

    refresh();

    if (counter % 5 == 0) {
      update(connection_fd, game, direction, atoi(argv[3]));
    }
  }
  // Close the socket
  close(connection_fd);
  free(game);
  endwin();
  return EXIT_SUCCESS;
}

///// FUNCTION DEFINITIONS

/*
    Explanation to the user of the parameters required to run the program
*/
void usage(char * program) {
  printf("Usage:\n");
  printf("\t%s {server_address} {port_number} {delay (ms)}\n", program);
  exit(EXIT_FAILURE);
}


void startGame(int connection_fd, game_t * game) {
  char buffer[BUFFER_SIZE];

  // Prepare the message to the server
  sprintf(buffer, "%d", GAME);

  game->players = malloc(sizeof *game->players);

  // SEND
  // Send the request
  sendString(connection_fd, buffer);
  if ( !recvString(connection_fd, buffer, BUFFER_SIZE) ) {
      printf("Server closed the connection\n");
      return;
  }
  game->board = malloc(sizeof(board_t));
  sscanf(buffer, "%d,%d,%d",
    &game->players->player_count,
    &game->board->width,
    &game->board->height);
  // Initialize player stati
  game->stati = malloc(game->players->player_count * sizeof(*game->stati));
  for(int i = 0; i < game->players->player_count; i++) {
    game->stati[i].coordinates.x_position = -1;
    game->stati[i].coordinates.y_position = -1;
  }
}

void update(int connection_fd, game_t * game, direction_t move, int time) {
  char buffer[BUFFER_SIZE];
  usleep(time);
  // Prepare the message to the server
  sprintf(buffer, "%d", move);

  //usleep(time);

  // SEND
  // Send the request
  sendString(connection_fd, buffer);

  // RECV
  // Receive the response
  if ( !recvString(connection_fd, buffer, BUFFER_SIZE) ) {
    printf("Server closed the connection\n");
    return;
  }
  decompressGame(buffer, game);
}

void * threadEntry (void * arg) {
  int * ch = arg;
  while (1) {
    *ch = getch();
  }
  pthread_exit(NULL);
}
