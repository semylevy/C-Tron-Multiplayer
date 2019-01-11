/* TRON Multiplayer Server.
 * Implementation for the server functionality using threads and mutexes.
 * 
 * Christian Aguilar
 * Salomon Levy
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
// Signals library
#include <errno.h>
#include <signal.h>
// Sockets libraries
#include <netdb.h>
#include <sys/poll.h>
// Posix threads library
#include <pthread.h>

// Custom libraries
#include "codes.h"
#include "sockets.h"
#include "fatal_error.h"
#include "tron_simulation.h"

#define BUFFER_SIZE 1024
#define MAX_QUEUE 5
#define SLEEP 10000

// use for printing debug info
// #define DEBUG

///// Structure definitions

// Structure for the mutexes to keep the data consistent
typedef struct locks_struct {
  // Mutex array for the operations on the accounts
  pthread_mutex_t count_mutex;
} locks_t;

// Data that will be sent to each structure
typedef struct data_struct {
  // Unique id for each player
  int player_number;
  // The file descriptor for the socket
  int connection_fd;
  // A pointer to a player data structure
  game_t * game_data;
  // A pointer to a locks structure
  locks_t * data_locks;
} thread_data_t;


// Global variable to detect when a signal arrived
int interrupted = 0;

///// FUNCTION DECLARATIONS
void usage(char * program);
void setupHandlers();
void initGame(game_t * game_data, int player_c, locks_t * data_locks);
void waitForConnections(int server_fd, game_t * game_data, locks_t * data_locks);
void * attentionThread(void * arg);
int checkValidAccount(int account);
void closeGame(game_t * game_data, locks_t * data_locks);
void detectInterruption(int signal);
void manageGame(game_t * game_data, locks_t * data_locks);

///// MAIN FUNCTION
int main(int argc, char * argv[]) {
  int server_fd;
  game_t game_data;
  locks_t data_locks;

  printf("\n=== TRON SERVER ===\n");

  // Check the correct arguments
  if (argc != 3) {
    usage(argv[0]);
  }

  // Configure the handler to catch SIGINT
  setupHandlers();

  // Initialize the data structures
  initGame(&game_data, atoi(argv[2]), &data_locks);

	// Show the IPs assigned to this computer
	printLocalIPs();
  // Start the server
  server_fd = initServer(argv[1], MAX_QUEUE);
  // Listen for connections from the clients
  waitForConnections(server_fd, &game_data, &data_locks);
  // All players have connected, start game
  manageGame(&game_data, &data_locks);
  // Close the socket
  close(server_fd);

  // Clean the memory used
  closeGame(&game_data, &data_locks);

  // Finish the main thread
  //pthread_exit(NULL);

  return 0;
}

///// FUNCTION DEFINITIONS

/*
    Explanation to the user of the parameters required to run the program
*/
void usage(char * program) {
  printf("Usage:\n");
  printf("\t%s {port_number} {player_number}\n", program);
  exit(EXIT_FAILURE);
}

/*
    Modify the signal handlers for specific events
*/
void setupHandlers() {
  struct sigaction new_action;

  // Configure the action to take
  // Block all signals during the time the handler funciton is running
  sigfillset(&new_action.sa_mask);
  new_action.sa_handler = detectInterruption;

  // Set the handler
  sigaction(SIGINT, &new_action, NULL);
}

// Signal handler
void detectInterruption(int signal) {
  printf("INTERRUPT\n");
  // Change the global variable
  interrupted = 1;
}

/*
    Function to initialize all the information necessary
    This will allocate memory for the accounts, and for the mutexes
*/
void initGame(game_t * game_data, int player_c, locks_t * data_locks) {
  printf("INIT GAME\n");
  // Game hasn's started
  game_data->status = 0;
  // Initialize board
  game_data->board = create_board(BOARD_WIDTH, BOARD_HEIGHT);
  // Initialize player stati
  game_data->stati = malloc(player_c * sizeof(*game_data->stati));
  // Initialize players
  game_data->players = malloc(sizeof *game_data->players);
  // Initialize connected player count
  game_data->players->connected_players = 0;
  // Initialize players ready count
  game_data->players->players_ready = 0;
  // Set the number of players
  game_data->players->player_count = player_c;
  // Initialize the mutex
  pthread_mutex_init(&data_locks->count_mutex, NULL);
}

/*
    Main loop to wait for incomming connections
*/
void waitForConnections(int server_fd, game_t * game_data, locks_t * data_locks) {
  struct sockaddr_in client_address;
  socklen_t client_address_size;
  char client_presentation[INET_ADDRSTRLEN];
  int client_fd;
  pthread_t new_tid;
  thread_data_t * connection_data = NULL;
  int poll_response;
  int timeout = 500;		// Time in milliseconds (0.5 seconds)
  int current_player_c = 0;

  // Get the size of the structure to store client information
  client_address_size = sizeof client_address;

  while (!interrupted) {
    printf("%d / %d players!\n", current_player_c, game_data->players->player_count);
    if (current_player_c >= game_data->players->player_count) {
      printf("All players have connected, starting game...\n");
      game_data->status = 1;
      break;
    }
    //// POLL
    // Create a structure array to hold the file descriptors to poll
    struct pollfd test_fds[1];
    // Fill in the structure
    test_fds[0].fd = server_fd;
    test_fds[0].events = POLLIN;    // Check for incomming data
    // Check if there is any incomming communication
    poll_response = poll(test_fds, 1, timeout);

    // Error when polling
    if (poll_response == -1) {
      // Test if the error was caused by an interruption
      if (errno == EINTR) {
        //printf("Poll did not finish. The program was interrupted");
        break;
      } else {
        fatalError("ERROR: poll");
      }
    }
    // Timeout finished without reading anything
    else if (poll_response == 0) {
      //printf("No response after %d seconds\n", timeout);
    } else {
      // There is something ready at the socket
      // Check the type of event detected
      if (test_fds[0].revents & POLLIN) {
        // ACCEPT
        // Wait for a client connection
        client_fd = accept(server_fd, (struct sockaddr *)&client_address,
                           &client_address_size);
        if (client_fd == -1) {
          fatalError("ERROR: accept");
        }
      
        // Get the data from the client
        inet_ntop(client_address.sin_family, &client_address.sin_addr,
                  client_presentation, sizeof client_presentation);
        printf("Received incomming connection from %s on port %d\n",
                client_presentation, client_address.sin_port);

        current_player_c++;

        // Update player data
        game_data->players->connected_players = current_player_c;
        game_data->stati[current_player_c - 1].player_number = current_player_c;
        game_data->stati[current_player_c - 1].current_direction = getStartDirection();
        game_data->stati[current_player_c - 1].coordinates = getStartPosition(game_data->board, current_player_c);
        game_data->stati[current_player_c - 1].status = 1;

        // Prepare the structure to send to the thread
        connection_data = malloc(sizeof(*connection_data));
        connection_data->player_number = current_player_c;
        connection_data->connection_fd = client_fd;
        connection_data->game_data = game_data;
        connection_data->data_locks = data_locks;

        // CREATE A THREAD
        int status = pthread_create(&new_tid, NULL, &attentionThread, connection_data);
        if (status) {
          fprintf(stderr, "ERROR: pthread_create %d\n", status);
          exit(EXIT_FAILURE);
        }
      }
    }
  }
}

/*
    Hear the request from the client, wait for every player and start game
*/
void * attentionThread(void * arg) {
  // Receive the client data, mutexes and socket file descriptor
  thread_data_t * connection_data = arg;
  int poll_response;
  int timeout = 10;		// Time in milliseconds
  char buffer[BUFFER_SIZE];
  direction_t direction;
  operation_t op;
  int disconnected = 0;
  char * compressed = NULL;

  recvString(connection_data->connection_fd, buffer, BUFFER_SIZE);
  sscanf(buffer, "%d", &op);
  if (op != GAME) {
    // error
  }
  sprintf(buffer, "%d,%d,%d",
    connection_data->game_data->players->player_count,
    connection_data->game_data->board->width,
    connection_data->game_data->board->width);
  sendString(connection_data->connection_fd, buffer);

  // Wait for every player to connect
  while (!connection_data->game_data->status && !interrupted) {
    usleep(SLEEP);
    printf("Player %d is waiting for all players to connect.\n", connection_data->player_number);
  }

  // Loop to listen for messages from the client
  while (!interrupted && !disconnected) {
    //// POLL
    // Create a structure array to hold the file descriptors to poll
    struct pollfd test_fds[1];
    // Fill in the structure
    test_fds[0].fd = connection_data->connection_fd;
    test_fds[0].events = POLLIN;    // Check for incomming data
    // Check if there is any incomming communication
    poll_response = poll(test_fds, 1, timeout);
    // Error when polling
    if (poll_response == -1) {
      // Test if the error was caused by an interruption
      if (errno == EINTR) {
        printf("Poll did not finish. The program was interrupted");
        break;
      } else {
        fatalError("ERROR: poll");
      }
    }
    // Timeout finished without reading anything
    else if (poll_response == 0) {
      //printf("No response on thread %d\n", connection_data->connection_fd);
    } else {
      // There is something ready at the socket
      // Check the type of event detected
      if (test_fds[0].revents & POLLIN) {
        // Receive the request
        recvString(connection_data->connection_fd, buffer, BUFFER_SIZE);
        // Process the request being careful of data consistency
        sscanf(buffer, "%d", &direction);
        connection_data->game_data->
          stati[connection_data->player_number - 1].current_direction = direction;
        #ifdef DEBUG
          printf("Received %d from player %d\n", direction, connection_data->player_number);
        #endif
        pthread_mutex_lock(&connection_data->data_locks->count_mutex);
          connection_data->game_data->players->players_ready++;
        pthread_mutex_unlock(&connection_data->data_locks->count_mutex);
        // The server sets count to 0 once all players are ready
        while (connection_data->game_data->players->players_ready != 0
               && !interrupted) {
          // Check if there is any incomming communication
          poll_response = poll(test_fds, 1, timeout);
          if (poll_response == 0) {
            #ifdef DEBUG
              printf("%d", connection_data->player_number);
            #endif
            fflush(stdout);
          } else {
            printf("BREAK\n");
            disconnected = 1;
            break;
          }
        }
        compressed = compressGame(connection_data->game_data);
        #ifdef DEBUG
          printf("Sending message %s to %d\n",compressed, connection_data->player_number);
        #endif
        sendString(connection_data->connection_fd, compressed);
        ///printf("After send: %s\n", compressed);
        free(compressed);
      }
    }
  }
  // Let server know the client disconnected
  connection_data->game_data->players->connected_players--;
  // Free memory allocated by parent
  free(connection_data);
  pthread_exit(NULL);
}

void manageGame(game_t * game_data, locks_t * data_locks) {
  while (!interrupted) {
    usleep(1000);
    if (game_data->players->players_ready >= game_data->players->connected_players) {
      game_simulation(game_data->board, game_data->stati, game_data->players->player_count);
      #ifdef DEBUG
        print_board(game_data->board);
      #endif
      pthread_mutex_lock(&data_locks->count_mutex);
        game_data->players->players_ready = 0;
      pthread_mutex_unlock(&data_locks->count_mutex);
    }
    if (game_data->players->connected_players == 0) {
      printf("Game finished\n");
      return;
    }
  }
}

/*
    Free all the memory used for the bank data
*/
void closeGame(game_t * player_data, locks_t * data_locks) {
    #ifdef DEBUG
      printf("DEBUG: Clearing the memory for the thread\n");
    #endif
    free_board(player_data->board);
    free(player_data->stati);
    free(player_data->players);
}
