########################################################################
# DPST1092 23T3 -- Assignment 1 -- Pacman!
#
#
# !!! IMPORTANT !!!
# Before starting work on the assignment, make sure you set your tab-width to 8!
# It is also suggested to indent with tabs only.
# Instructions to configure your text editor can be found here:
#   https://cgi.cse.unsw.edu.au/~dp1092/23T3/resources/mips-editors.html
# !!! IMPORTANT !!!
#
#
# This program was written by YOUR-NAME-HERE (z5555555)
# on INSERT-DATE-HERE. INSERT-SIMPLE-PROGRAM-DESCRIPTION-HERE
#
########################################################################

#![tabsize(8)]

# Constant definitions.
# !!! DO NOT ADD, REMOVE, OR MODIFY ANY OF THESE DEFINITIONS !!!

# Bools
FALSE = 0
TRUE  = 1

# Directions
LEFT  = 0
UP    = 1
RIGHT = 2
DOWN  = 3
TOTAL_DIRECTIONS = 4

# Map
MAP_WIDTH  = 13
MAP_HEIGHT = 10
MAP_DOTS   = 53
NUM_GHOSTS = 3

WALL_CHAR   = '#'
DOT_CHAR    = '.'
PLAYER_CHAR = '@'
GHOST_CHAR  = 'M'
EMPTY_CHAR  = ' '

# Other helpful constants
GHOST_T_X_OFFSET          = 0
GHOST_T_Y_OFFSET          = 4
GHOST_T_DIRECTION_OFFFSET = 8
SIZEOF_GHOST_T            = 12
SIZEOF_INT                = 4

########################################################################
# DATA SEGMENT
# !!! DO NOT ADD, REMOVE, MODIFY OR REORDER ANY OF THESE DEFINITIONS !!!

	.data
map:
	.byte '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#'
	.byte '#', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '#'
	.byte '#', '.', '#', '#', '#', '#', '#', '#', '#', '#', '#', '.', '#'
	.byte '#', '.', '#', ' ', '#', '.', '.', '.', '.', '.', '.', '.', '#'
	.byte '#', '.', '#', '#', '#', '#', '#', '.', '#', '#', '#', '.', '#'
	.byte '#', '.', '.', '.', '.', '.', '#', '.', '#', '.', '.', '.', '#'
	.byte '#', '.', '#', '#', '#', '#', '#', '.', '#', '#', '#', '.', '#'
	.byte '#', '.', '#', '.', '#', '.', '.', '.', '#', '.', '.', '.', '#'
	.byte '#', '.', '.', '.', '.', '.', '#', '.', '.', '.', '#', '.', '#'
	.byte '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#'

	.align 2
ghosts:
	.word 3, 3, UP		# ghosts[0]
	.word 4, 5, RIGHT	# ghosts[1]
	.word 9, 7, LEFT	# ghosts[2]

player_x:
	.word 7
player_y:
	.word 5

map_copy:
	.space	MAP_HEIGHT * MAP_WIDTH

	.align 2
valid_directions:
	.space	4 * TOTAL_DIRECTIONS
dots_collected:
	.word	0
x_copy:
	.word	0
y_copy:
	.word	0

lfsr_state:
	.space	4

# print_welcome strings
welcome_msg:
	.asciiz "Welcome to 1092 Pacman!\n"
welcome_msg_wall:
	.asciiz " = wall\n"
welcome_msg_you:
	.asciiz " = you\n"
welcome_msg_dot:
	.asciiz " = dot\n"
welcome_msg_ghost:
	.asciiz " = ghost\n"
welcome_msg_objective:
	.asciiz "\nThe objective is to collect all the dots.\n"
welcome_msg_wasd:
	.asciiz "Use WASD to move.\n"
welcome_msg_ghost_move:
	.asciiz "Ghosts will move every time you move.\nTouching them will end the game.\n"

# get_direction strings
choose_move_msg:
	.asciiz "Choose next move (wasd): "
invalid_input_msg:
	.asciiz "Invalid input! Use the wasd keys to move.\n"

# main strings
dots_collected_msg_1:
	.asciiz "You've collected "
dots_collected_msg_2:
	.asciiz " out of "
dots_collected_msg_3:
	.asciiz " dots.\n"

# check_ghost_collision strings
game_over_msg:
	.asciiz "You ran into a ghost, game over! :(\n"

# collect_dot_and_check_win strings
game_won_msg:
	.asciiz "All dots collected, you won! :D\n"


# ------------------------------------------------------------------------------
#                                 Text Segment
# ------------------------------------------------------------------------------

	.text

############################################################
####                                                    ####
####   Your journey begins here, intrepid adventurer!   ####
####                                                    ####
############################################################

################################################################################
#
# Implement the following functions,
# and check these boxes as you finish implementing each function.
#
#  SUBSET 0
#  - [ ] print_welcome
#  SUBSET 1
#  - [ ] main
#  - [ ] get_direction
#  - [ ] play_tick
#  SUBSET 2
#  - [ ] copy_map
#  - [ ] get_valid_directions
#  - [ ] print_map
#  - [ ] try_move
#  SUBSET 3
#  - [ ] check_ghost_collision
#  - [ ] collect_dot_and_check_win
#  - [ ] do_ghost_logic
#     (and also the ghosts part of print_map)
#  PROVIDED
#  - [X] get_seed
#  - [X] random_number


################################################################################
# .TEXT <print_welcome>
	.text
print_welcome:
	# Subset:   0
	#
	# Args:     void
	#
	# Returns:  void
	#
	# Frame:    [...]
	# Uses:     [...]
	# Clobbers: [...]
	#
	# Locals:
	#   - ...
	#
	# Structure:
	#   print_welcome
	#   -> [prologue]
	#       -> body
	#   -> [epilogue]

print_welcome__prologue:

print_welcome__body:
    # Print welcome message
    li $v0, 4
    la $a0, welcome_msg
    syscall

    # Print wall message
    li $v0, 4
    la $a0, welcome_msg_wall
    syscall

    # Print player message
    li $v0, 4
    la $a0, welcome_msg_you
    syscall

    # Print dot message
    li $v0, 4
    la $a0, welcome_msg_dot
    syscall

    # Print ghost message
    li $v0, 4
    la $a0, welcome_msg_ghost
    syscall

    # Print objective message
    li $v0, 4
    la $a0, welcome_msg_objective
    syscall

    # Print move message
    li $v0, 4
    la $a0, welcome_msg_wasd
    syscall

    # Print ghost move message
    li $v0, 4
    la $a0, welcome_msg_ghost_move
    syscall

print_welcome__epilogue:

	jr	$ra


################################################################################
# .TEXT <main>
	.text
main:
	# Subset:   1
	#
	# Args:     void
	#
	# Returns:
	#    - $v0: int
	#
	# Frame:    [...]
	# Uses:     [...]
	# Clobbers: [...]
	#
	# Locals:
	#   - ...
	#
	# Structure:
	#   main
	#   -> [prologue]
	#       -> body
	#   -> [epilogue]

main__prologue:
	# TODO: put your prologue for main here
	# jal get_seed

	jal print_welcome
    jal print_map
main__body:
	# TODO: put your body for main here

main__epilogue:
	# TODO: put your epilogue for main here

	jr	$ra


################################################################################
# .TEXT <get_direction>
	.text
get_direction:
	# Subset:   1
	#
	# Args:     void
	#
	# Returns:
	#    - $v0: int
	#
	# Frame:    [...]
	# Uses:     [...]
	# Clobbers: [...]
	#
	# Locals:
	#   - ...
	#
	# Structure:
	#   get_direction
	#   -> [prologue]
	#       -> printmovemsg
	#   -> [loop_body]
	#       -> return_LEFT  
	#       -> return_RIGHT
	#       -> return_DOWN
	#       -> return_UP
	#   -> [epilogue]

get_direction__prologue:
    li $v0, 4
    la $a0, choose_move_msg
    syscall

get_direction__body:
	li $v0, 12        
    syscall
    move $t0, $v0

	li $t1, 'a'
    beq $t0, $t1, return_LEFT
    li $t1, 'w'
    beq $t0, $t1, return_UP
    li $t1, 'd'
    beq $t0, $t1, return_RIGHT
    li $t1, 's'
    beq $t0, $t1, return_DOWN

	# Handle invalid input
    li $v0, 4
    la $a0, invalid_input_msg
    syscall
    j get_direction__body

return_LEFT:
    # Return LEFT (0)
    li $v0, 0
    j get_direction__epilogue

return_UP:
    # Return UP (1)
    li $v0, 1
    j get_direction__epilogue

return_RIGHT:
    # Return RIGHT (2)
    li $v0, 2
    j get_direction__epilogue

return_DOWN:
    # Return DOWN (3)
    li $v0, 3
    j get_direction__epilogue

get_direction__epilogue:
	jr	$ra


################################################################################
# .TEXT section
.text
play_tick:
    # Subset: 1
    #
    # Args:
    #    - $a0: int *dots_collected
    #
    # Returns:
    #    - $v0: int
    #
    # Frame:    [...]
    # Uses:     [...]
    # Clobbers: [...]
    #
    # Locals:
    #   - $t0: int* (player_x)
    #   - $t1: int* (player_y)
    #   - $t2: int (direction)
    #   - $t3: int (result)
    #   - $t4: int (temporary)
    #   - $s0: int* (dots_collected)

    # Save the address of dots_collected in $s0
    move $s0, $a0

    # Load player_x address into $t0
    la $t0, player_x

    # Load player_y address into $t1
    la $t1, player_y

    # Ask the player which direction to move
    jal get_direction

    # Store the result (direction) in $t2
    move $t2, $v0

    # Try to move the player in that direction
    move $a0, $t0         # x coordinate (address)
    move $a1, $t1         # y coordinate (address)
    move $a2, $t2         # direction
    jal try_move

    # Check for ghost collision
    jal check_ghost_collision
    bnez $v0, play_tick__epilogue  # 不等于0就结束

    # Do ghost logic
    jal do_ghost_logic

    # Check for ghost collision again
    jal check_ghost_collision
    bnez $v0, play_tick__epilogue  # 不等于0就结束

    # Check whether the player has collected all the dots
    move $a0, $s0         # Pass the address of dots_collected
    jal collect_dot_and_check_win
    bnez $v0, play_tick__epilogue  # If collect_dot_and_check_win returns FALSE, jump to game_over

    # Return TRUE (continue playing)
    li $v0, TRUE
    j exit

play_tick__epilogue:
    # Handle game over condition
    # Print a message and set $v0 to FALSE
    li $v0, FALSE
    j exit

exit:
    # Return from the function
    jr $ra

################################################################################
# .TEXT <copy_map>
    .text
copy_map:
    # Subset:   2
    #
    # Args:
    #    - $a0: char dst[MAP_HEIGHT][MAP_WIDTH]
    #    - $a1: char src[MAP_HEIGHT][MAP_WIDTH]
    #
    # Returns:  void
    #
    # Frame:    [...]
    # Uses:     $t0 (i), $t1 (j), $t2 (temp)
    # Clobbers: $t0, $t1, $t2
    #
    # Locals:
    #   - ...
    #
    # Structure:
    #   copy_map
    #   -> [prologue]
    #       -> body
    #   -> [epilogue]

copy_map__prologue:
    # Prologue code here (if needed)

copy_map__body:
    # Initialize loop counter i (i = 0)
    li $t0, 0

copy_map_outer_loop:
    bge $t0, MAP_HEIGHT, copy_map__epilogue

    li $t1, 0

copy_map_inner_loop:
    # Check if j >= MAP_WIDTH, if true, exit inner loop
    bge $t1, MAP_WIDTH, copy_map_outer_loop_end

    # Load a character from src[i][j]
    lb $t3, ($a1)             # Load src[i][j] into $t3

    # Store the character in dst[i][j]
    sb $t3, ($a0)             # Store $t3 in dst[i][j]

    # Increment j
    addi $t1, $t1, 1

    # Increment src and dst pointers
    addi $a0, $a0, 1          # Increment dst pointer
    addi $a1, $a1, 1          # Increment src pointer

    j copy_map_inner_loop

copy_map_outer_loop_end:
    # Increment i
    addi $t0, $t0, 1
    j copy_map_outer_loop

copy_map__epilogue:
    # Epilogue code here (if needed)

    jr $ra


################################################################################
# .TEXT <get_valid_directions>
.text
get_valid_directions:
    # Subset:   2
    #
    # Args:
    #    - $a0: int x
    #    - $a1: int y
    #    - $a2: int dir_array[TOTAL_DIRECTIONS]
    #
    # Returns:
    #    - $v0: int
    #
    # Frame:    [...]
    # Uses:     $t0, $t1, $t2, $t3, $t4, $t5
    # Clobbers: $t6, $t7, $t8, $t9, $t10, $t11
    #
    # Locals:
    #   - $t0: int valid_dirs
    #   - $t1: int dir
    #   - $t2: int x_copy
    #   - $t3: int y_copy
    #
    # Structure:
    #   get_valid_directions
    #   -> [prologue]
    #       -> body
    #   -> [epilogue]

get_valid_directions__prologue:
    subu $sp, $sp, 12     # Allocate space for locals on the stack
    sw $ra, 0($sp)       # Save return address
    sw $t0, 4($sp)       # Save $t0 (valid_dirs)
    sw $t1, 8($sp)       # Save $t1 (dir)
    sw $t2, 12($sp)      # Save $t2 (x_copy)
    sw $t3, 16($sp)      # Save $t3 (y_copy)

    li $t0, 0            # valid_dirs to 0
    li $t1, 0            # Initialize dir to 0
    li $t4, TOTAL_DIRECTIONS  # Load TOTAL_DIRECTIONS into $t4

get_valid_directions__body:
    move $t2, $a0         # x to x_copy
    move $t3, $a1         # y to y_copy

    # Call try_move(&x_copy, &y_copy, dir)
    move $a0, $t2         # Load x_copy into $a0
    move $a1, $t3         # Load y_copy into $a1
    move $a2, $t1         # Load dir into $a2
    jal try_move

    beqz $v0, skip_addi_dir  # If try_move returns 0, skip adding dir to dir_array

    # Add dir to dir_array
	mul $t5, $t0, 4
    add $t5, $t5, $a2     # Add index to dir_array base address
    sw $t1, ($t5)         # Store dir in dir_array at the calculated index

    # Increment valid_dirs
    addi $t0, $t0, 1

skip_addi_dir:
    # Increment dir
    addi $t1, $t1, 1

    bne $t1, $t4, get_valid_directions__body

get_valid_directions__epilogue:
    move $v0, $t0         # Move valid_dirs to $v0
    lw $ra, 0($sp)        # Restore return address
    lw $t0, 4($sp)        # Restore $t0 (valid_dirs)
    lw $t1, 8($sp)        # Restore $t1 (dir)
    lw $t2, 12($sp)       # Restore $t2 (x_copy)
    lw $t3, 16($sp)       # Restore $t3 (y_copy)
    addu $sp, $sp, 12     # Deallocate space for locals
    jr $ra                # Return


################################################################################
# .TEXT <print_map>
.text
print_map:
    # Subset: 2
    #
    # Args: void
    #
    # Returns: void
    #
    # Frame: [...]
    # Uses: $t0, $t1, $t2, $t3, $t4, $t5, $t6, $t7, $s0, $s1, $s2, $s3, $s4
    # Clobbers: $a0, $a1, $a2, $v0
    #
    # Locals:
    #   - ...
    push $ra
    la $a0, map_copy
    la $a1, map
    jal copy_map

    # Mark the player's position
    lw $t2, player_x      # Load player_x into $t2
    lw $t3, player_y      # Load player_y into $t3
    mul $t4, $t3, MAP_WIDTH # Calculate row offset
    add $t4, $t4, $t2     # Calculate offset for player's position
    la $t5, map_copy      # Load the base address of map_copy
    add $t5, $t5, $t4     # Calculate address of player's position in map_copy
    li $t6, PLAYER_CHAR   # Load PLAYER_CHAR into $t6
    sb $t6, 0($t5)        # Mark player's position in map_copy

    # NOTE: We don't need to implement the loop for placing ghosts (Subset 3).
    # Initialize loop counter
    li $t0, 0            # $t0 = i

    # Loop through ghosts
for_loop:
    # Check if i >= NUM_GHOSTS, if so, exit the loop
    bge $t0, NUM_GHOSTS, end_for_loop

    # Load ghosts[i] address into $t1
    la $t1, ghosts      # $t1 = address of ghosts
    mul $t7, $t0, 12    # Each ghost entry is 12 bytes (3 words)
    add $t1, $t1, $t7   # $t1 = address of ghosts[i]

    # Load ghosts[i].x into $t2
    lw $t2, 0($t1)      # $t2 = ghosts[i].x

    # Load ghosts[i].y into $t3
    lw $t3, 4($t1)      # $t3 = ghosts[i].y

    # Calculate map_copy index
    mul $t4, $t3, MAP_WIDTH  # $t4 = row offset
    add $t4, $t4, $t2       # $t4 = offset for map_copy[row][col]

    # Load GHOST_CHAR into $t5
    li $t5, GHOST_CHAR

    # Store GHOST_CHAR in map_copy[row][col]
    la $t6, map_copy    # $t6 = address of map_copy
    add $t6, $t6, $t4   # $t6 = address of map_copy[row][col]
    sb $t5, 0($t6)

    # Increment loop counter (i)
    addi $t0, $t0, 1

    # Repeat the loop
    j for_loop

end_for_loop:


    # Print the map
    li $t0, 0             # $t0 = 0 (row index)
outer_loop:
    bge $t0, MAP_HEIGHT, print_map__done  # Exit the outer loop if row >= MAP_HEIGHT

    li $t1, 0             # $t1 = 0 (column index)
inner_loop:
    bge $t1, MAP_WIDTH, inner_loop__done  # Exit the inner loop if col >= MAP_WIDTH

    # Load map_copy[i][j] into $t7
    mul $t2, $t0, MAP_WIDTH   # Calculate row offset
    add $t2, $t2, $t1         # Calculate offset for map_copy[i][j]
    la $t5, map_copy              # Load the base address of map_copy
    add $t5, $t5, $t2         # Calculate address of map_copy[i][j]
    lb $t7, 0($t5)            # Load map_copy[i][j] into $t7

    # Print map_copy[i][j]
    move $a0, $t7             # Pass map_copy[i][j] to print_char
    li $v0, 11                 # Set syscall code for printing a character
    syscall

    # Increment column index
    addi $t1, $t1, 1
    j inner_loop

inner_loop__done:
    # Print a newline character
    li $a0, '\n'
    li $v0, 11                  # Set syscall code for printing a character
    syscall

    # Increment row index
    addi $t0, $t0, 1
    j outer_loop

print_map__done:
    pop $ra
    jr $ra



################################################################################
# .TEXT <try_move>
    .text
try_move:
    # Subset:   2
    #
    # Args:
    #    - $a0: int *x
    #    - $a1: int *y
    #    - $a2: int direction
    #
    # Returns:
    #    - $v0: int
    #
    # Frame:    [...]
    # Uses:     $t0 (new_x), $t1 (new_y), $t2 (temp), $t3 (map_char)
    # Clobbers: $t0, $t1, $t2, $t3
    #
    # Locals:
    #   - ...
    #
    # Structure:
    #   try_move
    #   -> [prologue]
    #       -> body
    #   -> [epilogue]

try_move__prologue:
    # Prologue code here (if needed)

    # t0:*x
    lw $t0, 0($a0)

    # $t1:*y
    lw $t1, 0($a1)

    # t2:direction
    move $t2, $a2

try_move__body:
    beq $t2, LEFT, try_move_left
    beq $t2, UP, try_move_up
    beq $t2, RIGHT, try_move_right
    beq $t2, DOWN, try_move_down

try_move_left:
    sub $t0, $t0, 1
    j end_judge

try_move_up:
    sub $t1, $t1, 1
    j end_judge

try_move_right:
    add $t0, $t0, 1
    j end_judge

try_move_down:
    add $t1, $t1, 1

end_judge:
    # Calculate map index based on new_x and new_y
	# 1 0
    li $t3, MAP_WIDTH
	# t3 = 13 * 0 = 0
    mul $t3, $t3, $t1
	# t3 = 0
    add $t3, $t3, $t0

    # Load map_char from map into $t3
    lb $t3, map($t3)

    # Compare map_char with WALL_CHAR
    beq $t3, WALL_CHAR, wall_detected

    sw $t0, 0($a0)
    sw $t1, 0($a1)

    li $v0, 1
    j try_move__epilogue

wall_detected:
	# return FALSE
    li $v0, 0

try_move__epilogue:
    # Epilogue code here (if needed)

    jr $ra



################################################################################
# .TEXT <check_ghost_collision>
.text
check_ghost_collision:
    # Subset: 3
    #
    # Args:   void
    # Returns:
    #   - $v0: int
    #
    # Frame:  [...]
    # Uses:   [...]
    # Clobbers: [...]
    #
    # Locals:
    #   - ...
    #
    # Structure:
    #   check_ghost_collision
    #   -> [prologue]
    #       -> body
    #   -> [epilogue]

check_ghost_collision__prologue:
    # Prologue code (if needed)

check_ghost_collision__body:
    li $t0, NUM_GHOSTS
    li $t1, 0	# t1 : i

loop_check_ghosts:
    bge $t1, $t0, no_ghost_collision

    mul $t2, $t1, 12   # 12 bytes (3 words)

    lw $t3, player_x
    lw $t4, ghosts($t2)
    lw $t5, player_y
	# lw $t6, ghosts($t2+4)

    beq $t3, $t4, x_collision
    beq $t5, $t6, y_collision

    # If no collision, increment the loop counter and repeat
    addi $t1, $t1, 1
    j loop_check_ghosts

x_collision:
    beq $t5, $t6, ghost_collision
    j no_ghost_collision

y_collision:
    # Player_y and ghosts[i].y are equal, it's a collision
    j ghost_collision

ghost_collision:
    # Ghost collision, print message and set $v0 to TRUE
    li $v0, 4     
    la $a0, game_over_msg
    syscall

    # Exit the function
    jr $ra

no_ghost_collision:
    # No ghost collision, set $v0 to FALSE
    li $v0, 0           # 0 indicates FALSE (no collision)

check_ghost_collision__epilogue:
    # Epilogue code (if needed)
    jr $ra



################################################################################
# .TEXT <collect_dot_and_check_win>
.text
collect_dot_and_check_win:
    # Subset:   3
    #
    # Args:
    #    - $a0: int *dots_collected
    #
    # Returns:
    #    - $v0: int
    #
    # Frame:    [...]
    # Uses:     $t0, $t1, $t2, $t3
    # Clobbers: [...]
    #
    # Locals:
    #   - ...
    #
    # Structure:
    #   collect_dot_and_check_win
    #   -> [prologue]
    #       -> body
    #   -> [epilogue]

collect_dot_and_check_win__prologue:
    # Save callee-saved registers
    addi $sp, $sp, -4
    sw $t0, 0($sp)
    addi $sp, $sp, -4
    sw $t1, 0($sp)
    addi $sp, $sp, -4
    sw $t2, 0($sp)
    addi $sp, $sp, -4
    sw $t3, 0($sp)

collect_dot_and_check_win__body:
    # Check whether the player is on top of a dot
    # Load the address of map[player_y][player_x] into $t0
    li $t1, MAP_WIDTH         # Set $t1 = MAP_WIDTH
    mul $t2, $t1, $s1         # Calculate offset for row (player_y * MAP_WIDTH)
    add $t0, $s0, $t2         # $t0 = &map[player_y][0]
    add $t0, $t0, $s2         # $t0 = &map[player_y][player_x]
    lb $t3, 0($t0)           # Load map_char from map into $t3

    # Check if the player is on a dot
    li $t1, DOT_CHAR
    beq $t3, $t1, collect_dot_and_check_win__collect_dot

    # If not on a dot, return FALSE
    li $v0, FALSE
    j collect_dot_and_check_win__epilogue

collect_dot_and_check_win__collect_dot:
    # Remove the dot from the map and increment count
    li $t1, EMPTY_CHAR
    sb $t1, 0($t0)           # Set map[player_y][player_x] = EMPTY_CHAR

    # Increment the dots_collected
    lw $t1, 0($a0)
    addi $t1, $t1, 1
    sw $t1, 0($a0)

    # Check if all dots are collected
    li $t2, MAP_DOTS
    beq $t1, $t2, collect_dot_and_check_win__win

    # If not all dots are collected, return FALSE
    li $v0, FALSE
    j collect_dot_and_check_win__epilogue

collect_dot_and_check_win__win:
    # Print win message and return TRUE
    li $v0, TRUE
    la $a0, game_won_msg
    li $v0, 4
    syscall

collect_dot_and_check_win__epilogue:
    # Restore callee-saved registers
    lw $t3, 0($sp)
    addi $sp, $sp, 4
    lw $t2, 0($sp)
    addi $sp, $sp, 4
    lw $t1, 0($sp)
    addi $sp, $sp, 4
    lw $t0, 0($sp)
    addi $sp, $sp, 4
    jr $ra



################################################################################
# .TEXT <do_ghost_logic>
	.text
do_ghost_logic:
	# Subset:   3
	#
	# Args:     void
	#
	# Returns:  void
	#
	# Frame:    [...]
	# Uses:     $t0, $t1, $t2, $t3, $t4, $t5, $t6, $s0, $s1, $s2, $s3
	# Clobbers: $t0, $t1, $t2, $t3, $t4, $t5, $t6, $s0, $s1, $s2, $s3
	#
	# Locals:
	#   - ghost_id ($t0)
	#   - n_valid_dirs ($t1)
	#   - dir_index ($t2)
	#   - temp_x ($t3)
	#   - temp_y ($t4)
	#   - decision_point ($t5)
	#   - rand_num ($t6)
	#   - tmp ($t7)
	#
	# Structure:
	#   do_ghost_logic
	#   -> [prologue]
	#       -> body
	#   -> [epilogue]

do_ghost_logic__prologue:
	# Save registers that need to be preserved
	addi $sp, $sp, -12   # Allocate space on the stack
	sw $t0, 0($sp)       # Save $t0
	sw $t1, 4($sp)       # Save $t1
	sw $t2, 8($sp)       # Save $t2

	li $t0, 0
	la $s2, valid_directions
	la $s3, ghosts

do_ghost_logic__body:
	# Check if ghost_id >= NUM_GHOSTS
	bge $t0, NUM_GHOSTS, do_ghost_logic__epilogue

	# Load ghosts[ghost_id].x into temp_x
	lw $t3, 0($s3)  # $s3 points to ghosts[ghost_id].x

	# Load ghosts[ghost_id].y into temp_y
	lw $t4, 4($s3)  # Offset 4 bytes to access y coordinate

	# Get valid directions and store the count in n_valid_dirs
	move $a0, $t3         # x coordinate
	move $a1, $t4         # y coordinate
    move $a2, $s2         # dir_array (valid_directions)
	jal get_valid_directions
	move $t1, $v0         # n_valid_dirs = return value

	# Check if n_valid_dirs == 0
	beqz $t1, continue_loop

	# Calculate decision point condition
	li $t5, 2             # Decision point threshold
	bgt $t1, $t5, not_decision_point

	# Check if the ghost can move in its current direction
	move $a0, $t3         # x coordinate
	move $a0, $s3         # x coordinate
	move $a1, $t4         # y coordinate
	# move $a1, $s3+4         # y coordinate
	move $a2, $t2         # direction
	jal try_move
	beqz $v0, not_decision_point

	# Ghost is not at a decision point
	b continue_loop

not_decision_point:
	# Generate a random number (rand_num = random_number() % n_valid_dirs)
	move $a0, $t1         # n_valid_dirs
	jal random_number
	move $t6, $v0

	# Calculate dir_index (dir_index = rand_num % n_valid_dirs)
	move $t7, $t1         # n_valid_dirs
	remu $t2, $t6, $t7    # dir_index = rand_num % n_valid_dirs

	# Load valid_directions[dir_index] into ghosts[ghost_id].direction
	sll $t2, $t2, 2
    add $t2, $t2, $s2     # Address of valid_directions[dir_index]
	lw $t7, 0($t2)        # Load new valid direction
    addi $s3, $s3, 12      # Move s3 to the next ghosts element (each element is 12 bytes)
	sw $t7, 8($s3)        # Store it in ghosts[ghost_id].direction

	# Try to move the ghost with the new direction
	move $a0, $t3         # x coordinate
	move $a0, $s3         # x coordinate
	move $a1, $t4         # y coordinate
    # move $a1, $s3+4         # y coordinate
	move $a2, $t7         # new ghosts[ghost_id].direction
	jal try_move

continue_loop:
	# Increment ghost_id
	addi $t0, $t0, 1
	# Repeat the loop
	j do_ghost_logic__body

do_ghost_logic__epilogue:
	# Restore registers
	lw $t0, 0($sp)       # Restore $t0
	lw $t1, 4($sp)       # Restore $t1
	lw $t2, 8($sp)       # Restore $t2
	addi $sp, $sp, 12    # Deallocate space on the stack

	# Return from function
	jr $ra


################################################################################
################################################################################
###                   PROVIDED FUNCTIONS — DO NOT CHANGE                     ###
################################################################################
################################################################################

	.data
get_seed_prompt_msg:
	.asciiz "Enter a non-zero number for the seed: "
invalid_seed_msg:
	.asciiz "Seed can't be zero.\n"

################################################################################
# .TEXT <get_seed>
	.text
get_seed:
	# Args:     void
	#
	# Returns:  void
	#
	# Frame:    [$ra]
	# Uses:     [$v0, $a0]
	# Clobbers: [$v0, $a0]
	#
	# Locals:
	#   - $v0: copy of lfsr_state
	#
	# Structure:
	#   get_seed
	#   -> [prologue]
	#       -> body
	#       -> loop_start
	#       -> loop_end
	#   -> [epilogue]
	#
	# PROVIDED FUNCTION — DO NOT CHANGE

get_seed__prologue:
	begin
	push	$ra

get_seed__body:
get_seed__loop:					# while (TRUE) {
	li	$v0, 4				#   syscall 4: print_string
	la	$a0, get_seed_prompt_msg
	syscall					#   printf("Enter a non-zero number for the seed: ");

	li	$v0, 5				#   syscall 5: read_int
	syscall
	sw	$v0, lfsr_state			#   scanf("%u", &lfsr_state);

	bnez	$v0, get_seed__loop_end		#   if (lfsr_state != 0) break;

	li	$v0, 4				#   syscall 4: print_string
	la	$a0, invalid_seed_msg
	syscall					#   printf("Seed can't be zero.\n");

	b	get_seed__loop			# }

get_seed__loop_end:
get_seed__epilogue:
	pop	$ra
	end

	jr	$ra


################################################################################
# .TEXT <random_number>
	.text
random_number:
	# Args:     void
	#
	# Returns:
	#    - $v0: uint32_t
	#
	# Frame:    [$ra]
	# Uses:     [$t0, $t1, $t2, $v0]
	# Clobbers: [$t0, $t1, $t2, $v0]
	#
	# Locals:
	#   - $t0: uint32_t bit
	#   - $t1: copy of lfsr_state
	#   - $t2: temporary shift result
	#
	# Structure:
	#   random_number
	#   -> [prologue]
	#       -> body
	#   -> [epilogue]
	#
	# PROVIDED FUNCTION — DO NOT CHANGE

random_number__prologue:
	begin
	push	$ra

random_number__body:
	lw	$t1, lfsr_state		# load lfsr_state
	move	$t0, $t1		# uint32_t bit = lfsr_state;

	srl	$t2, $t1, 10		# lfsr_state >> 10
	xor	$t0, $t0, $t2		# bit ^= lfsr_state >> 10;

	srl	$t2, $t1, 30		# lfsr_state >> 30
	xor	$t0, $t0, $t2		# bit ^= lfsr_state >> 30;

	srl	$t2, $t1, 31		# lfsr_state >> 31
	xor	$t0, $t0, $t2		# bit ^= lfsr_state >> 31;

	andi	$t0, $t0, 1		# bit &= 0x1u;

	sll	$t1, $t1, 1		# lfsr_state <<= 1;
	or	$t1, $t1, $t0		# lfsr_state |= bit;

	sw	$t1, lfsr_state		# store lfsr_state
	move	$v0, $t1		# return lfsr_state;

random_number__epilogue:
	pop	$ra
	end

	jr	$ra
