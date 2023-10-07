/**
 * pacman.c
 *
 * A simple game of Pacman.
 *
 * Prior to translating this program into MIPS assembly, you may wish to make use
 * of the provided `pacman.simple.c` file. In it, you can replace complex C 
 * constructs like loops with constructs which will be easier to translate
 * into assembly. To help you check that you haven't altered the behaviour of
 * the game, you can run some automated tests using the command
 *     1092 autotest pacman.simple
 * The simplified C version of this code is not marked.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


#define FALSE 0
#define TRUE  1

#define LEFT             0
#define UP               1
#define RIGHT            2
#define DOWN             3
#define TOTAL_DIRECTIONS 4


struct ghost_t {
    int x;
    int y;
    int direction;
};

/* -------------------------------------------------------------------------- */
/*                                     Map                                    */
/* -------------------------------------------------------------------------- */

#define MAP_WIDTH  13
#define MAP_HEIGHT 10
#define MAP_DOTS   53
#define NUM_GHOSTS 3

#define WALL_CHAR   '#'
#define DOT_CHAR    '.'
#define PLAYER_CHAR '@'
#define GHOST_CHAR  'M'
#define EMPTY_CHAR  ' '


char map[MAP_HEIGHT][MAP_WIDTH] = {
    {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#'},
    {'#', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '#'},
    {'#', '.', '#', '#', '#', '#', '#', '#', '#', '#', '#', '.', '#'},
    {'#', '.', '#', ' ', '#', '.', '.', '.', '.', '.', '.', '.', '#'},
    {'#', '.', '#', '#', '#', '#', '#', '.', '#', '#', '#', '.', '#'},
    {'#', '.', '.', '.', '.', '.', '#', '.', '#', '.', '.', '.', '#'},
    {'#', '.', '#', '#', '#', '#', '#', '.', '#', '#', '#', '.', '#'},
    {'#', '.', '#', '.', '#', '.', '.', '.', '#', '.', '.', '.', '#'},
    {'#', '.', '.', '.', '.', '.', '#', '.', '.', '.', '#', '.', '#'},
    {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#'},
};

struct ghost_t ghosts[NUM_GHOSTS] = {
    {
        .x = 3,
        .y = 3,
        .direction = UP,
    },
    {
        .x = 4,
        .y = 5,
        .direction = RIGHT,
    },
    {
        .x = 9,
        .y = 7,
        .direction = LEFT,
    },
};

int player_x = 7;
int player_y = 5;


/* -------------------------------------------------------------------------- */
/*                        Other global variables                              */
/* -------------------------------------------------------------------------- */

// Most of these should be locals, but local stack variables in MIPS aren't taught.

// A copy of the map, used exclusively in print_map
char map_copy[MAP_HEIGHT][MAP_WIDTH];
// The number of valid directions, used exclusively in do_ghost_logic
int valid_directions[TOTAL_DIRECTIONS];
// Number of dots collected, used exclusively in main
int dots_collected = 0;
// These variables must be addressable, used exclusively in get_valid_directions
int x_copy;
int y_copy;

// State for the random number generator, used in the provided code:
// - get_seed
// - random_number
uint32_t lfsr_state;

/* -------------------------------------------------------------------------- */
/*                            Function Declarations                           */
/* -------------------------------------------------------------------------- */

/* -------------------------------- Subset 0 -------------------------------- */
void print_welcome(void);

/* -------------------------------- Subset 1 -------------------------------- */
int main(void);
int get_direction(void);
int play_tick(int *dots_collected);

/* -------------------------------- Subset 2 -------------------------------- */
void copy_map(char dst[MAP_HEIGHT][MAP_WIDTH], char src[MAP_HEIGHT][MAP_WIDTH]);
int get_valid_directions(int x, int y, int dir_array[TOTAL_DIRECTIONS]);
void print_map(void); // print_map is a mix of subset 2 and 3
int try_move(int *x, int *y, int direction);

/* -------------------------------- Subset 3 -------------------------------- */
int check_ghost_collision(void);
int collect_dot_and_check_win(int *dots_collected);
/* void print_map(void); */ // print_map is a mix of subset 2 and 3
void do_ghost_logic(void);

/* -------------------------------- Provided -------------------------------- */
void get_seed(void);
uint32_t random_number(void);

/* -------------------------------------------------------------------------- */
/*                          Function Implementations                          */
/* -------------------------------------------------------------------------- */

// Subset 1:
// - Function calls
// - Loop with function call as condition
// - Load store from global integer
//
// Main function get the seed, prints welcome, and runs the game loop.
int main(void) {
    get_seed();
    print_welcome();

    do {
        print_map();
        // dots_collected is global
        printf("You've collected %d out of %d dots.\n", dots_collected, MAP_DOTS);
    } while (play_tick(&dots_collected));

    return 0;
}

// Subset 0:
// - Simple string and character prints

// Prints the welcome message
void print_welcome(void) {
    printf("Welcome to 1092 Pacman!\n");
    printf("%c = wall\n", WALL_CHAR);
    printf("%c = you\n", PLAYER_CHAR);
    printf("%c = dot\n", DOT_CHAR);
    printf("%c = ghost\n", GHOST_CHAR);
    printf("\nThe objective is to collect all the dots.\n");
    printf("Use WASD to move.\n");
    printf("Ghosts will move every time you move.\nTouching them will end the game.\n");
}



// Subset 2:
// - Nested for loops
// - Load store from 2D arrays
//
// Copies the game map from src to dst.
void copy_map(char dest[MAP_HEIGHT][MAP_WIDTH], char source[MAP_HEIGHT][MAP_WIDTH]) {
    for (int i = 0; i < MAP_HEIGHT; i++) {
        for (int j = 0; j < MAP_WIDTH; j++) {
            dest[i][j] = source[i][j];
        }
    }

}

// Subset 2:
// - Function calls
// - Nested for loops
// - Load store from 2D arrays
//
// Prints the game map, overlaying the player and ghost afterwards.
// The original map is unchanged.
void print_map(void) {
    // map and map_copy are global
    copy_map(map_copy, map);
    map_copy[player_y][player_x] = PLAYER_CHAR;

    // NOTE - You don't need to implement this for loop until subset 3
    // Put the ghosts on the map
    for (int i = 0; i < NUM_GHOSTS; i++) {
        map_copy[ghosts[i].y][ghosts[i].x] = GHOST_CHAR;
    }

    // Print the map
    for (int i = 0; i < MAP_HEIGHT; i++) {
        for (int j = 0; j < MAP_WIDTH; j++) {
            putchar(map_copy[i][j]);
        }
        putchar('\n');
    }
}

// Subset 1:
// - Nested function calls
// - Load store from global int
// - Function call in if condition
//
// Play 1 tick of the game by:
//
// 1. Asking the player which direction to move
// 2. Trying to move the player in that direction
// 3. Check whether the player has collided with a ghost
// 4. Move the ghosts around
// 5. Check for ghost collision again
// 6. Check whether the player has collected all the dots
//
// Ghost collision must be checked twice to handle crossover and overlap cases.
int play_tick(int *dots) {
    try_move(&player_x, &player_y, get_direction());

    if (check_ghost_collision()) {
        return FALSE;
    }

    do_ghost_logic();

    if (check_ghost_collision()) {
        return FALSE;
    }

    // HINT: This return statement below could be turned into
    // simplified C with an if statement that looks something
    // like:
    //     if (collect_dot_and_check_win(dots) == 0) {
    //         ...
    //     } else {
    //         ...
    //     }
    // Carefully consider what should be put in the body of
    // the if and else statements (fill in the ...). Note
    // that the below line doesn't involve the constants
    // TRUE or FALSE (and so its simple C expansion should
    // also not involve TRUE or FALSE).
    return !collect_dot_and_check_win(dots);
}

// Subset 3:
// - Load store from 2D arrays
// - Advanced pointer (de)referencing
//
// Checks whether the player is on top of a dot and returns whther they've won.
// Returns TRUE when every dot has been collected, FALSE otherwise.
int collect_dot_and_check_win(int *dots) {
    // Check whether the player is on top of a dot
    char *map_char = &map[player_y][player_x];
    if (*map_char == DOT_CHAR) {
        // Remove the dot from the map and increment count
        *map_char = EMPTY_CHAR;
        (*dots)++;

        // Check whether the player has collected all the dots
        if (*dots == MAP_DOTS) {
            printf("All dots collected, you won! :D\n");
            return TRUE;
        }
    }

    return FALSE;
}

// Subset 3:
// - Array of structs
// - Function calls in complex if conditions
//
// Iterate through each ghost and move them.
// If they're at a decision point (defined below), generate a new direction for them to move.
void do_ghost_logic(void) {
    // Iterate through the ghosts
    for (int ghost_id = 0; ghost_id < NUM_GHOSTS; ghost_id++) {
        int n_valid_dirs = get_valid_directions(
            ghosts[ghost_id].x,
            ghosts[ghost_id].y,
            valid_directions
        );

        if (n_valid_dirs == 0) {
            // Edge case where a ghost is boxed in and has nowhere to go
            continue;
        }

        // Check whether this ghost has reached a decision point
        //
        // A decision point is when a ghost
        // - can't move forward anymore (e.g. reached a corner or dead end) or
        // - is at an intersection.
        //
        // Once a ghost is at a decision point,
        // it randomly chooses a valid direction to go
        //
        // NOTE - `||` is a 'short-circuiting' operator
        if (
            n_valid_dirs > 2
            // try_move tries to move the ghost if it can
            || !try_move(
                &ghosts[ghost_id].x,
                &ghosts[ghost_id].y,
                ghosts[ghost_id].direction
            )
        ) {
            // The ghost is at a decision point
            // Randomly generate a new direction
            // NOTE - random_number() returns an *unsigned* value
            uint32_t dir_index = random_number() % n_valid_dirs;
            // Update the ghost's current direction
            ghosts[ghost_id].direction = valid_directions[dir_index];

            // Try to move the ghost again with the new direction (should succeed)
            try_move(
                &ghosts[ghost_id].x,
                &ghosts[ghost_id].y,
                ghosts[ghost_id].direction
            );
        }
    }
}

// Subset 3:
// - Array of structs
// - Complex if condition
//
// Returns TRUE if the player has collided with any of the ghosts, FALSE otherwise.
int check_ghost_collision(void) {
    for (int i = 0; i < NUM_GHOSTS; i++) {
        if (player_x == ghosts[i].x && player_y == ghosts[i].y) {
            printf("You ran into a ghost, game over! :(\n");
            return TRUE;
        }
    }

    return FALSE;
}

// Subset 2:
// - Load store 2D array in if condition
// - Simple pointer (de)referencing
//
// Tries to move a coordinate 1 space in the given direction and returns whether it was successful.
//
// If there's a wall in that direction, x and y are unchanged, and FALSE is returned.
// Otherwise, x or y are updated to the new coordinate, and TRUE is returned.
//
// direction:
// LEFT  == 0
// UP    == 1
// RIGHT == 2
// DOWN  == 3
int try_move(int *x, int *y, int direction) {
    int new_x = *x;
    int new_y = *y;

    if (direction == LEFT) {
        new_x--;
    } else if (direction == UP) {
        new_y--;
    } else if (direction == RIGHT) {
        new_x++;
    } else if (direction == DOWN) {
        new_y++;
    }

    if (map[new_y][new_x] == WALL_CHAR) {
        return FALSE;
    }

    *x = new_x;
    *y = new_y;
    return TRUE;
}

// Subset 1:
// - Simple syscall
// - Infinite Loop
// - Simple if condition
// - Early return
// - Continue
//
// Asks the user which direction to move.
// a == LEFT  == 0
// w == UP    == 1
// d == RIGHT == 2
// s == DOWN  == 3
// Returns the direction as an enumerated integer.
//
// You can assume in this function that all user input consists
// of lines containing either a single ASCII character, or empty
// lines. You can assume all lines are terminated by a newline.
// You do not need to handle EOF.
int get_direction(void) {
    printf("Choose next move (wasd): ");
    while (TRUE) {
        // Note that the getchar() call here should be directly
        // translated to a read_character syscall, there is no
        // requirement to handle input special cases such as being
        // given multiple characters in one line of input.
        int input = getchar();

        if (input == 'a') {
            return LEFT;
        }
        if (input == 'w') {
            return UP;
        }
        if (input == 'd') {
            return RIGHT;
        }
        if (input == 's') {
            return DOWN;
        }
        if (input == '\n') {
            continue;
        }

        printf("Invalid input! Use the wasd keys to move.\n");
    }
}

// Subset 2:
// - Load store from global integer
// - Load store from 1D array
// - Function call in if condition
//
// Given a coordinate, check which directions are moveable.
// Returns the number of directions moveable from the given coordinate.
// dir_array will contain the valid directions as their enums.
//
// LEFT  == 0
// UP    == 1
// RIGHT == 2
// DOWN  == 3
//
// E.g. If the function returns:
// - 3, dir_array[0 to 2] will contain valid directions
// - 1, dir_array[0] will be the only valid direction
// - 0, don't use dir_array, since there are no valid directions
int get_valid_directions(int x, int y, int dir_array[TOTAL_DIRECTIONS]) {
    int valid_dirs = 0;

    for (int dir = 0; dir < TOTAL_DIRECTIONS; dir++) {
        x_copy = x;
        y_copy = y;
        if (try_move(&x_copy, &y_copy, dir)) {
            dir_array[valid_dirs] = dir;
            valid_dirs++;
        }
    }

    return valid_dirs;
}

// NOTE - Provided code.
// You do not need to undertsand the operations used in this function.
//
// A 32-bit random number generator.
// Uses a left shifting XOR linear feedback shift register.
// The maximal polynomial 32, 22, 2, 1 is used.
// https://en.wikipedia.org/wiki/Linear-feedback_shift_register
uint32_t random_number(void) {
    // lfsr_state is global
    uint32_t bit = lfsr_state;

    bit ^= lfsr_state >> 10;
    bit ^= lfsr_state >> 30;
    bit ^= lfsr_state >> 31;
    bit &= 0x1u;

    lfsr_state <<= 1;
    lfsr_state |= bit;

    return lfsr_state;
}

// NOTE - Provided code
// Asks the user for the seed and sets it.
// Assume the user will always input an integer.
void get_seed(void) {
    while (TRUE) {
        printf("Enter a non-zero number for the seed: ");
        scanf("%u", &lfsr_state);

        if (lfsr_state != 0) {
            // Seed is valid
            break;
        }

        printf("Seed can't be zero.\n");
    }
}
