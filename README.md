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