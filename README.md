# Card Game
## Description
This is a card game simulator. Parent process is a server and child processes are players. Players get cards represented by numbers from 1 do M. In each round they choose one random card which cannot be reused later. Player with the highest card wins and get N points. Game ends when players have no cards left. Players communicate with server by pipes.
## Usage
To run this program, use:
```
make
./sample.o N M
```
