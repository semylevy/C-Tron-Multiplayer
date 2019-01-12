# C-TRON Multiplayer
A C developed, multiplayer adaptation of the original "Tron" arcade game.

## Compilation Instructions
    make

## Running the game
To start server:

    ./server port-number player-count wait-time

player-count is the number of players to expect (game won't start until all players have connected).
wait-time is the speed of the game in ms. Try values anywhere from 10,000 to 100,000.

To start clients:

    ./client server-ip port-number

## How to play
Use the arrow keys to navigate the screen. As you and the other players move, a trail will be left behind. The only rule of the game is: **do not touch any trail**. The first player to touch a trail loses and the game ends.

## Future requests
* Create better end of game
* Allow for more than player to lose (while others keep playing)

## Known bugs
* When a game finishes, client terminals keep Ncurses settings (such as noecho) resulting in invisible cursor and characters. These are just loacl visual settings and don't affect anything outside the window.
    * To fix close and open again terminal window.