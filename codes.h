#ifndef CODES_H
#define CODES_H

// The different types of operations available
typedef enum valid_operations {START, END, UPDATE, GAME} operation_t;

// The types of responses available
//typedef enum valid_responses {OK, READY, ERROR, BYE} response_t;

typedef enum space_state{EMPTY, PLAYER_TRAIL, PLAYER} space_state_t;

typedef enum current_direction{UP, RIGHT, DOWN, LEFT} direction_t;

#endif
