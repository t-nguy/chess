#include "Chess.h"
#include <iostream>
#include "MoveGenerator.h"

//
// add a helper to Square so it returns out FEN chess notation in the form p for white pawn, K for black king, etc.
// this version is used from the top level board to record moves
//
//
//
const char Chess::bitToPieceNotation(int row, int column) const
{
    if (row < 0 || row >= 8 || column < 0 || column >= 8) {
        return '0';
    }

    const char *bpieces = { "?PNBRQK" };
    const char *wpieces = { "?pnbrqk" };
    unsigned char notation = '0';
    Bit *bit = _grid[row][column].bit();
    if (bit) {
        notation = bit->gameTag() < 128 ? wpieces[bit->gameTag()] : bpieces[bit->gameTag()&127];
    } else {
        notation = '0';
    }
    return notation;
}

// Convert row and column index to chess notation
std::string Chess::indexToNotation(int row, int col)
{
    return std::string(1, 'a' + col) + std::string(1, '8' - row);
}

Chess::Chess()
{

}

Chess::~Chess()
{
}

//
// make a chess piece for a player
//
Bit* Chess::PieceForPlayer(const int playerNumber, ChessPiece piece)
{
    const char *pieces[] = { "pawn.png", "knight.png", "bishop.png", "rook.png", "queen.png", "king.png" };

    // depending on playerNumber load the "x.png" or the "o.png" graphic
    Bit *bit = new Bit();
    // should possibly be cached from player class?
    const char *pieceName = pieces[piece - 1];
    std::string spritePath = std::string("chess/") + (playerNumber == 0 ? "b_" : "w_") + pieceName;
    bit->LoadTextureFromFile(spritePath.c_str());
    bit->setOwner(getPlayerAt(playerNumber));
    return bit;
}

void Chess::setUpBoard()
{
    const ChessPiece initialBoard[2][8] = {
        {Rook, Knight, Bishop, Queen, King, Bishop, Knight, Rook},  // 1st Rank
        {Pawn, Pawn, Pawn, Pawn, Pawn, Pawn, Pawn, Pawn}  // 2nd Rank
    };

    setNumberOfPlayers(2);
    _gameOptions.rowX = 8;
    _gameOptions.rowY = 8;
    for (int y=0; y<_gameOptions.rowY; y++) {
        for (int x=0; x<_gameOptions.rowX; x++) {
            _grid[y][x].initHolder(ImVec2((float)(100*x + 100),(float)(100*y + 100)), "boardsquare.png", x, y);
            _grid[y][x].setGameTag(0);
            _grid[y][x].setNotation( indexToNotation(y, x) );
        }
    }

    for (int rank=0; rank<2; rank++) {
        for (int x=0; x<_gameOptions.rowX; x++) {
            ChessPiece piece = initialBoard[rank][x];
            Bit *bit = PieceForPlayer(0, initialBoard[rank][x]);
            bit->setPosition(_grid[rank][x].getPosition());
            bit->setParent(&_grid[rank][x]);
            bit->setGameTag( piece + 128);
            _grid[rank][x].setBit( bit );

            bit = PieceForPlayer(1, initialBoard[rank][x]);
            bit->setPosition(_grid[7-rank][x].getPosition());
            bit->setParent(&_grid[7-rank][x]);
            bit->setGameTag( piece );
            _grid[7-rank][x].setBit( bit );
        }
    }

    if (gameHasAI()) {
        setAIPlayer(AI_PLAYER);
    }

    // Set bitboard
    std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    _mg->parseFen(fen.c_str());
    _mg->generateMoves();

    startGame();
}

bool Chess::canBitMoveFrom(Bit& bit, BitHolder& src)
{
    return true;
}

bool Chess::clickedBit(Bit& bit)
{
    return true;
}

// Collision semantics are taken care of in ChessSquare::canDropBitAtPoint
// This function checks if the move is valid for the piece

bool Chess::canBitMoveFromTo(Bit& bit, BitHolder& src, BitHolder& dst)
{
    ChessSquare &srcSquare = static_cast<ChessSquare&>(src);
    int srcPos = rowColToPos(srcSquare.getRow(), srcSquare.getColumn());

    ChessSquare &dstSquare = static_cast<ChessSquare&>(dst);
    int dstPos = rowColToPos(dstSquare.getRow(), dstSquare.getColumn());

	for(int i = 0; i < _mg->moveCount; i++){
		if(srcPos == _mg->getSrc(_mg->moveList[i]) && dstPos == _mg->getDst(_mg->moveList[i])){
            return true;
		}
	}

    return false;
}

// We need:
// Own Pieces
// Opponent Pieces
// All Pieces

int Chess::rowColToPos(int row, int col){
    return row * 8 + col;
}

// Update chess board
void Chess::bitMovedFromTo(Bit& bit, BitHolder& src, BitHolder& dst)
{
    ChessSquare &srcSquare = static_cast<ChessSquare&>(src);
    int srcPos = rowColToPos(srcSquare.getRow(), srcSquare.getColumn());

    ChessSquare &dstSquare = static_cast<ChessSquare&>(dst);
    int dstPos = rowColToPos(dstSquare.getRow(), dstSquare.getColumn());

	for(int i = 0; i < _mg->moveCount; i++){
        int move = _mg->moveList[i];
		if(srcPos == _mg->getSrc(move) && dstPos == _mg->getDst(move)){
            if(_mg->getPromotedPiece(move)){
                Bit *bit = PieceForPlayer(!_mg->side, Queen);
                bit->setPosition(_grid[dstSquare.getRow()][dstSquare.getColumn()].getPosition());
                bit->setParent(&_grid[dstSquare.getRow()][dstSquare.getColumn()]);
                bit->setGameTag(!_mg->side ? Queen : Queen + 128);
                _grid[dstSquare.getRow()][dstSquare.getColumn()].setBit( bit );
            }
            if(_mg->enpassant == dstPos){
                _grid[_mg->enpassantPiece / 8][_mg->enpassantPiece % 8].setBit(nullptr);
            }
            if(_mg->getCastle(move)){
                int dst = _mg->getDst(move);
                // White kingside
                if(dst == _mg->g1){
                    // king to g1
                    // rook from h1 to f1
                    //_grid[_mg->f1 / 8][_mg->f1 % 8].setBit(_grid[_mg->h1 / 8][_mg->h1 % 8].bit());
                    ChessSquare &rookDst= static_cast<ChessSquare&>(_grid[_mg->f1 / 8][_mg->f1 % 8]);
                    Bit* dragBitRook = _grid[_mg->h1 / 8][_mg->h1 % 8].bit();
                    rookDst.dropBitAtPoint(dragBitRook, dragBitRook->getPosition());
                    _grid[_mg->h1 / 8][_mg->h1 % 8].setBit(nullptr);
                }
                // White queenside
                else if(dst == _mg->c1){
                    // king to c1
                    // rook from a1 to d1
                    ChessSquare &rookDst= static_cast<ChessSquare&>(_grid[_mg->d1 / 8][_mg->d1 % 8]);
                    Bit* dragBitRook = _grid[_mg->a1 / 8][_mg->a1 % 8].bit();
                    rookDst.dropBitAtPoint(dragBitRook, dragBitRook->getPosition());
                    _grid[_mg->a1 / 8][_mg->a1 % 8].setBit(nullptr);
                }
                // Black kingside
                else if(dst == _mg->g8){
                    // king to g8
                    // rook from h8 to f8
                    ChessSquare &rookDst= static_cast<ChessSquare&>(_grid[_mg->f8 / 8][_mg->f8 % 8]);
                    Bit* dragBitRook = _grid[_mg->h8 / 8][_mg->h8 % 8].bit();
                    rookDst.dropBitAtPoint(dragBitRook, dragBitRook->getPosition());
                    _grid[_mg->h8 / 8][_mg->h8 % 8].setBit(nullptr);
                }
                // Black queenside
                else if(dst == _mg->c8){
                    // king to c8
                    // rook from a8 to d8
                    ChessSquare &rookDst= static_cast<ChessSquare&>(_grid[_mg->d8 / 8][_mg->d8 % 8]);
                    Bit* dragBitRook = _grid[_mg->a8 / 8][_mg->a8 % 8].bit();
                    rookDst.dropBitAtPoint(dragBitRook, dragBitRook->getPosition());
                    _grid[_mg->a8 / 8][_mg->a8 % 8].setBit(nullptr);
                }
            }

            _mg->makeMove(_mg->moveList[i]);
		}
	}
    _mg->generateMoves();
    endTurn();
}

// move piece function for the ai
void Chess::movePiece(int move){
    uint64_t src = _mg->getSrc(move);
    uint64_t dst = _mg->getDst(move);

    ChessSquare &srcSquare = static_cast<ChessSquare&>(_grid[src / 8][src % 8]);
    ChessSquare &dstSquare = static_cast<ChessSquare&>(_grid[dst / 8][dst % 8]);

    BitHolder &dstHolder = static_cast<BitHolder&>(_grid[dst / 8][dst % 8]);

    Bit* bit = srcSquare.bit();
    dstSquare.dropBitAtPoint(bit, dstSquare.getPosition());
    srcSquare.draggedBitTo(bit, &dstHolder);

    // check special cases

    if(_mg->getPromotedPiece(move)){
        Bit *bit = PieceForPlayer(!_mg->side, Queen);
        bit->setPosition(_grid[dstSquare.getRow()][dstSquare.getColumn()].getPosition());
        bit->setParent(&_grid[dstSquare.getRow()][dstSquare.getColumn()]);
        bit->setGameTag(!_mg->side ? Queen : Queen + 128);
        _grid[dstSquare.getRow()][dstSquare.getColumn()].setBit( bit );
    }
    if(_mg->enpassant == dst){
        _grid[_mg->enpassantPiece / 8][_mg->enpassantPiece % 8].setBit(nullptr);
    }
    if(_mg->getCastle(move)){
        // White kingside
        if(dst == _mg->g1){
            // king to g1
            // rook from h1 to f1
            ChessSquare &rookDst= static_cast<ChessSquare&>(_grid[_mg->f1 / 8][_mg->f1 % 8]);
            Bit* dragBitRook = _grid[_mg->h1 / 8][_mg->h1 % 8].bit();
            rookDst.dropBitAtPoint(dragBitRook, dragBitRook->getPosition());
            _grid[_mg->h1 / 8][_mg->h1 % 8].setBit(nullptr);
        }
        // White queenside
        else if(dst == _mg->c1){
            // king to c1
            // rook from a1 to d1
            ChessSquare &rookDst= static_cast<ChessSquare&>(_grid[_mg->d1 / 8][_mg->d1 % 8]);
            Bit* dragBitRook = _grid[_mg->a1 / 8][_mg->a1 % 8].bit();
            rookDst.dropBitAtPoint(dragBitRook, dragBitRook->getPosition());
            _grid[_mg->a1 / 8][_mg->a1 % 8].setBit(nullptr);
        }
        // Black kingside
        else if(dst == _mg->g8){
            // king to g8
            // rook from h8 to f8
            ChessSquare &rookDst= static_cast<ChessSquare&>(_grid[_mg->f8 / 8][_mg->f8 % 8]);
            Bit* dragBitRook = _grid[_mg->h8 / 8][_mg->h8 % 8].bit();
            rookDst.dropBitAtPoint(dragBitRook, dragBitRook->getPosition());
            _grid[_mg->h8 / 8][_mg->h8 % 8].setBit(nullptr);
        }
        // Black queenside
        else if(dst == _mg->c8){
            // king to c8
            // rook from a8 to d8
            ChessSquare &rookDst= static_cast<ChessSquare&>(_grid[_mg->d8 / 8][_mg->d8 % 8]);
            Bit* dragBitRook = _grid[_mg->a8 / 8][_mg->a8 % 8].bit();
            rookDst.dropBitAtPoint(dragBitRook, dragBitRook->getPosition());
            _grid[_mg->a8 / 8][_mg->a8 % 8].setBit(nullptr);
        }
    }
    _mg->makeMove(move);
    _mg->generateMoves();
    endTurn();
}

//
// free all the memory used by the game on the heap
//
void Chess::stopGame()
{
}

Player* Chess::checkForWinner()
{
    if(_mg->isCheckmate()){
        return _players.at(!_mg->side);
    }
}

bool Chess::checkForDraw()
{
    return _mg->isStalemate();
}

//
// state strings
//
std::string Chess::initialStateString()
{
    return stateString();
}

//
// this still needs to be tied into imguis init and shutdown
// we will read the state string and store it in each turn object
//
std::string Chess::stateString() const
{
    std::string s;
    for (int y=0; y<_gameOptions.rowY; y++) {
        for (int x=0; x<_gameOptions.rowX; x++) {
            s += bitToPieceNotation( y, x );
        }
    }
    return s;
}

//
// this still needs to be tied into imguis init and shutdown
// when the program starts it will load the current game from the imgui ini file and set the game state to the last saved state
//
void Chess::setStateString(const std::string &s)
{
    // not implemented here
}
int Chess::evaluateBoard(const char* stateString){
    return 0;
}

ChessAI* Chess::clone(){
    ChessAI* ai = new ChessAI();
    MoveGenerator* mgCopy = new MoveGenerator(*_mg);
    ai->_mg = mgCopy;
    return ai;
}

void Chess::updateAI(){
    std::cout << "calling update ai" << std::endl;
    //_mg->generateMoves();
    uint32_t moveList[256];
    _mg->copyMoveList(moveList);
    int moveCount = _mg->copyMoveCount();
    std::cout << "Move count: " << moveCount << std::endl;

    int maxScore = -10000;
    int maxMove = moveList[0];

    for(int i = 0; i < moveCount; i++){

        ChessAI* ai = this->clone();

        ai->_mg->makeMove(moveList[i]);
        int score = ai->negamax(-9999, 9999, 2);

        if(score > maxScore){
            maxScore = score;
            maxMove = moveList[i];
        }
        delete ai;
    }

    // Make move
    std::cout << "making move" << std::endl;
    movePiece(maxMove);
    _mg->printBoard();
}

void ChessAI::setBoard(){

}

// returns winner
int ChessAI::AICheckForWinner(){
    if(_mg->isCheckmate()){
        std::cout << "checkmate, side under check" << _mg->side << std::endl;
        return !_mg->side;
    }
    return -1;
}

int ChessAI::evaluateBoard()
{
    if(AICheckForWinner() >= 0){
        // AI Lost
        if(_mg->side == AI_PLAYER){
            return -10;
        }
        // AI win
        else {
            return 10;
        }
    }

    // Evaluate un won board state

    return 0;
}

int ChessAI::negamax(int alpha, int beta, int depth){
    // negamax
    _mg->generateMoves();

    // Return at max depth
    if(depth == 0){
        return evaluateBoard();
    }
    // Evaluate board
    int score = evaluateBoard();
    if(score == 10) return score - depth;
    else if(score == -10) return score + depth;

    score = -10000;

    uint32_t moveList[256];
    _mg->copyMoveList(moveList);
    int moveCount = _mg->copyMoveCount();

    for(int i = 0; i < moveCount; i++){
        // Copy board state
        uint64_t bitboards[12];
        _mg->copyBitboards(bitboards);
        uint64_t allBlacks = _mg->copyAllBlacks();
        uint64_t allWhites = _mg->copyAllWhites();
        int side = _mg->copySide();
        int castle = _mg->copyCastle();
        int enpassant = _mg->copyEnpassant();
        int enpassantPiece = _mg->copyEnpassantPiece();
        _mg->makeMove(moveList[i]);

        int current = -negamax(-alpha, -beta, depth - 1);

        if(current > score){
            score = current;
        }

        _mg->undoBitboards(bitboards);
        _mg->setAllBlacks(allBlacks);
        _mg->setAllWhites(allWhites);
        _mg->setAllPieces(allBlacks, allWhites);
        _mg->setSide(side);
        _mg->setCastle(castle);
        _mg->setEnpassant(enpassant);
        _mg->setEnpassantPiece(enpassantPiece);
    }
    return score;
}