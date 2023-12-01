#pragma once
#include <cstdint>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <cmath>

#define set_bit(bitboard, square) ((bitboard |= (1ULL << square)))
#define get_bit(bitboard, square) ((bitboard & (1ULL << square)))
#define pop_bit(bitboard, square) ((get_bit(bitboard, square) ? bitboard ^= (1ULL << square) : 0))

class MoveGenerator {
public:
    MoveGenerator(){
        initPawnAttacks();
        initKnightAttacks();
        initKingAttacks();
        initSliderAttacks(1);
        initSliderAttacks(0);
        initSquaresBetween();
    };
    ~MoveGenerator(){};
    MoveGenerator(MoveGenerator& other){
        for(int i = 0; i < 12; i++){
            bitboards[i] = other.bitboards[i];
        }

        uint64_t allBlacks = other.allBlacks;
        uint64_t allWhites = other.allWhites;
        int side = other.side;
        int castle = other.castle;
        int enpassant = other.enpassant;
        uint64_t enpassantPiece = other.enpassantPiece;

        initPawnAttacks();
        initKnightAttacks();
        initKingAttacks();
        initSliderAttacks(1);
        initSliderAttacks(0);
        initSquaresBetween();
    }

    enum {
        a8, b8, c8, d8, e8, f8, g8, h8,
        a7, b7, c7, d7, e7, f7, g7, h7,
        a6, b6, c6, d6, e6, f6, g6, h6,
        a5, b5, c5, d5, e5, f5, g5, h5,
        a4, b4, c4, d4, e4, f4, g4, h4,
        a3, b3, c3, d3, e3, f3, g3, h3,
        a2, b2, c2, d2, e2, f2, g2, h2,
        a1, b1, c1, d1, e1, f1, g1, h1, nil
    };

    enum {
        P, N, B, R, Q, K, p, n, b, r, q, k
    };

    enum {
        white, black
    };

    // Enums for castling
    enum {
        wk = 1, wq = 2, bk = 4, bq = 8
    };

    // ASCII pieces
    char ascii_pieces[13] = "PNBRQKpnbrqk";

    // Constants to help check if pawns or knights is on the A/B or H/G files.
    // Copied from Chess Programming BBC Chess engine
    const uint64_t notAFile = 18374403900871474942;
    const uint64_t notHFile = 9187201950435737471;
    const uint64_t notHGFile = 4557430888798830399;
    const uint64_t notABFile = 18229723555195321596;

    // OCCUPANCY BITS: Calculated from the amount of possible attacks for bishops/rooks on a square, excluding the edge (as this is guaranteed)
    // Copied from Chess Programming's BBC Chess Engine

    // bishop relevant occupancy bit count for every square on board
    const int bishop_relevant_bits[64] = {
        6, 5, 5, 5, 5, 5, 5, 6,
        5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 7, 7, 7, 7, 5, 5,
        5, 5, 7, 9, 9, 7, 5, 5,
        5, 5, 7, 9, 9, 7, 5, 5,
        5, 5, 7, 7, 7, 7, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5,
        6, 5, 5, 5, 5, 5, 5, 6
    };

    // rook relevant occupancy bit count for every square on board
    const int rook_relevant_bits[64] = {
        12, 11, 11, 11, 11, 11, 11, 12,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        12, 11, 11, 11, 11, 11, 11, 12
    };

    // ROOK AND BISHOP MAGIC NUMBERS - Numbers to multiply an occupancy bitstring by for optimal hashing
    // Copied from Chess Programming's BBC Chess Engine

    // rook magic numbers
    const uint64_t rook_magic_numbers[64] = {
        0x8a80104000800020ULL,
        0x140002000100040ULL,
        0x2801880a0017001ULL,
        0x100081001000420ULL,
        0x200020010080420ULL,
        0x3001c0002010008ULL,
        0x8480008002000100ULL,
        0x2080088004402900ULL,
        0x800098204000ULL,
        0x2024401000200040ULL,
        0x100802000801000ULL,
        0x120800800801000ULL,
        0x208808088000400ULL,
        0x2802200800400ULL,
        0x2200800100020080ULL,
        0x801000060821100ULL,
        0x80044006422000ULL,
        0x100808020004000ULL,
        0x12108a0010204200ULL,
        0x140848010000802ULL,
        0x481828014002800ULL,
        0x8094004002004100ULL,
        0x4010040010010802ULL,
        0x20008806104ULL,
        0x100400080208000ULL,
        0x2040002120081000ULL,
        0x21200680100081ULL,
        0x20100080080080ULL,
        0x2000a00200410ULL,
        0x20080800400ULL,
        0x80088400100102ULL,
        0x80004600042881ULL,
        0x4040008040800020ULL,
        0x440003000200801ULL,
        0x4200011004500ULL,
        0x188020010100100ULL,
        0x14800401802800ULL,
        0x2080040080800200ULL,
        0x124080204001001ULL,
        0x200046502000484ULL,
        0x480400080088020ULL,
        0x1000422010034000ULL,
        0x30200100110040ULL,
        0x100021010009ULL,
        0x2002080100110004ULL,
        0x202008004008002ULL,
        0x20020004010100ULL,
        0x2048440040820001ULL,
        0x101002200408200ULL,
        0x40802000401080ULL,
        0x4008142004410100ULL,
        0x2060820c0120200ULL,
        0x1001004080100ULL,
        0x20c020080040080ULL,
        0x2935610830022400ULL,
        0x44440041009200ULL,
        0x280001040802101ULL,
        0x2100190040002085ULL,
        0x80c0084100102001ULL,
        0x4024081001000421ULL,
        0x20030a0244872ULL,
        0x12001008414402ULL,
        0x2006104900a0804ULL,
        0x1004081002402ULL
    };

    // bishop magic numbers
    const uint64_t bishop_magic_numbers[64] = {
        0x40040844404084ULL,
        0x2004208a004208ULL,
        0x10190041080202ULL,
        0x108060845042010ULL,
        0x581104180800210ULL,
        0x2112080446200010ULL,
        0x1080820820060210ULL,
        0x3c0808410220200ULL,
        0x4050404440404ULL,
        0x21001420088ULL,
        0x24d0080801082102ULL,
        0x1020a0a020400ULL,
        0x40308200402ULL,
        0x4011002100800ULL,
        0x401484104104005ULL,
        0x801010402020200ULL,
        0x400210c3880100ULL,
        0x404022024108200ULL,
        0x810018200204102ULL,
        0x4002801a02003ULL,
        0x85040820080400ULL,
        0x810102c808880400ULL,
        0xe900410884800ULL,
        0x8002020480840102ULL,
        0x220200865090201ULL,
        0x2010100a02021202ULL,
        0x152048408022401ULL,
        0x20080002081110ULL,
        0x4001001021004000ULL,
        0x800040400a011002ULL,
        0xe4004081011002ULL,
        0x1c004001012080ULL,
        0x8004200962a00220ULL,
        0x8422100208500202ULL,
        0x2000402200300c08ULL,
        0x8646020080080080ULL,
        0x80020a0200100808ULL,
        0x2010004880111000ULL,
        0x623000a080011400ULL,
        0x42008c0340209202ULL,
        0x209188240001000ULL,
        0x400408a884001800ULL,
        0x110400a6080400ULL,
        0x1840060a44020800ULL,
        0x90080104000041ULL,
        0x201011000808101ULL,
        0x1a2208080504f080ULL,
        0x8012020600211212ULL,
        0x500861011240000ULL,
        0x180806108200800ULL,
        0x4000020e01040044ULL,
        0x300000261044000aULL,
        0x802241102020002ULL,
        0x20906061210001ULL,
        0x5a84841004010310ULL,
        0x4010801011c04ULL,
        0xa010109502200ULL,
        0x4a02012000ULL,
        0x500201010098b028ULL,
        0x8040002811040900ULL,
        0x28000010020204ULL,
        0x6000020202d0240ULL,
        0x8918844842082200ULL,
        0x4010011029020020ULL
    };

    uint64_t blackPawns   = 0x000000000000ff00;
    uint64_t whitePawns   = 0x00ff000000000000;
    uint64_t blackKnights = 0x0000000000000042;
    uint64_t whiteKnights = 0x4200000000000000;
    uint64_t blackBishops = 0x0000000000000024;
    uint64_t whiteBishops = 0x2400000000000000;
    uint64_t blackRooks   = 0x0000000000000081;
    uint64_t whiteRooks   = 0x8100000000000000;
    uint64_t blackQueens  = 0x0000000000000008;
    uint64_t whiteQueens  = 0x0800000000000000;
    uint64_t blackKing    = 0x0000000000000010;
    uint64_t whiteKing    = 0x1000000000000000;

    //Precomputed square masks
    // copied from nkarve/surge 's chess engine
    const uint64_t SQUARE_BB[65] = {
        0x1, 0x2, 0x4, 0x8,
        0x10, 0x20, 0x40, 0x80,
        0x100, 0x200, 0x400, 0x800,
        0x1000, 0x2000, 0x4000, 0x8000,
        0x10000, 0x20000, 0x40000, 0x80000,
        0x100000, 0x200000, 0x400000, 0x800000,
        0x1000000, 0x2000000, 0x4000000, 0x8000000,
        0x10000000, 0x20000000, 0x40000000, 0x80000000,
        0x100000000, 0x200000000, 0x400000000, 0x800000000,
        0x1000000000, 0x2000000000, 0x4000000000, 0x8000000000,
        0x10000000000, 0x20000000000, 0x40000000000, 0x80000000000,
        0x100000000000, 0x200000000000, 0x400000000000, 0x800000000000,
        0x1000000000000, 0x2000000000000, 0x4000000000000, 0x8000000000000,
        0x10000000000000, 0x20000000000000, 0x40000000000000, 0x80000000000000,
        0x100000000000000, 0x200000000000000, 0x400000000000000, 0x800000000000000,
        0x1000000000000000, 0x2000000000000000, 0x4000000000000000, 0x8000000000000000,
        0x0
    };

    uint64_t allWhites = whitePawns | whiteRooks | whiteKnights | whiteBishops | whiteQueens | whiteKing;
    uint64_t allBlacks = blackPawns | blackRooks | blackKnights | blackBishops | blackQueens | blackKing;
    uint64_t allPieces = allWhites | allBlacks;
    uint64_t enpassant = nil;
    int enpassantPiece = nil;

    uint64_t bitboards[12];
    uint64_t pawnAttacks[2][64];
    uint64_t knightAttacks[64];
    uint64_t kingAttacks[64];

    uint64_t bishopMasks[64];
    uint64_t rookMasks[64];
    uint64_t bishopAttacks[64][512];
    uint64_t rookAttacks[64][4096];

    // lookup table to help with generating legal moves
    uint64_t squaresBetween[64][64];

    uint32_t moveList[256];
    int moveCount = 0;
    // Used for detecting checkmate
    int kingMoveCount = 0;

    int side;
    int castle;

    // Move representation
    // 0000 0000 0000 0000 0011 1111  Src Square     0x3f
    // 0000 0000 0000 1111 1100 0000  Dst Square     0xfc0
    // 0000 0000 1111 0000 0000 0000  Piece          0xf000
    // 0000 1111 0000 0000 0000 0000  Promoted piece 0xf0000
    // 0001 0000 0000 0000 0000 0000  Capture        0x100000
    // 0010 0000 0000 0000 0000 0000  Double push    0x200000
    // 0100 0000 0000 0000 0000 0000  Enpassant      0x400000
    // 1000 0000 0000 0000 0000 0000  Castling       0x800000

    /* HELPER FUNCTIONS FOR STORING AND UNDOING MOVE */

    void copyBitboards(uint64_t (&bitboardsCopy)[12]){
        std::copy(std::begin(bitboards), std::end(bitboards), std::begin(bitboardsCopy));
    }

    void copyMoveList(uint32_t (&moveListCopy)[256]){
        std::copy(std::begin(moveList), std::end(moveList), std::begin(moveListCopy));
    }

    int copyMoveCount(){ return moveCount; }

    uint64_t copyAllBlacks(){ return allBlacks; }
    uint64_t copyAllWhites(){ return allWhites; }
    int copySide(){ return side; }
    int copyCastle(){ return castle; }
    uint64_t copyEnpassant(){ return enpassant; }
    int copyEnpassantPiece(){ return enpassantPiece; }

    void undoBitboards(uint64_t (&bitboardsCopy)[12]){
        std::copy(std::begin(bitboardsCopy), std::end(bitboardsCopy), std::begin(bitboards));
    }

    void setAllBlacks(uint64_t _allBlacks){ allBlacks = _allBlacks; }
    void setAllWhites(uint64_t _allWhites){ allWhites = _allWhites; }
    void setAllPieces(uint64_t _allBlacks, uint64_t _allWhites){ allPieces = _allBlacks | _allWhites; }
    void setSide(int _side){ side = _side; }
    void setCastle(int _castle){ castle = _castle; }
    void setEnpassant(uint64_t _enpassant){ enpassant = _enpassant; }
    void setEnpassantPiece(int _enpassantPiece){ enpassantPiece = _enpassantPiece; }

    void initSquaresBetween(){
        uint64_t commonSquares;
        for (int sq1 = a8; sq1 <= h1; sq1++){
            for (int sq2 = a8; sq2 <= h1; sq2++) {
                commonSquares = SQUARE_BB[sq1] | SQUARE_BB[sq2];
                int sq1Rank = sq1/8;
                int sq1File = sq1%8;
                int sq2Rank = sq2/8;
                int sq2File = sq2%8;

                // on horizontal or vertical
                if (sq1Rank == sq2Rank || sq1File == sq2File)
                    squaresBetween[sq1][sq2] = getRookAttacks(sq1, commonSquares) & getRookAttacks(sq2, commonSquares);
                // on diagonal
                else if (std::abs(sq1Rank-sq2Rank) == std::abs(sq1File-sq2File))
                    squaresBetween[sq1][sq2] = getBishopAttacks(sq1, commonSquares) & getBishopAttacks(sq2, commonSquares);
            }
        }
    }

    void encodeMove(int src, int dst, int piece, int promotedPiece, int capture, int allowEnpassant, int castleMove){
        moveList[moveCount] = (0ULL | (src) | (dst << 6) | (piece << 12) | (promotedPiece << 16) | (capture << 20) | (allowEnpassant << 22) | (castleMove << 23));
        moveCount++;

        if(!side && piece == K){
            kingMoveCount++;
        }
        else if(side && piece == k){
            kingMoveCount++;
        }
    }

    bool isCheckmate(){
        // king is under attack and has no moves
        int whiteSquareAttacked = isSquareAttacked(getLsb(bitboards[K]), 1, nil);
        int blackSquareAttacked = isSquareAttacked(getLsb(bitboards[k]), 0, nil);

        if(whiteSquareAttacked){
            std::cout << "white square attacked: " << whiteSquareAttacked << std::endl;
            //printBoard();
            return moveCount == 0;
        }
        else if(blackSquareAttacked){
            std::cout << "black square attacked: " << blackSquareAttacked << std::endl;
            //printBoard();
            return moveCount == 0;
        }
        return false;
    }
    bool isStalemate(){
        return moveCount == 0;
    }

    int getSrc(uint32_t move){
        return (move & 0x3f);
    }

    int getDst(uint32_t move){
        return ((move & 0xfc0) >> 6);
    }

    int getPiece(uint32_t move){
        return ((move & 0xf000) >> 12);
    }

    int getPromotedPiece(uint32_t move){
        return ((move & 0xf0000) >> 16);
    }

    int getCapture(uint32_t move){
        return (move & 0x100000);
    }

    int getEnpassant(uint32_t move){
        return (move & 0x400000);
    }

    int getCastle(uint32_t move){
        return (move & 0x800000);
    }

    void makeMove(int move){
        int src = getSrc(move);
        int dst = getDst(move);
        int piece = getPiece(move);
        int promotedPiece = getPromotedPiece(move);
        int capture = getCapture(move);
        int allowEnpassant = getEnpassant(move);
        int castleMove = getCastle(move);
        pop_bit(bitboards[piece], src);

        // If king moves, make castle invalid
        if(piece == K && (castle & wk || castle & wq)){
            castle &= ~wk;
            castle &= ~wq;
        }
        if(piece == k && (castle & bk || castle & bq)){
            castle &= ~bk;
            castle &= ~bq;
        }
        // If rook moves, make castle invalid
        // White queenside rook
        if(src == a1 && (castle & wq)){
            castle &= ~wq;
        }
        // White kingside rook
        else if(src == h1 && (castle & wk)){
            castle &= ~wk;
        }
        // Black queenside rook
        else if(src == a8 && (castle & bq)){
            castle &= ~bq;
        }
        // Black kingside rook
        else if(src == h8 && (castle & bk)){
            castle &= ~bk;
        }

        if(castleMove){
            // White kingside
            if(dst == g1){
                pop_bit(bitboards[K], src);
                set_bit(bitboards[K], g1);
                pop_bit(bitboards[R], h1);
                set_bit(bitboards[R], f1);
            }
            // White queenside
            else if(dst == c1){
                pop_bit(bitboards[K], src);
                set_bit(bitboards[K], c1);
                pop_bit(bitboards[R], a1);
                set_bit(bitboards[R], d1);
            }
            // Black kingside
            else if(dst == g8){
                pop_bit(bitboards[k], src);
                set_bit(bitboards[k], g8);
                pop_bit(bitboards[r], h8);
                set_bit(bitboards[r], f8);
            }
            // Black queenside
            else if(dst == c8){
                pop_bit(bitboards[k], src);
                set_bit(bitboards[k], c8);
                pop_bit(bitboards[r], a8);
                set_bit(bitboards[r], d8);
            }
        }
        else {
            if(promotedPiece){
                set_bit(bitboards[promotedPiece], dst);
            }
            else {
                set_bit(bitboards[piece], dst);
            }

            if(enpassant && capture && enpassant == dst){
                pop_bit((!side ? bitboards[p] : bitboards[P]), enpassantPiece);
            }
            else if(capture){
                // Delete captured piece
                int startPiece = !side ? p : P;
                int endPiece = !side ? k : K;

                for(int i = startPiece; i <= endPiece; i++){
                    if(get_bit(bitboards[i], dst)){
                        pop_bit(bitboards[i], dst);
                    }
                }
            }
        }
        allWhites = 0;
        allBlacks = 0;
        for(int i = P; i <= K; i++){
            allWhites |= bitboards[i];
        }
        for(int i = p; i <= k; i++){
            allBlacks |= bitboards[i];
        }

        //std::cout << "Make move: allWhites: " << std::endl;
        //printBitboard(allWhites);
        //std::cout << "Make move: allBlacks: " << std::endl;
        //printBitboard(allBlacks);

        allPieces = 0;
        allPieces |= allBlacks;
        allPieces |= allWhites;

        side = !side;
        if(!allowEnpassant){
            enpassant = nil;
            enpassantPiece = nil;
        }
    }

    // encoding:
    // 0000 000000        0000 000000
    // second checker     first checker
    // piece  location    piece location

    // previous piece is needed for king evasion. makes sure it doesn't block checks.
    int isSquareAttacked(int square, int attackingSide, int previousSquare){
        int count = 0;
        int checkers = 0;
        int checkingSquare = 0;
        // Attacked by white side
        if(!attackingSide){
            // Attacked by white pawns
            uint64_t pawnAttacked = pawnAttacks[black][square] & bitboards[P];
            if(pawnAttacked){
                checkers |= (getLsb(pawnAttacked) << (count * 10));
                checkers |= ((P + 1) << (6 + count * 10));
                count++;
            }
        }
        // Black side
        else {
            uint64_t pawnAttacked = pawnAttacks[white][square] & bitboards[p];
            if(pawnAttacked){
                checkers |= (getLsb(pawnAttacked) << (count * 10));
                checkers |= ((p + 1) << (6 + count * 10));
                count++;
            }
        }

        uint64_t allPiecesMask = allPieces;
        if(previousSquare != nil){
            allPiecesMask ^= (1ULL << previousSquare);
        }

        uint64_t knightAttacked = knightAttacks[square] & (!attackingSide ? bitboards[N] : bitboards[n]);
        if(knightAttacked){
            checkers |= (getLsb(knightAttacked) << (count * 10));
            checkers |= (((!attackingSide ? N : n) + 1) << (6 + count * 10));
            count++;
        }
        uint64_t bishopAttacked = getBishopAttacks(square, allPiecesMask) & (!attackingSide ? bitboards[B] : bitboards[b]);
        std::cout << "all pieces" << std::endl;
        printBitboard(getBishopAttacks(square, allPieces));
        std::cout << "all piecesmask" << std::endl;
        printBitboard(getBishopAttacks(square, allPiecesMask));
        std::cout << "attacking bishop" << std::endl;
        printBitboard(!attackingSide ? bitboards[B] : bitboards[b]);
        if(bishopAttacked){
            checkers |= (getLsb(bishopAttacked) << (count * 10));
            checkers |= (((!attackingSide ? B : b) + 1) << (6 + count * 10));
            count++;
        }
        uint64_t rookAttacked = getRookAttacks(square, allPiecesMask) & (!attackingSide ? bitboards[R] : bitboards[r]);
        if(rookAttacked){
            checkers |= (getLsb(rookAttacked) << (count * 10));
            checkers |= (((!attackingSide ? R : r) + 1) << (6 + count * 10));
            count++;
        }

        uint64_t queenAttacked = getQueenAttacks(square, allPiecesMask) & (!attackingSide ? bitboards[Q] : bitboards[q]);
        if(queenAttacked){
            checkers |= (getLsb(queenAttacked) << (count * 10));
            checkers |= (((!attackingSide ? Q : q) + 1) << (6 + count * 10));
            count++;
        }

        uint64_t kingAttacked = kingAttacks[square] & (!attackingSide ? bitboards[K] : bitboards[k]);
        if(kingAttacked){
            checkers |= (getLsb(kingAttacked) << (count * 10));
            checkers |= (((!attackingSide ? K : k) + 1) << (6 + count * 10));
            count++;
        }
        return checkers;
    }

    int getCheckerLocation(int checkers){
        return (checkers & 0x3f);
    }
    int getCheckerPiece(int checkers){
        return ((checkers & 0x3c0) >> 6) - 1;
    }

    // convert ASCII character pieces to encoded constants
    int charPieces(char piece){
        switch(piece){
            case 'P':
                return P;
            case 'N':
                return N;
            case 'B':
                return B;
            case 'R':
                return R;
            case 'Q':
                return Q;
            case 'K':
                return K;
            case 'p':
                return p;
            case 'n':
                return n;
            case 'b':
                return b;
            case 'r':
                return r;
            case 'q':
                return q;
            case 'k':
                return k;
        }
        return -1;
    }

    // Parse Fen String function : Copied from Chess Programming's BBC Engine
    void parseFen(const char *fen){
        memset(bitboards, 0, sizeof(bitboards));
        allWhites = 0;
        allBlacks = 0;
        allPieces = 0;
        side = 0;
        enpassant = nil;
        castle = 0;

        for (int rank = 0; rank < 8; rank++){
            for (int file = 0; file < 8; file++){
                int square = rank * 8 + file;
                // match ascii pieces within FEN string
                if ((*fen >= 'a' && *fen <= 'z') || (*fen >= 'A' && *fen <= 'Z')){
                    // init piece type
                    int piece = charPieces(*fen);
                    set_bit(bitboards[piece], square);
                    fen++;
                }
                // match empty square numbers within FEN string
                if (*fen >= '0' && *fen <= '9'){
                    // init offset (convert char 0 to int 0)
                    int offset = *fen - '0';
                    int piece = -1;
                    for (int bb_piece = P; bb_piece <= k; bb_piece++){
                        // if there is a piece on current square
                        if (get_bit(bitboards[bb_piece], square))
                            piece = bb_piece;
                    }
                    // on empty current square
                    if (piece == -1)
                        // decrement file
                        file--;

                    // adjust file counter
                    file += offset;

                    // increment pointer to FEN string
                    fen++;
                }
                // match rank separator
                if (*fen == '/')
                    // increment pointer to FEN string
                    fen++;
            }
        }

        // got to parsing side to move (increment pointer to FEN string)
        fen++;

        // parse side to move
        (*fen == 'w') ? (side = white) : (side = black);

        // go to parsing castling rights
        fen += 2;

        // parse castling rights
        while (*fen != ' ')
        {
            switch (*fen)
            {
                case 'K': castle |= wk; break;
                case 'Q': castle |= wq; break;
                case 'k': castle |= bk; break;
                case 'q': castle |= bq; break;
                case '-': break;
            }

            // increment pointer to FEN string
            fen++;
        }

        // got to parsing enpassant square (increment pointer to FEN string)
        fen++;

        // parse enpassant square
        if (*fen != '-')
        {
            // parse enpassant file & rank
            int file = fen[0] - 'a';
            int rank = 8 - (fen[1] - '0');

            // init enpassant square
            enpassant = rank * 8 + file;
        }

        // no enpassant square
        else
            enpassant = nil;

        // loop over white pieces bitboards
        for (int piece = P; piece <= K; piece++)
            // populate white occupancy bitboard
            allWhites |= bitboards[piece];

        // loop over black pieces bitboards
        for (int piece = p; piece <= k; piece++)
            // populate white occupancy bitboard
            allBlacks |= bitboards[piece];

        // init all occupancies
        allPieces |= allWhites;
        allPieces |= allBlacks;
    }


    void setBitboards(){
        bitboards[0] = whitePawns;
        bitboards[1] = whiteKnights;
        bitboards[2] = whiteBishops;
        bitboards[3] = whiteRooks;
        bitboards[4] = whiteQueens;
        bitboards[5] = whiteKing;
        bitboards[6] = blackPawns;
        bitboards[7] = blackKnights;
        bitboards[8] = blackBishops;
        bitboards[9] = blackRooks;
        bitboards[10] = blackQueens;
        bitboards[11] = blackKing;
    }

    std::string squareToCoords(int square){
        std::string coords = "";
        coords += ('a' + (square % 8));
        coords += ('8' - (square / 8));
        return coords;
    }

    int countBits(uint64_t bitboard){
        int count = 0;
        while(bitboard){
            count++;
            bitboard &= bitboard - 1;
        }
        return count;
    }

    int getLsb(uint64_t bitboard){
        if(bitboard){
            return countBits((bitboard & -bitboard) - 1);
        }
        printBoard();
        return -1;
    }

    uint64_t generatePawnAttack(int square, int side){
        uint64_t attacks = 0;
        uint64_t bitboard = 0;

        set_bit(bitboard, square);

        //white pawns
        if(!side){
            attacks |= ((bitboard >> 7) & notAFile);
            attacks |= ((bitboard >> 9) & notHFile);
        }
        //black pawns
        else {
            attacks |= ((bitboard << 7) & notHFile);
            attacks |= ((bitboard << 9) & notAFile);
        }

        return attacks;
    }

    uint64_t generateKnightAttack(int square){
        uint64_t attacks = 0;
        uint64_t bitboard = 0;

        set_bit(bitboard, square);

        attacks |= ((bitboard >> 15) & notAFile);
        attacks |= ((bitboard >> 17) & notHFile);

        attacks |= ((bitboard >> 10) & notHGFile);
        attacks |= ((bitboard >> 6) & notABFile);

        attacks |= ((bitboard << 15) & notHFile);
        attacks |= ((bitboard << 17) & notAFile);

        attacks |= ((bitboard << 10) & notABFile);
        attacks |= ((bitboard << 6) & notHGFile);

        return attacks;
    }

    uint64_t generateKingAttack(int square){
        uint64_t attacks = 0;
        uint64_t bitboard = 0;

        set_bit(bitboard, square);

        // Up right diagonal
        attacks |= ((bitboard >> 7) & notAFile);
        // Up
        attacks |= (bitboard >> 8);
        // Up left diagonal
        attacks |= ((bitboard >> 9) & notHFile);

        // Down left diagonal
        attacks |= ((bitboard << 7) & notHFile);
        // Down
        attacks |= (bitboard << 8);
        // Down right diagonal
        attacks |= ((bitboard << 9) & notAFile);

        // Left
        attacks |= ((bitboard >> 1) & notHFile);
        // Right
        attacks |= ((bitboard << 1) & notAFile);

        return attacks;
    }

    uint64_t generateBishopAttack(int square){
        uint64_t attacks = 0;
        int r = square / 8;
        int c = square % 8;
        int i, j;

        for(i = r + 1, j = c + 1; i <= 6 && j <= 6; i++, j++) attacks |= (1ULL << (i * 8 + j));
        for(i = r + 1, j = c - 1; i <= 6 && j >= 1; i++, j--) attacks |= (1ULL << (i * 8 + j));
        for(i = r - 1, j = c + 1; i >= 1 && j <= 6; i--, j++) attacks |= (1ULL << (i * 8 + j));
        for(i = r - 1, j = c - 1; i >= 1 && j >= 1; i--, j--) attacks |= (1ULL << (i * 8 + j));

        return attacks;
    }

    uint64_t generateRookAttack(int square){
        uint64_t attacks = 0;
        int r = square / 8;
        int c = square % 8;
        int i, j;

        for(i = r + 1; i <= 6; i++) attacks |= (1ULL << (i * 8 + c));
        for(i = r - 1; i >= 1; i--) attacks |= (1ULL << (i * 8 + c));
        for(j = c + 1; j <= 6; j++) attacks |= (1ULL << (r * 8 + j));
        for(j = c - 1; j >= 1; j--) attacks |= (1ULL << (r * 8 + j));

        return attacks;
    }

    uint64_t generateBlockedBishopAttack(int square, uint64_t blockers){
        uint64_t attacks = 0;
        uint64_t attack = 0;
        int r = square / 8;
        int c = square % 8;
        int i, j;

        for(i = r + 1, j = c + 1; i <= 7 && j <= 7; i++, j++){
            attack = (1ULL << (i * 8 + j));
            attacks |= attack;
            if(attack & blockers) break;
        }
        for(i = r + 1, j = c - 1; i <= 7 && j >= 0; i++, j--){
            attack = (1ULL << (i * 8 + j));
            attacks |= attack;
            if(attack & blockers) break;
        }
        for(i = r - 1, j = c + 1; i >= 0 && j <= 7; i--, j++){
            attack = (1ULL << (i * 8 + j));
            attacks |= attack;
            if(attack & blockers) break;
        }
        for(i = r - 1, j = c - 1; i >= 0 && j >= 0; i--, j--){
            attack = (1ULL << (i * 8 + j));
            attacks |= attack;
            if(attack & blockers) break;
        }
        return attacks;
    }

    uint64_t generateBlockedRookAttack(int square, uint64_t blockers){
        uint64_t attacks = 0;
        uint64_t attack = 0;
        int r = square / 8;
        int c = square % 8;
        int i, j;

        for(i = r + 1; i <= 7; i++){
            attack = (1ULL << (i * 8 + c));
            attacks |= attack;
            if(attack & blockers) break;
        }
        for(i = r - 1; i >= 0; i--){
            attack = (1ULL << (i * 8 + c));
            attacks |= attack;
            if(attack & blockers) break;
        }
        for(j = c + 1; j <= 7; j++){
            attack = (1ULL << (r * 8 + j));
            attacks |= attack;
            if(attack & blockers) break;
        }
        for(j = c - 1; j >= 0; j--){
            attack = (1ULL << (r * 8 + j));
            attacks |= attack;
            if(attack & blockers) break;
        }

        return attacks;
    }

    // Set Occupancies: Copied from Chess Programming's BBC Chess Engine
    uint64_t getPossibleOccupancies(int index, int bitsInMask, uint64_t attackMask){
        uint64_t occupancy = 0;

        for(int count = 0; count < bitsInMask; count++){
            int square = getLsb(attackMask);
            pop_bit(attackMask, square);

            if(index & (1 << count))
                occupancy |= (1ULL << square);
        }

        return occupancy;
    }



    void initPawnAttacks(){
        for(int i = 0; i < 64; i++){
            pawnAttacks[0][i] = generatePawnAttack(i, 0);
            pawnAttacks[1][i] = generatePawnAttack(i, 1);
        }
    }
    void initKnightAttacks(){
        for(int i = 0; i < 64; i++){
            knightAttacks[i] = generateKnightAttack(i);
        }
    }
    void initKingAttacks(){
        for(int i = 0; i < 64; i++){
            kingAttacks[i] = generateKingAttack(i);
        }
    }

    void initSliderAttacks(int bishop){
        for(int square = 0; square < 64; square++){

            // All possible attacks (Except edge)
            bishopMasks[square] = generateBishopAttack(square);
            rookMasks[square] = generateRookAttack(square);

            // All possible attacks of the given square (except edge)
            uint64_t attackMask = bishop ? bishopMasks[square] : rookMasks[square];

            // Number of bits needed to store the attack board
            int relevantBitsCount = countBits(attackMask);
            //
            int occupancyIndices = (1 << relevantBitsCount);

            for(int i = 0; i < occupancyIndices; i++){
                if(bishop){
                    uint64_t occupancy = getPossibleOccupancies(i, relevantBitsCount, attackMask);
                    int magicIndex = (occupancy * bishop_magic_numbers[square]) >> (64 - bishop_relevant_bits[square]);
                    bishopAttacks[square][magicIndex] = generateBlockedBishopAttack(square, occupancy);
                }
                else {
                    uint64_t occupancy = getPossibleOccupancies(i, relevantBitsCount, attackMask);
                    int magicIndex = (occupancy * rook_magic_numbers[square]) >> (64 - rook_relevant_bits[square]);
                    rookAttacks[square][magicIndex] = generateBlockedRookAttack(square, occupancy);
                }
            }

        }
    }

    uint64_t getBishopAttacks(int square, uint64_t occupancy){
        occupancy &= bishopMasks[square];
        occupancy *= bishop_magic_numbers[square];
        occupancy >>= 64 - bishop_relevant_bits[square];

        return bishopAttacks[square][occupancy];
    }

    uint64_t getRookAttacks(int square, uint64_t occupancy){
        occupancy &= rookMasks[square];
        occupancy *= rook_magic_numbers[square];
        occupancy >>= 64 - rook_relevant_bits[square];

        return rookAttacks[square][occupancy];
    }

    uint64_t getQueenAttacks(int square, uint64_t occupancy){

        uint64_t queenAttacks = 0;
        uint64_t bishopOccupancy = occupancy;
        uint64_t rookOccupancy = occupancy;

        bishopOccupancy &= bishopMasks[square];
        bishopOccupancy *= bishop_magic_numbers[square];
        bishopOccupancy >>= 64 - bishop_relevant_bits[square];

        rookOccupancy &= rookMasks[square];
        rookOccupancy *= rook_magic_numbers[square];
        rookOccupancy >>= 64 - rook_relevant_bits[square];

        queenAttacks = bishopAttacks[square][bishopOccupancy];
        queenAttacks |= rookAttacks[square][rookOccupancy];
        return queenAttacks;
    }

    void printBitboard(uint64_t bitboard){
        for(int rank = 0; rank < 8; rank++){
            for(int file = 0; file < 8; file++){
                int square = rank * 8 + file;
                if(!file)
                    std::cout << "  " << 8 - rank << "  ";
                std::cout << " " << (get_bit(bitboard, square) ? "1" : ".");
            }
            std::cout << std::endl;
        }
        std::cout << "      a b c d e f g h\n\n";
        std::cout << bitboard << std::endl;
    }

    void printBoard(){
        for(int rank = 0; rank < 8; rank++){
            for(int file = 0; file < 8; file++){
                int square = rank * 8 + file;
                if(!file)
                    std::cout << "  " << 8 - rank << "  ";
                int piece = -1;
                for(int i = P; i <= k; i++){
                    if(get_bit(bitboards[i], square))
                        piece = i;
                }

                std::cout << " " << ((piece == -1) ? '.' : ascii_pieces[piece]);
            }
            std::cout << std::endl;
        }
        std::cout << "      a b c d e f g h\n\n";
        std::cout << "      Side: " << (!side ? "White" : "Black");
        std::cout << "      Enpassant: " << ((enpassant != nil) ? squareToCoords(enpassant) : "No");
        std::cout << "      Castling: " << ((castle & wk) ? 'K' : '-') << ((castle & wq) ? 'Q' : '-') << ((castle & bk) ? 'k' : '-') << ((castle & bq) ? 'q' : '-');
        std::cout << std::endl;
    }

    // generate only legal moves, when king is in check
    void generateLegalMoves(){
        // if king is attacked by two pieces, only encode king evasion moves.
        // 15 = 1111,  checkers are encoded like 0000 0000 second checker first checker
        uint64_t bitboard, attacks;
        moveCount = 0;
        kingMoveCount = 0;

        int checkers = !side ? isSquareAttacked(getLsb(bitboards[K]), 1, nil) : isSquareAttacked(getLsb(bitboards[k]), 0, nil);

        // King
        bitboard = !side ? bitboards[K] : bitboards[k];
        while(bitboard){
            int src = getLsb(bitboard);
            attacks = kingAttacks[src] & (!side ? ~allWhites : ~allBlacks);

            while(attacks){
                int target = getLsb(attacks);

                // Quiet move
                if(!isSquareAttacked(target, !side, src) && (!get_bit((!side ? allBlacks : allWhites), target))){
                    std::cout << "King quiet move " << squareToCoords(src) << " -> " << squareToCoords(target) << std::endl;
                    encodeMove(src, target, (!side ? K : k), 0, 0, 0, 0);
                }
                // Capture Move
                else if(!isSquareAttacked(target, !side, src)){
                    std::cout << "King capture move " << squareToCoords(src) << " -> " << squareToCoords(target) << std::endl;
                    encodeMove(src, target, (!side ? K : k), 0, 1, 0, 0);
                }

                pop_bit(attacks, target);
            }
            pop_bit(bitboard, src);
        }

        // two checkers, only generate king moves
        if(checkers >= 1024){
            return;
        }

        // only one checker, subtract one , isSquareattacked returns 1-index pieces to avoid 0
        int checkingPiece = getCheckerPiece(checkers);
        int checkingPieceSquare = getCheckerLocation(checkers);
        uint64_t blockingMask = squaresBetween[!side ? getLsb(bitboards[K]) : getLsb(bitboards[k])][checkingPieceSquare];

        std::cout<<"common squares: "<<std::endl;
        printBitboard(SQUARE_BB[!side ? getLsb(bitboards[K]) : getLsb(bitboards[k])] | SQUARE_BB[checkingPieceSquare]);

        std::cout<<"blocking mask: "<<std::endl;
        printBitboard(blockingMask);

        // knight, pawn, queen, rook, bishop -> captures, and quiet moves that coincide with blocking mask

        // IF CHECKER IS KNIGHT OR PAWN - only generate captures.
        bool onlyGenerateCaptures = checkingPiece == N || checkingPiece == n || checkingPiece == P || checkingPiece == p;

        // White pawn quiet moves and captures
        if(side == white){
            // captures
            attacks = pawnAttacks[black][checkingPieceSquare] & bitboards[P];

            while(attacks){
                int src = getLsb(attacks);
                if(checkingPieceSquare < a7){
                    std::cout << "CheckEvasion: Pawn promotion capture " << squareToCoords(src) << " -> " << squareToCoords(checkingPieceSquare) << std::endl;
                    encodeMove(src, checkingPieceSquare, P, Q, 1, 0, 0);
                }
                // enpassant is also handle in this case automatically by makemove
                else {
                    std::cout << "CheckEvasion: Pawn capture " << squareToCoords(src) << " -> " << squareToCoords(checkingPieceSquare) << std::endl;
                    encodeMove(src, checkingPieceSquare, P, 0, 1, 0, 0);
                }
                pop_bit(attacks, src);
            }
            // quiet moves
            if(!onlyGenerateCaptures){
                bitboard = bitboards[P];
                while(bitboard){
                    int src = getLsb(bitboard);
                    // Forward push
                    int dst = src - 8;

                    // if single forward push will block a check
                    if(blockingMask & (1ULL << dst)){
                        if(dst >= a8 && !get_bit(allPieces, dst)){
                            // Check for promotion
                            if(dst < a7){
                                std::cout << "CheckEvasion: Pawn promotion " << squareToCoords(src) << " -> " << squareToCoords(dst) << std::endl;
                                encodeMove(src, dst, P, Q, 0, 0, 0);
                            }
                            else {
                                std::cout << "CheckEvasion: Pawn quiet move " << squareToCoords(src) << " -> " << squareToCoords(dst) << std::endl;
                                encodeMove(src, dst, P, 0, 0, 0, 0);
                            }
                        }
                    }
                    int doubleDst = dst - 8;
                    // if double push will block a check
                    if(blockingMask & (1ULL << doubleDst)){
                        if(src >= a2 && src <= h2 && (!get_bit(allPieces, doubleDst))){
                            if(pawnAttacks[white][dst] & bitboards[p]){
                                std::cout << "CheckEvasion: Move up twice (enemy can enpassant)" << squareToCoords(src) << " -> " << squareToCoords(doubleDst) << std::endl;
                                enpassant = dst;
                                enpassantPiece = doubleDst;
                                encodeMove(src, doubleDst, P, 0, 0, 1, 0);
                            }
                            else {
                                std::cout << "CheckEvasion: Move up twice " << squareToCoords(src) << " -> " << squareToCoords(doubleDst) << std::endl;
                                encodeMove(src, doubleDst, P, 0, 0, 0, 0);
                            }
                        }
                    }
                    pop_bit(bitboard, src);
                }
            }
        }
        else if(side == black){
            attacks = pawnAttacks[white][checkingPieceSquare] & bitboards[p];

            while(attacks){
                int src = getLsb(attacks);
                if(checkingPieceSquare >= a1){
                    std::cout << "CheckEvasion: Pawn promotion capture " << squareToCoords(src) << " -> " << squareToCoords(checkingPieceSquare) << std::endl;
                    encodeMove(src, checkingPieceSquare, p, q, 1, 0, 0);
                }
                else {
                    std::cout << "CheckEvasion: Pawn capture " << squareToCoords(src) << " -> " << squareToCoords(checkingPieceSquare) << std::endl;
                    encodeMove(src, checkingPieceSquare, p, 0, 1, 0, 0);
                }
                pop_bit(attacks, src);
            }
            // quiet moves
            if(!onlyGenerateCaptures){
                bitboard = bitboards[p];
                while(bitboard){
                    int src = getLsb(bitboard);
                    // Forward push
                    int dst = src + 8;

                    // if single forward push will block a check
                    if(blockingMask & (1ULL << dst)){
                        if(dst <= h1 && !get_bit(allPieces, dst)){
                            // Check for promotion
                            if(dst >= a1){
                                std::cout << "CheckEvasion: Pawn promotion " << squareToCoords(src) << " -> " << squareToCoords(dst) << std::endl;
                                encodeMove(src, dst, p, q, 0, 0, 0);
                            }
                            else {
                                std::cout << "CheckEvasion: Pawn quiet move " << squareToCoords(src) << " -> " << squareToCoords(dst) << std::endl;
                                encodeMove(src, dst, p, 0, 0, 0, 0);
                            }
                        }
                    }
                    int doubleDst = dst + 8;
                    // if double push will block a check
                    if(blockingMask & (1ULL << doubleDst)){
                        if(src >= a7 && src <= h7 && (!get_bit(allPieces, doubleDst))){
                            if(pawnAttacks[black][dst] & bitboards[P]){
                                enpassant = dst;
                                enpassantPiece = doubleDst;
                                std::cout << "CheckEvasion: Move up twice (enemy can enpassant)" << squareToCoords(src) << " -> " << squareToCoords(doubleDst) << std::endl;
                                encodeMove(src, doubleDst, p, 0, 0, 1, 0);
                            }
                            else {
                                std::cout << "CheckEvasion: Move up twice " << squareToCoords(src) << " -> " << squareToCoords(doubleDst) << std::endl;
                                encodeMove(src, doubleDst, p, 0, 0, 0, 0);
                            }
                        }
                    }
                    pop_bit(bitboard, src);
                }
            }
        }
        // knight

        // knight captures checking piece
        uint64_t evasionCaptures = knightAttacks[checkingPieceSquare] & (!side ? bitboards[N] : bitboards[n]);

        while(evasionCaptures){
            int src = getLsb(evasionCaptures);
            std::cout << "CheckEvasion: Knight capture move " << squareToCoords(src) << " -> " << squareToCoords(checkingPieceSquare) << std::endl;
            encodeMove(src, checkingPieceSquare, (!side ? N : n), 0, 1, 0, 0);
            pop_bit(evasionCaptures, src);
        }
        // knight quiet moves
        if(!onlyGenerateCaptures){
            bitboard = !side ? bitboards[N] : bitboards[n];
            while(bitboard){
                int src = getLsb(bitboard);
                attacks = knightAttacks[src] & (!side ? ~allWhites : ~allBlacks);
                uint64_t quietBlocks = attacks & blockingMask;

                while(quietBlocks){
                    int target = getLsb(quietBlocks);
                    std::cout << "CheckEvasion: Knight quiet move " << squareToCoords(src) << " -> " << squareToCoords(target) << std::endl;
                    encodeMove(src, target, (!side ? N : n), 0, 0, 0, 0);
                    pop_bit(quietBlocks, target);
                }
                pop_bit(bitboard, src);
            }
        }

        // Bishop
        bitboard = !side ? bitboards[B] : bitboards[b];
        while(bitboard){
            int src = getLsb(bitboard);
            attacks = getBishopAttacks(src, allPieces) & (!side ? ~allWhites : ~allBlacks);
            uint64_t quietBlocks = attacks & blockingMask;
            uint64_t evasionCaptures = getBishopAttacks(checkingPieceSquare, allPieces) & (!side ? bitboards[B] : bitboards[b]);

            // capture checking piece
            while(evasionCaptures){
                int src = getLsb(evasionCaptures);
                std::cout << "CheckEvasion: Bishop capture move " << squareToCoords(src) << " -> " << squareToCoords(checkingPieceSquare) << std::endl;
                encodeMove(src, checkingPieceSquare, (!side ? B : b), 0, 1, 0, 0);
                pop_bit(evasionCaptures, src);
            }
            if(!onlyGenerateCaptures){
                // quiet moves that block
                while(quietBlocks){
                    int target = getLsb(quietBlocks);
                    std::cout << "CheckEvasion: Bishop quiet move " << squareToCoords(src) << " -> " << squareToCoords(target) << std::endl;
                    encodeMove(src, target, (!side ? B : b), 0, 0, 0, 0);
                    pop_bit(quietBlocks, target);
                }
            }
            pop_bit(bitboard, src);
        }

        // Rook
        bitboard = !side ? bitboards[R] : bitboards[r];
        while(bitboard){
            int src = getLsb(bitboard);
            attacks = getRookAttacks(src, allPieces) & (!side ? ~allWhites : ~allBlacks);
            uint64_t quietBlocks = attacks & blockingMask;
            uint64_t evasionCaptures = getRookAttacks(checkingPieceSquare, allPieces) & (!side ? bitboards[R] : bitboards[r]);

            // capture checking piece
            while(evasionCaptures){
                int src = getLsb(evasionCaptures);
                std::cout << "CheckEvasion: Rook capture move " << squareToCoords(src) << " -> " << squareToCoords(checkingPieceSquare) << std::endl;
                encodeMove(src, checkingPieceSquare, (!side ? R : r), 0, 1, 0, 0);
                pop_bit(evasionCaptures, src);
            }
            if(!onlyGenerateCaptures){
                // quiet moves that block
                while(quietBlocks){
                    int target = getLsb(quietBlocks);
                    std::cout << "CheckEvasion: Rook quiet move " << squareToCoords(src) << " -> " << squareToCoords(target) << std::endl;
                    encodeMove(src, target, (!side ? R : r), 0, 0, 0, 0);
                    pop_bit(quietBlocks, target);
                }
            }
            pop_bit(bitboard, src);
        }

        // Queen
        bitboard = !side ? bitboards[Q] : bitboards[q];
        while(bitboard){
            int src = getLsb(bitboard);
            attacks = getQueenAttacks(src, allPieces) & (!side ? ~allWhites : ~allBlacks);
            uint64_t quietBlocks = attacks & blockingMask;
            uint64_t evasionCaptures = getQueenAttacks(checkingPieceSquare, allPieces) & (!side ? bitboards[Q] : bitboards[q]);

            // capture checking piece
            while(evasionCaptures){
                int src = getLsb(evasionCaptures);
                std::cout << "CheckEvasion: Queen capture move " << squareToCoords(src) << " -> " << squareToCoords(checkingPieceSquare) << std::endl;
                encodeMove(src, checkingPieceSquare, (!side ? Q : q), 0, 1, 0, 0);
                pop_bit(evasionCaptures, src);
            }
            if(!onlyGenerateCaptures){
                // quiet moves that block
                while(quietBlocks){
                    int target = getLsb(quietBlocks);
                    std::cout << "CheckEvasion: Queen quiet move " << squareToCoords(src) << " -> " << squareToCoords(target) << std::endl;
                    encodeMove(src, target, (!side ? Q : q), 0, 0, 0, 0);
                    pop_bit(quietBlocks, target);
                }
            }

            pop_bit(bitboard, src);
        }
    }

    void generateMoves(){
        // king is under check, generate strictly legal moves
        if((!side && isSquareAttacked(getLsb(bitboards[K]), 1, nil)) || (side && isSquareAttacked(getLsb(bitboards[k]), 0, nil))){
            generateLegalMoves();
            return;
        }

        uint64_t bitboard, attacks;
        moveCount = 0;
        kingMoveCount = 0;

        if(side == white){
            // White pawn
            //std::cout << "White moves: " << std::endl;
            bitboard = bitboards[P];

            while(bitboard){
                int src = getLsb(bitboard);
                // Forward push
                int dst = src - 8;

                if(dst >= a8 && !get_bit(allPieces, dst)){
                    // Check for promotion
                    if(dst < a7){
                        //std::cout << "Pawn promotion " << squareToCoords(src) << " -> " << squareToCoords(dst) << std::endl;
                        encodeMove(src, dst, P, Q, 0, 0, 0);
                    }
                    else {
                        // Single push
                        //std::cout << "Move up one " << squareToCoords(src) << " -> " << squareToCoords(dst) << std::endl;
                        encodeMove(src, dst, P, 0, 0, 0, 0);
                        if(src >= a2 && src <= h2 && (!get_bit(allPieces, dst - 8))){
                            // Double push
                            int doubleDst = dst - 8;
                            //std::cout << "Move up twice " << squareToCoords(src) << " -> " << squareToCoords(doubleDst) << std::endl;

                            if(pawnAttacks[white][dst] & bitboards[p]){
                                enpassant = dst;
                                enpassantPiece = doubleDst;
                                encodeMove(src, doubleDst, P, 0, 0, 1, 0);
                            }
                            else {
                                encodeMove(src, doubleDst, P, 0, 0, 0, 0);
                            }
                        }
                    }
                }

                attacks = pawnAttacks[side][src] & allBlacks;

                while(attacks){
                    int attack = getLsb(attacks);
                    if(attack < a7){
                        //std::cout << "Pawn promotion capture " << squareToCoords(src) << " -> " << squareToCoords(attack) << std::endl;
                        encodeMove(src, attack, P, Q, 1, 0, 0);
                    }
                    else {
                        //std::cout << "Pawn capture " << squareToCoords(src) << " -> " << squareToCoords(attack) << std::endl;
                        encodeMove(src, attack, P, 0, 1, 0, 0);
                    }
                    pop_bit(attacks, attack);
                }

                if(enpassant != nil){
                    uint64_t enpassantAttacks = pawnAttacks[side][src] & (1ULL << enpassant);
                    if(enpassantAttacks){
                        int attack = getLsb(enpassantAttacks);
                        //std::cout << "Pawn enpassant capture " << squareToCoords(src) << " -> " << squareToCoords(attack) << std::endl;
                        encodeMove(src, attack, P, 0, 1, 0, 0);
                    }
                }

                pop_bit(bitboard, src);
            }

            // King Castling Moves
            if(castle & wk){
                // Make sure kingside squares are empty
                if(!get_bit(allPieces, f1) && !get_bit(allPieces, g1)){
                    if(!isSquareAttacked(e1, black, nil) && !isSquareAttacked(f1, black, nil)){
                        //std::cout << "Castling move e1 -> g1" << std::endl;
                        encodeMove(e1, g1, K, 0, 0, 0, 1);
                    }
                }
            }
            if(castle & wq){
                // Make sure queenside squares are empty
                if(!get_bit(allPieces, b1) && !get_bit(allPieces, c1) && !get_bit(allPieces, d1)){
                    if(!isSquareAttacked(e1, black, nil) && !isSquareAttacked(d1, black, nil)){
                        //std::cout << "Castling move e1 -> c1" << std::endl;
                        encodeMove(e1, c1, K, 0, 0, 0, 1);
                    }
                }
            }
        }
        else if(side == black){
            //std::cout << "Black moves: " << std::endl;
            // Black pawn
            bitboard = bitboards[p];

            while(bitboard){
                int src = getLsb(bitboard);
                // Forward push
                int dst = src + 8;

                if(dst <= h1 && !get_bit(allPieces, dst)){
                    // Check for promotion
                    if(dst >= a1){
                        //std::cout << "Pawn promotion " << squareToCoords(src) << " -> " << squareToCoords(dst) << std::endl;
                        encodeMove(src, dst, p, q, 0, 0, 0);
                    }
                    else {
                        // Single push
                        //std::cout << "Move up one " << squareToCoords(src) << " -> " << squareToCoords(dst) << std::endl;
                        encodeMove(src, dst, p, 0, 0, 0, 0);
                        if(src >= a7 && src <= h7 && (!get_bit(allPieces, dst + 8))){
                            // Double push
                            int doubleDst = dst + 8;
                            //std::cout << "Move up twice " << squareToCoords(src) << " -> " << squareToCoords(doubleDst) << std::endl;
                            if(pawnAttacks[black][dst] & bitboards[P]){
                                enpassant = dst;
                                enpassantPiece = doubleDst;
                                encodeMove(src, doubleDst, p, 0, 0, 1, 0);
                            }
                            else {
                                encodeMove(src, doubleDst, p, 0, 0, 0, 0);
                            }
                        }
                    }
                }

                attacks = pawnAttacks[side][src] & allWhites;

                while(attacks){
                    int attack = getLsb(attacks);
                    if(attack >= a1){
                        //std::cout << "Pawn promotion capture " << squareToCoords(src) << " -> " << squareToCoords(attack) << std::endl;
                        encodeMove(src, attack, p, q, 1, 0, 0);
                    }
                    else {
                        //std::cout << "Pawn capture " << squareToCoords(src) << " -> " << squareToCoords(attack) << std::endl;
                        encodeMove(src, attack, p, 0, 1, 0, 0);
                    }
                    pop_bit(attacks, attack);
                }

                if(enpassant != nil){
                    uint64_t enpassantAttacks = pawnAttacks[side][src] & (1ULL << enpassant);
                    if(enpassantAttacks){
                        int attack = getLsb(enpassantAttacks);
                        //std::cout << "Pawn enpassant capture " << squareToCoords(src) << " -> " << squareToCoords(attack) << std::endl;
                        encodeMove(src, attack, p, 0, 1, 0, 0);
                    }
                }

                pop_bit(bitboard, src);
            }
            // King Castling Moves
            if(castle & bk){
                // Make sure kingside squares are empty
                if(!get_bit(allPieces, f8) && !get_bit(allPieces, g8)){
                    if(!isSquareAttacked(e8, white, nil) && !isSquareAttacked(f8, white, nil)){
                        //std::cout << "Castling move e8 -> g8" << std::endl;
                        encodeMove(e8, g8, k, 0, 0, 0, 1);
                    }
                }

            }
            if(castle & bq){
                // Make sure queenside squares are empty
                if(!get_bit(allPieces, b8) && !get_bit(allPieces, c8) && !get_bit(allPieces, d8)){
                    if(!isSquareAttacked(e8, white, nil) && !isSquareAttacked(d8, white, nil)){
                        //std::cout << "Castling move e8 -> c8" << std::endl;
                        encodeMove(e8, c8, k, 0, 0, 0, 1);
                    }
                }
            }
        }

        // Knight
        bitboard = !side ? bitboards[N] : bitboards[n];
        while(bitboard){
            int src = getLsb(bitboard);
            attacks = knightAttacks[src] & (!side ? ~allWhites : ~allBlacks);

            while(attacks){
                int target = getLsb(attacks);

                // Quiet move
                if(!get_bit((!side ? allBlacks : allWhites), target)){
                    //std::cout << "Knight quiet move " << squareToCoords(src) << " -> " << squareToCoords(target) << std::endl;
                    encodeMove(src, target, (!side ? N : n), 0, 0, 0, 0);
                }
                // Capture Move
                else {
                    //std::cout << "Knight capture move " << squareToCoords(src) << " -> " << squareToCoords(target) << std::endl;
                    encodeMove(src, target, (!side ? N : n), 0, 1, 0, 0);
                }

                pop_bit(attacks, target);
            }
            pop_bit(bitboard, src);
        }

        // Bishop
        bitboard = !side ? bitboards[B] : bitboards[b];
        while(bitboard){
            int src = getLsb(bitboard);
            attacks = getBishopAttacks(src, allPieces) & (!side ? ~allWhites : ~allBlacks);

            while(attacks){
                int target = getLsb(attacks);

                // Quiet move
                if(!get_bit((!side ? allBlacks : allWhites), target)){
                    //std::cout << "Bishop quiet move " << squareToCoords(src) << " -> " << squareToCoords(target) << std::endl;
                    encodeMove(src, target, (!side ? B : b), 0, 0, 0, 0);
                }
                // Capture Move
                else {
                    //std::cout << "Bishop capture move " << squareToCoords(src) << " -> " << squareToCoords(target) << std::endl;
                    encodeMove(src, target, (!side ? B : b), 0, 1, 0, 0);
                }

                pop_bit(attacks, target);
            }
            pop_bit(bitboard, src);
        }

        // Rook
        bitboard = !side ? bitboards[R] : bitboards[r];
        while(bitboard){
            int src = getLsb(bitboard);
            attacks = getRookAttacks(src, allPieces) & (!side ? ~allWhites : ~allBlacks);

            while(attacks){
                int target = getLsb(attacks);

                // Quiet move
                if(!get_bit((!side ? allBlacks : allWhites), target)){
                    //std::cout << "Rook quiet move " << squareToCoords(src) << " -> " << squareToCoords(target) << std::endl;
                    encodeMove(src, target, (!side ? R : r), 0, 0, 0, 0);
                }
                // Capture Move
                else {
                    //std::cout << "Rook capture move " << squareToCoords(src) << " -> " << squareToCoords(target) << std::endl;
                    encodeMove(src, target, (!side ? R : r), 0, 1, 0, 0);
                }

                pop_bit(attacks, target);
            }
            pop_bit(bitboard, src);
        }


        // Queen
        bitboard = !side ? bitboards[Q] : bitboards[q];
        while(bitboard){
            int src = getLsb(bitboard);
            attacks = getQueenAttacks(src, allPieces) & (!side ? ~allWhites : ~allBlacks);

            while(attacks){
                int target = getLsb(attacks);

                // Quiet move
                if(!get_bit((!side ? allBlacks : allWhites), target)){
                    //std::cout << "Queen quiet move " << squareToCoords(src) << " -> " << squareToCoords(target) << std::endl;
                    encodeMove(src, target, (!side ? Q : q), 0, 0, 0, 0);
                }
                // Capture Move
                else {
                    //std::cout << "Queen capture move " << squareToCoords(src) << " -> " << squareToCoords(target) << std::endl;
                    encodeMove(src, target, (!side ? Q : q), 0, 1, 0, 0);
                }

                pop_bit(attacks, target);
            }
            pop_bit(bitboard, src);
        }
        // King
        bitboard = !side ? bitboards[K] : bitboards[k];
        while(bitboard){
            int src = getLsb(bitboard);
            attacks = kingAttacks[src] & (!side ? ~allWhites : ~allBlacks);

            while(attacks){
                int target = getLsb(attacks);

                // Quiet move
                if(!isSquareAttacked(target, !side, src) && (!get_bit((!side ? allBlacks : allWhites), target))){
                    //std::cout << "King quiet move " << squareToCoords(src) << " -> " << squareToCoords(target) << std::endl;
                    encodeMove(src, target, (!side ? K : k), 0, 0, 0, 0);
                }
                // Capture Move
                else if(!isSquareAttacked(target, !side, src)){
                    //std::cout << "King capture move " << squareToCoords(src) << " -> " << squareToCoords(target) << std::endl;
                    encodeMove(src, target, (!side ? K : k), 0, 1, 0, 0);
                }

                pop_bit(attacks, target);
            }
            pop_bit(bitboard, src);
        }
    }
};