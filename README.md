I chose to do a bitboard approach for generating the moves in preparation for implementing the AI, because I knew I would eventually want to use bitboards to make the AI as fast as possible. 

I stored the board state in multiple uint64_t "bitboards", as there are 64 possible positions on the board. There are 12 bitboards for each of the pieces, so one for only white pawns, one for black pawns and so on. There is one bitboard for all white pieces, one for all black pieces, and one for all pieces, and these bitboards can be easily formed by performing or operations on the 12 piece bitboards.

The bulk of this part was to generate the move/attack list for a board state. For pawns, knights, and kings, their moves and attacks can be precalculated on program startup because they will be the same regardless of where other pieces are.

For bishops, rooks, and queens however, their moves and attacks depend on other pieces on the board that can obstruct their path. For maximum performance, I researched a technique called "Magic Bitboards", a way to precalculate moves for these sliding pieces accounting for all the possible combinations obstructions.

In essence, there is a finite amount of possible combinations of obstructions. For example, let's take a rook on a1. We do not have to consider the furthest moves (h1, a8) because those are guaranteed, obstruction or not. This means there are 12 spots the obstruction can be, thus there are 2^12= 4096 total combinations of obstructions.

With an optimal hashing function, it is possible to store the combinations of the sliding piece moves. This is where the "magic number" comes in, it is a number that is multiplied by the key to avoid collisions. There is a lot of work done to find the best magic number online, and I copied one from Chess Programming's BBC chess engine.

The bulk of the work is found in MoveGenerator.h, where I wrote a lot of helper functions to generate the moves for each piece, to encode these moves into a move list easily accessible by the Chess.cpp and Game.cpp class. 
