#pragma once
#include "Game.h"
#include "ChessSquare.h"

//
// the classic game of chess
//
enum ChessPiece {
    Pawn = 1,
    Knight,
    Bishop,
    Rook,
    Queen,
    King
};

class ChessAI
{
public:
    void setBoard();
    int evaluateBoard();
    int negamax(int alpha, int beta, int depth);
    int AICheckForWinner();

    MoveGenerator* _mg;
};
//
// the main game class
//
class Chess : public Game
{
public:
    Chess();
    ~Chess();

    // set up the board
    void        setUpBoard() override;

    Player*     checkForWinner() override;
    bool        checkForDraw() override;
    std::string initialStateString() override;
    std::string stateString() const override;
    void        setStateString(const std::string &s) override;
    bool        actionForEmptyHolder(BitHolder &holder) override {return false; }
    bool        canBitMoveFrom(Bit& bit, BitHolder& src) override;
    bool        canBitMoveFromTo(Bit& bit, BitHolder& src, BitHolder& dst) override;
    void        bitMovedFromTo(Bit& bit, BitHolder& src, BitHolder& dst) override;
    bool	    clickedBit(Bit& bit) override;
    void        stopGame() override;
    int         evaluateBoard(const char* stateString);
    int         rowColToPos(int row, int col);
    void        movePiece(int move);

    BitHolder &getHolderAt(const int x, const int y) override { return _grid[y][x]; }

    //ai functions
    ChessAI* clone();
    void updateAI() override;

private:
    const char  bitToPieceNotation(int row, int column) const;
    std::string indexToNotation(int row, int col);

    Bit *       PieceForPlayer(const int playerNumber, ChessPiece piece);
    ChessSquare         _grid[8][8];

    uint64_t    board;
    uint64_t    w_board;
    uint64_t    b_board;


};
