#include "Application.h"
#include "imgui/imgui.h"
#include "classes/TicTacToe.h"
#include "classes/Chess.h"
#include "classes/Zobrist.h"
#include "classes/MoveGenerator.h"

namespace ClassGame {
        //
        // our global variables
        //
        Chess *game = nullptr;
        bool gameOver = false;
        int gameWinner = -1;
        Zobrist zobrist;

        uint64_t perft(MoveGenerator* mg, int depth){
            int i;
            uint64_t nodes = 0;

            if(depth == 0)
                return 1ULL;

            mg->generateMoves();
            if(depth == 1)
                return mg->moveCount;

            uint32_t moveList[256];
            mg->copyMoveList(moveList);
            int moveCount = mg->copyMoveCount();


            for(i = 0; i < moveCount; i++){
                // Copy board state
                uint64_t bitboards[12];
                mg->copyBitboards(bitboards);
                uint64_t allBlacks = mg->copyAllBlacks();
                uint64_t allWhites = mg->copyAllWhites();
                int side = mg->copySide();
                int castle = mg->copyCastle();
                int enpassant = mg->copyEnpassant();
                int enpassantPiece = mg->copyEnpassantPiece();
                //std::cout << "Depth " << depth << " making move" << std::endl;
                //mg->printBitboard(allWhites | allBlacks);
                //mg->printBoard();

                mg->makeMove(moveList[i]);
                nodes += perft(mg, depth - 1);

                mg->undoBitboards(bitboards);
                mg->setAllBlacks(allBlacks);
                mg->setAllWhites(allWhites);
                mg->setAllPieces(allBlacks, allWhites);
                mg->setSide(side);
                mg->setCastle(castle);
                mg->setEnpassant(enpassant);
                mg->setEnpassantPiece(enpassantPiece);

                //std::cout << "Depth " << depth << " undo move" << std::endl;
                //mg->printBitboard(allWhites | allBlacks);
                //mg->printBoard();
            }
            return nodes;
        }

        //
        // game starting point
        // this is called by the main render loop in main.cpp
        //
        void GameStartUp()
        {
            game = new Chess();
            game->setUpBoard();
            //std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
            //game->_mg->parseFen(fen.c_str());

            /*
            std::string fen = "rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2";
            game->_mg->parseFen(fen.c_str());
            game->_mg->printBoard();
            game->_mg->generateMoves();

            uint64_t bitboards[12];
            game->_mg->copyBitboards(bitboards);

            for(int i = 0; i < 12; i++){
                game->_mg->printBitboard(bitboards[i]);
            }
            std::cout << "making move" << std::endl;
            game->_mg->makeMove(game->_mg->moveList[0]);

            game->_mg->printBoard();

            std::cout << "undid move" << std::endl;
            game->_mg->undoBitboards(bitboards);

            for(int i = 0; i < 12; i++){
                game->_mg->printBitboard(bitboards[i]);
            }
            game->_mg->printBoard();*/




            //std::cout << "Perft 1: " << perft(game->_mg, 1) << std::endl;
            //std::cout << "Perft 2: " << perft(game->_mg, 2) << std::endl;
            //std::cout << "Perft 3: " << perft(game->_mg, 3) << std::endl;
            //std::cout << "Perft 4: " << perft(game->_mg, 4) << std::endl;
            //std::cout << "Perft 5: " << perft(game->_mg, 5) << std::endl;
        }

        //
        // game render loop
        // this is called by the main render loop in main.cpp
        //
        void RenderGame()
        {
                ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

                ImGui::Begin("Settings");
                ImGui::Text("Current Player Number: %d", game->getCurrentPlayer()->playerNumber());
                ImGui::Text("");
                std::string state = game->stateString();
                //
                // break state string into 8 rows of 8 characters
                //
                if (state.length() == 64) {
                    for (int y=0; y<8; y++) {
                        std::string row = state.substr(y*8, 8);
                        ImGui::Text("%s", row.c_str());
                    }
                    ImGui::Text("");
                    int64_t hash = zobrist.ZobristHash(state.c_str(),64);
                    ImGui::Text("zobrist hash: %llx", hash);
                    ImGui::Text("");
                    ImGui::Text("board evaluation value: %d", game->evaluateBoard(state.c_str()));
                } else {
                        ImGui::Text("%s", state.c_str());
                }
                if (game->gameHasAI()) {
                    ImGui::Text("");
                    ImGui::Text("AI Depth Searches: %d", game->getAIDepathSearches());
                }
                if (gameOver) {
                    ImGui::Text("Game Over!");
                    ImGui::Text("Winner: %d", gameWinner);
                    if (ImGui::Button("Reset Game")) {
                        game->stopGame();
                        game->setUpBoard();
                        gameOver = false;
                        gameWinner = -1;
                    }
                }
                ImGui::End();

                ImGui::Begin("GameWindow");
                game->drawFrame();
                ImGui::End();
        }

        //
        // end turn is called by the game code at the end of each turn
        // this is where we check for a winner
        //
        void EndOfTurn()
        {
            Player *winner = game->checkForWinner();
            if (winner)
            {
                gameOver = true;
                gameWinner = winner->playerNumber();
            }
            if (game->checkForDraw()) {
                gameOver = true;
                gameWinner = -1;
            }
        }
}
