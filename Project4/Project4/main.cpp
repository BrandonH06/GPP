#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <chrono>
#include <unordered_map>
#include <random>

using namespace sf;

enum PieceType
{
    Empty = 0,
    Pawn = 1,
    Knight = 2,
    Bishop = 3,
    Rook = 4,
    Queen = 5,
    King = 6
};
int debug = 0;

int debug2 = 0;

struct move
{
    int from;
    int to;
    float priority = 0.0f;

    move(int f, int t, float p = 0.0f)
    {
        from = f;
        to = t;
        priority = p;
    }

};

struct moveInfo
{
    int depth;
    float eval;
    std::vector<move> moves;
};

struct Piece
{
    int type;

    bool isBlack;

    Piece(int t = 0, bool c = false)
    {
        type = t;
        isBlack = c;
    }
};

//bool compare(const void* a, const void* b)
bool compare(move MoveA, move MoveB)
{
    /*int main()
{
    int n;
    qsort(values, 6, sizeof(Move), compare);
    for (n = 0; n < 6; n++)
        printf("%d ", values[n]);
    return 0;
}*/

    /*move* MoveA = (move*)a;
    move* MoveB = (move*)b;*/

    float t = MoveB.priority - MoveA.priority;

    if (t <= 0.0f)
    {
        return true;
    }
    else
    {
        return false;
    }
}

std::vector<std::vector<uint32_t>> randomHashValues;

std::vector<uint32_t> randomHashValuesCastle;

std::vector<uint32_t> randomHashValuesEP;

uint32_t randomHashValuesIsBlack;

std::vector<int> pawnTable =
{
    0,  0,  0,  0,  0,  0,  0,  0,
    50, 50, 50, 50, 50, 50, 50, 50,
    10, 10, 20, 30, 30, 20, 10, 10,
    5,  5, 10, 25, 25, 10,  5,  5,
    0,  0,  0, 20, 20,  0,  0,  0,
    5, -5, -10,  0,  0,-10, -5,  5,
    5, 10, 10,-20,-20, 10, 10,  5,
    0,  0,  0,  0,  0,  0,  0,  0
};

std::vector<int> knightTable =
{
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50
};

std::vector<int> bishopTable =
{
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -20,-10,-10,-10,-10,-10,-10,-20,
};

std::vector<int> rookTable =
{
    0,  0,  0,  0,  0,  0,  0,  0,
    5, 10, 10, 10, 10, 10, 10,  5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    0,  0,  0,  5,  5,  0,  0,  0
};

std::vector<int> queenTable =
{
    -20,-10,-10, -5, -5,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5,  5,  5,  5,  0,-10,
    -5,  0,  5,  5,  5,  5,  0, -5,
    0,  0,  5,  5,  5,  5,  0, -5,
    -10,  5,  5,  5,  5,  5,  0,-10,
    -10,  0,  5,  0,  0,  0,  0,-10,
    -20,-10,-10, -5, -5,-10,-10,-20
};

std::vector<int> kingTable =
{
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -10,-20,-20,-20,-20,-20,-20,-10,
    20, 20,  0,  0,  0,  0, 20, 20,
    20, 30, 10,  0,  0, 10, 30, 20
};

std::vector<std::vector<int>> pTables = { pawnTable, knightTable, bishopTable, rookTable, queenTable, kingTable };

std::vector<float> pVal = { 0.0f, 1.0f, 3.0f, 3.0f, 5.0f, 9.0f, 1000.0f };


struct Board
{
    std::unordered_map<uint32_t, moveInfo>* transpositionTable;

    std::vector<Piece> Pieces;

    int enPassant = -10;

    bool blackTurn = false;

    bool bKCastle = true;
    bool bQCastle = true;
    bool wKCastle = true;
    bool wQCastle = true;

    int bKing = 4;
    int wKing = 60;

    float blackPieceTable = -95.0f;
    float whitePieceTable = -95.0f;

    float pieceDiffenece = 0.0f;

    uint32_t hash = 0;

    //std::vector<int> pinnedMask;

    void loadPosition(std::string fen)
    {
        //add the move counter stuff for 50 move rule ect

        int temp = 0;
        for (int i = 0; i < fen.length(); i++)
        {
            if (fen[i] == 'r')
            {
                Pieces.emplace_back(Piece(PieceType::Rook, true));
                
            }
            else if (fen[i] == 'n')
            {
                Pieces.emplace_back(Piece(PieceType::Knight, true));
                
            }
            else if (fen[i] == 'b')
            {
                Pieces.emplace_back(Piece(PieceType::Bishop, true));
                
            }
            else if (fen[i] == 'q')
            {
                Pieces.emplace_back(Piece(PieceType::Queen, true));
                
            }
            else if (fen[i] == 'k')
            {
                bKing = Pieces.size();
                Pieces.emplace_back(Piece(PieceType::King, true));
                
            }
            else if (fen[i] == 'p')
            {
                Pieces.emplace_back(Piece(PieceType::Pawn, true));
                
            }
            else if (fen[i] == 'P')
            {
                Pieces.emplace_back(Piece(PieceType::Pawn, false));
                
            }
            else if (fen[i] == 'K')
            {
                wKing = Pieces.size();
                Pieces.emplace_back(Piece(PieceType::King, false));
                
            }
            else if (fen[i] == 'Q')
            {
                Pieces.emplace_back(Piece(PieceType::Queen, false));
                
            }
            else if (fen[i] == 'B')
            {
                Pieces.emplace_back(Piece(PieceType::Bishop, false));
                
            }
            else if (fen[i] == 'N')
            {
                Pieces.emplace_back(Piece(PieceType::Knight, false));
                
            }
            else if (fen[i] == 'R')
            {
                Pieces.emplace_back(Piece(PieceType::Rook, false));
                
            }
            else if (fen[i] == '/')
            {
                //do nothing
            }
            else if (fen[i] == ' ')
            {
                temp = i;
                break;
            }
            else
            {
                int t = fen[i] - '0';

                for (int j = 0; j < t; j++)
                {
                    Pieces.emplace_back(Piece(PieceType::Empty, false));
                }
            }
        }

        for (int i = temp + 1; i < fen.length(); i++)
        {
            if (fen[i] == 'b')
            {
                blackTurn = true;
            }
            else if (fen[i] == 'w')
            {
                blackTurn = false;
            }
            else if (fen[i] == ' ')
            {
                temp = i;
                break;
            }
        }

        wKCastle = false;
        wQCastle = false;
        bKCastle = false;
        bQCastle = false;
        for (int i = temp + 1; i < fen.length(); i++)
        {
            if (fen[i] == 'K')
            {
                wKCastle = true;
            }
            else if (fen[i] == 'Q')
            {
                wQCastle = true;
            }
            else if (fen[i] == 'k')
            {
                bKCastle = true;
            }
            else if (fen[i] == 'q')
            {
                bQCastle = true;
            }
            else if (fen[i] == ' ')
            {
                temp = i;
                break;
            }
            else if (fen[i] == '-')
            {
                temp = i + 1;
                break;
            }
        }

        int epSquare = -10;
        for (int i = temp + 1; i < fen.length(); i++)
        {
            if (fen[i] == ' ' || fen[i] == '-')
            {
                temp = i;
                break;
            }
            else if (fen[i] >= 'a')
            {
                
                epSquare = 8 - (fen[i] - 'a');
            }
            else
            {
                epSquare += 8 * (fen[i] - '1');
            }
        }
        if (epSquare >= 0)
        {
            enPassant = 64 - epSquare;
        }
    }

    /*
    std::vector<int> possibleMoves(int p, bool ignoreKing)
    {
        std::vector<int> moves;

        //max 21 moves per piece
        //max 20 pieces

        if (Pieces[p].type == PieceType::Empty)
        {
            return moves;
        }
        else if (Pieces[p].type == PieceType::Pawn)
        {
            int direction = -1;
            if (Pieces[p].isBlack)
            {
                direction = 1;
            }

            if (!ignoreKing)
            {
                //Moving Forward
                int SquareInfront = p + (8 * direction);
                if (SquareInfront >= 0 && SquareInfront < 64)
                {
                    if (Pieces[SquareInfront].type == PieceType::Empty)
                    {
                        moves.emplace_back(SquareInfront);

                        //if moving to either the first rank or eighth rank

                        int Square2Infront = p + (16 * direction);

                        //Checks 2 squares infront if 1 square infront was clear and if its on either 2nd or 7th rank
                        if (Square2Infront >= 0 && Square2Infront < 64)
                        {
                            if (Pieces[Square2Infront].type == PieceType::Empty && (floor(p / 8) == 1 || floor(p / 8) == 6))
                            {
                                moves.emplace_back(Square2Infront);
                            }
                        }
                    }
                }
            }

            //Attacking diagonally right
            int SquareDiagRight = p + (8 * direction) + 1;
            if (Pieces[SquareDiagRight].type != PieceType::Empty)
            {
                if (SquareDiagRight < 64 && SquareDiagRight >= 0)
                {
                    //Checks infront and 1 square to the right
                    if (Pieces[SquareDiagRight].isBlack != Pieces[p].isBlack)
                    {
                        // limit to edges of the board on ranks 1 and 8
                        if (p % 8 != 7)
                        {
                            moves.emplace_back(SquareDiagRight);

                            //if moving to either the first rank or eighth rank

                        }
                    }
                }
            }

            //Attacking diagonally left
            int SquareDiagLeft = p + (8 * direction) - 1;
            if (Pieces[SquareDiagLeft].type != PieceType::Empty)
            {
                //Checks infront and 1 square to the left
                if (Pieces[SquareDiagLeft].isBlack != Pieces[p].isBlack)
                {
                    if (SquareDiagLeft < 64 && SquareDiagLeft >= 0)
                    {
                        // limit to edges of the board on ranks 1 and 8
                        if (p % 8 != 0)
                        {
                            moves.emplace_back(SquareDiagLeft);

                            //if moving to either the first rank or eighth rank

                        }
                    }
                }
            }

            if (ignoreKing)
            {
                // limit to edges of the board on ranks 1 and 8
                if (p % 8 != 7)
                {
                    moves.emplace_back(SquareDiagRight);
                }
                // limit to edges of the board on ranks 1 and 8
                if (p % 8 != 0)
                {
                    moves.emplace_back(SquareDiagLeft);
                }
            }

            //En passant diagonally right
            if (SquareDiagRight == enPassant)
            {
                // limit to edges of the board on ranks 1 and 8
                if (p % 8 != 7)
                {
                    moves.emplace_back(SquareDiagRight);
                }
            }

            //En passant diagonally left
            if (SquareDiagLeft == enPassant)
            {
                // limit to edges of the board on ranks 1 and 8
                if (p % 8 != 0)
                {
                    moves.emplace_back(SquareDiagLeft);
                }
            }
        }

        else if (Pieces[p].type == PieceType::Knight)
        {
            //2 up 1 left
            int topLeft = p - 17;
            if (topLeft >= 0 && topLeft < 64)
            {
                if (p % 8 != 0)
                {
                    if (!ignoreKing)
                    {
                        if (Pieces[topLeft].type == PieceType::Empty || (Pieces[topLeft].type != Empty && Pieces[topLeft].isBlack != Pieces[p].isBlack))
                        {
                            moves.emplace_back(topLeft);
                        }
                    }
                    else
                    {
                        moves.emplace_back(topLeft);
                    }
                }
            }

            //2 up 1 right
            int topRight = p - 15;
            if (topRight >= 0 && topRight < 64)
            {
                if (p % 8 != 7)
                {
                    if (!ignoreKing)
                    {
                        if (Pieces[topRight].type == PieceType::Empty || (Pieces[topRight].type != Empty && Pieces[topRight].isBlack != Pieces[p].isBlack))
                        {
                            moves.emplace_back(topRight);
                        }
                    }
                    else
                    {
                        moves.emplace_back(topRight);
                    }
                }
            }

            //2 left 1 up
            int leftTop = p - 10;
            if (leftTop >= 0 && leftTop < 64)
            {
                if (p % 8 != 1 && p % 8 != 0)
                {
                    if (!ignoreKing)
                    {
                        if (Pieces[leftTop].type == PieceType::Empty || (Pieces[leftTop].type != Empty && Pieces[leftTop].isBlack != Pieces[p].isBlack))
                        {
                            moves.emplace_back(leftTop);
                        }
                    }
                    else
                    {
                        moves.emplace_back(leftTop);
                    }
                }
            }

            //2 left 1 down
            int leftDown = p + 6;
            if (leftDown >= 0 && leftDown < 64)
            {
                if (p % 8 != 1 && p % 8 != 0)
                {
                    if (!ignoreKing)
                    {
                        if (Pieces[leftDown].type == PieceType::Empty || (Pieces[leftDown].type != Empty && Pieces[leftDown].isBlack != Pieces[p].isBlack))
                        {
                            moves.emplace_back(leftDown);
                        }
                    }
                    else
                    {
                        moves.emplace_back(leftDown);
                    }
                }
            }

            //2 right 1 up
            int rightUp = p - 6;
            if (rightUp >= 0 && rightUp < 64)
            {
                if (p % 8 != 7 && p % 8 != 6)
                {
                    if (!ignoreKing)
                    {
                        if (Pieces[rightUp].type == PieceType::Empty || (Pieces[rightUp].type != Empty && Pieces[rightUp].isBlack != Pieces[p].isBlack))
                        {
                            moves.emplace_back(rightUp);
                        }
                    }
                    else
                    {
                        moves.emplace_back(rightUp);
                    }
                }
            }

            //2 right 1 down
            int rightDown = p + 10;
            if (rightDown >= 0 && rightDown < 64)
            {
                if (p % 8 != 7 && p % 8 != 6)
                {
                    if (!ignoreKing)
                    {
                        if (Pieces[rightDown].type == PieceType::Empty || (Pieces[rightDown].type != Empty && Pieces[rightDown].isBlack != Pieces[p].isBlack))
                        {
                            moves.emplace_back(rightDown);
                        }
                    }
                    else
                    {
                        moves.emplace_back(rightDown);
                    }
                }
            }

            //2 down 1 left
            int downLeft = p + 15;
            if (downLeft >= 0 && downLeft < 64)
            {
                if (p % 8 != 0)
                {
                    if (!ignoreKing)
                    {
                        if (Pieces[downLeft].type == PieceType::Empty || (Pieces[downLeft].type != Empty && Pieces[downLeft].isBlack != Pieces[p].isBlack))
                        {
                            moves.emplace_back(downLeft);
                        }
                    }
                    else
                    {
                        moves.emplace_back(downLeft);
                    }
                }
            }

            //2 down 1 right
            int downRight = p + 17;
            if (downRight >= 0 && downRight < 64)
            {
                if (p % 8 != 7)
                {
                    if (!ignoreKing)
                    {
                        if (Pieces[downRight].type == PieceType::Empty || (Pieces[downRight].type != Empty && Pieces[downRight].isBlack != Pieces[p].isBlack))
                        {
                            moves.emplace_back(downRight);
                        }
                    }
                    else
                    {
                        moves.emplace_back(downRight);
                    }
                }
            }
        }

        else if (Pieces[p].type == PieceType::Bishop)
        {
            int topSquares = int(floor(p / 8) + 1);
            int leftSquares = ((p % 8) + 1);
            int bottomSquares = 8 - topSquares + 1;
            int rightSquares = 8 - leftSquares + 1;

            //Diag top left
            int topLeftSquares = std::min(leftSquares, topSquares);
            for (int i = 1; i < topLeftSquares; i++)
            {
                int topLeft = p - 9 * i;
                if (Pieces[topLeft].type == PieceType::Empty)
                {
                    moves.emplace_back(topLeft);
                }
                else
                {
                    if (Pieces[topLeft].isBlack != Pieces[p].isBlack)
                    {
                        if (ignoreKing && Pieces[topLeft].type == PieceType::King)
                        {
                            moves.emplace_back(topLeft);

                        }
                        else
                        {
                            moves.emplace_back(topLeft);
                            break;
                        }

                    }
                    else
                    {
                        if (ignoreKing)
                        {
                            moves.emplace_back(topLeft);
                        }
                        break;
                    }
                }
            }

            //Diag bottom left
            int bottomLeftSquares = std::min(bottomSquares, leftSquares);
            for (int i = 1; i < bottomLeftSquares; i++)
            {
                int bottomLeft = p + 7 * i;
                if (Pieces[bottomLeft].type == PieceType::Empty)
                {
                    moves.emplace_back(bottomLeft);
                }
                else
                {
                    if (Pieces[bottomLeft].isBlack != Pieces[p].isBlack)
                    {
                        if (ignoreKing && Pieces[bottomLeft].type == PieceType::King)
                        {
                            moves.emplace_back(bottomLeft);

                        }
                        else
                        {
                            moves.emplace_back(bottomLeft);
                            break;
                        }
                    }
                    else
                    {
                        if (ignoreKing)
                        {
                            moves.emplace_back(bottomLeft);
                        }
                        break;
                    }
                }
            }

            //Diag top right
            int topRightSquares = std::min(topSquares, rightSquares);
            for (int i = 1; i < topRightSquares; i++)
            {
                int topRight = p - 7 * i;
                if (Pieces[topRight].type == PieceType::Empty)
                {
                    moves.emplace_back(topRight);
                }
                else
                {
                    if (Pieces[topRight].isBlack != Pieces[p].isBlack)
                    {
                        if (ignoreKing && Pieces[topRight].type == PieceType::King)
                        {
                            moves.emplace_back(topRight);

                        }
                        else
                        {
                            moves.emplace_back(topRight);
                            break;
                        }
                    }
                    else
                    {
                        if (ignoreKing)
                        {
                            moves.emplace_back(topRight);
                        }
                        break;
                    }
                }
            }

            //Diag bottom right
            int bottomRightSquares = std::min(bottomSquares, rightSquares);
            for (int i = 1; i < bottomRightSquares; i++)
            {
                int bottomRight = p + 9 * i;
                if (Pieces[bottomRight].type == PieceType::Empty)
                {
                    moves.emplace_back(bottomRight);
                }
                else
                {
                    if (Pieces[bottomRight].isBlack != Pieces[p].isBlack)
                    {
                        if (ignoreKing && Pieces[bottomRight].type == PieceType::King)
                        {
                            moves.emplace_back(bottomRight);

                        }
                        else
                        {
                            moves.emplace_back(bottomRight);
                            break;
                        }
                    }
                    else
                    {
                        if (ignoreKing)
                        {
                            moves.emplace_back(bottomRight);
                        }
                        break;
                    }
                }
            }
        }

        else if (Pieces[p].type == PieceType::Rook)
        {
            //Left
            int leftSquares = ((p % 8) + 1);
            for (int i = 1; i < leftSquares; i++)
            {
                int left = p - 1 * i;
                if (Pieces[left].type == PieceType::Empty)
                {
                    moves.emplace_back(left);
                }
                else
                {
                    if (Pieces[left].isBlack != Pieces[p].isBlack)
                    {
                        if (ignoreKing && Pieces[left].type == PieceType::King)
                        {
                            moves.emplace_back(left);

                        }
                        else
                        {
                            moves.emplace_back(left);
                            break;
                        }
                    }
                    else
                    {
                        if (ignoreKing)
                        {
                            moves.emplace_back(left);
                        }
                        break;
                    }
                }
            }

            //Top
            int topSquares = int(floor(p / 8) + 1);
            for (int i = 1; i < topSquares; i++)
            {
                int top = p - 8 * i;
                if (Pieces[top].type == PieceType::Empty)
                {
                    moves.emplace_back(top);
                }
                else
                {
                    if (Pieces[top].isBlack != Pieces[p].isBlack)
                    {
                        if (ignoreKing && Pieces[top].type == PieceType::King)
                        {
                            moves.emplace_back(top);

                        }
                        else
                        {
                            moves.emplace_back(top);
                            break;
                        }
                    }
                    else
                    {
                        if (ignoreKing)
                        {
                            moves.emplace_back(top);
                        }
                        break;
                    }
                }
            }

            //Right
            int rightSquares = 8 - leftSquares + 1;
            for (int i = 1; i < rightSquares; i++)
            {
                int right = p + 1 * i;
                if (Pieces[right].type == PieceType::Empty)
                {
                    moves.emplace_back(right);
                }
                else
                {
                    if (Pieces[right].isBlack != Pieces[p].isBlack)
                    {
                        if (ignoreKing && Pieces[right].type == PieceType::King)
                        {
                            moves.emplace_back(right);

                        }
                        else
                        {
                            moves.emplace_back(right);
                            break;
                        }
                    }
                    else
                    {
                        if (ignoreKing)
                        {
                            moves.emplace_back(right);
                        }
                        break;
                    }
                }
            }

            //Bottom
            int bottomSquares = 8 - topSquares + 1;
            for (int i = 1; i < bottomSquares; i++)
            {
                int bottom = p + 8 * i;
                if (Pieces[bottom].type == PieceType::Empty)
                {
                    moves.emplace_back(bottom);
                }
                else
                {
                    if (Pieces[bottom].isBlack != Pieces[p].isBlack)
                    {
                        if (ignoreKing && Pieces[bottom].type == PieceType::King)
                        {
                            moves.emplace_back(bottom);

                        }
                        else
                        {
                            moves.emplace_back(bottom);
                            break;
                        }
                    }
                    else
                    {
                        if (ignoreKing)
                        {
                            moves.emplace_back(bottom);
                        }
                        break;
                    }
                }
            }
        }

        else if (Pieces[p].type == PieceType::Queen)
        {
            //Left
            int leftSquares = ((p % 8) + 1);
            for (int i = 1; i < leftSquares; i++)
            {
                int left = p - 1 * i;
                if (Pieces[left].type == PieceType::Empty)
                {
                    moves.emplace_back(left);
                }
                else
                {
                    if (Pieces[left].isBlack != Pieces[p].isBlack)
                    {
                        if (ignoreKing && Pieces[left].type == PieceType::King)
                        {
                            moves.emplace_back(left);

                        }
                        else
                        {
                            moves.emplace_back(left);
                            break;
                        }
                    }
                    else
                    {
                        if (ignoreKing)
                        {
                            moves.emplace_back(left);
                        }
                        break;
                    }
                }
            }

            //Top
            int topSquares = int(floor(p / 8) + 1);
            for (int i = 1; i < topSquares; i++)
            {
                int top = p - 8 * i;
                if (Pieces[top].type == PieceType::Empty)
                {
                    moves.emplace_back(top);
                }
                else
                {
                    if (Pieces[top].isBlack != Pieces[p].isBlack)
                    {
                        if (ignoreKing && Pieces[top].type == PieceType::King)
                        {
                            moves.emplace_back(top);

                        }
                        else
                        {
                            moves.emplace_back(top);
                            break;
                        }
                    }
                    else
                    {
                        if (ignoreKing)
                        {
                            moves.emplace_back(top);
                        }
                        break;
                    }
                }
            }

            //Right
            int rightSquares = 8 - leftSquares + 1;
            for (int i = 1; i < rightSquares; i++)
            {
                int right = p + 1 * i;
                if (Pieces[right].type == PieceType::Empty)
                {
                    moves.emplace_back(right);
                }
                else
                {
                    if (Pieces[right].isBlack != Pieces[p].isBlack)
                    {
                        if (ignoreKing && Pieces[right].type == PieceType::King)
                        {
                            moves.emplace_back(right);

                        }
                        else
                        {
                            moves.emplace_back(right);
                            break;
                        }
                    }
                    else
                    {
                        if (ignoreKing)
                        {
                            moves.emplace_back(right);
                        }
                        break;
                    }
                }
            }

            //Bottom
            int bottomSquares = 8 - topSquares + 1;
            for (int i = 1; i < bottomSquares; i++)
            {
                int bottom = p + 8 * i;
                if (Pieces[bottom].type == PieceType::Empty)
                {
                    moves.emplace_back(bottom);
                }
                else
                {
                    if (Pieces[bottom].isBlack != Pieces[p].isBlack)
                    {
                        if (ignoreKing && Pieces[bottom].type == PieceType::King)
                        {
                            moves.emplace_back(bottom);

                        }
                        else
                        {
                            moves.emplace_back(bottom);
                            break;
                        }
                    }
                    else
                    {
                        if (ignoreKing)
                        {
                            moves.emplace_back(bottom);
                        }
                        break;
                    }
                }
            }

            //Diag top left
            int topLeftSquares = std::min(leftSquares, topSquares);
            for (int i = 1; i < topLeftSquares; i++)
            {
                int topLeft = p - 9 * i;
                if (Pieces[topLeft].type == PieceType::Empty)
                {
                    moves.emplace_back(topLeft);
                }
                else
                {
                    if (Pieces[topLeft].isBlack != Pieces[p].isBlack)
                    {
                        if (ignoreKing && Pieces[topLeft].type == PieceType::King)
                        {
                            moves.emplace_back(topLeft);

                        }
                        else
                        {
                            moves.emplace_back(topLeft);
                            break;
                        }
                    }
                    else
                    {
                        if (ignoreKing)
                        {
                            moves.emplace_back(topLeft);
                        }
                        break;
                    }
                }
            }

            //Diag bottom left
            int bottomLeftSquares = std::min(bottomSquares, leftSquares);
            for (int i = 1; i < bottomLeftSquares; i++)
            {
                int bottomLeft = p + 7 * i;
                if (Pieces[bottomLeft].type == PieceType::Empty)
                {
                    moves.emplace_back(bottomLeft);
                }
                else
                {
                    if (Pieces[bottomLeft].isBlack != Pieces[p].isBlack)
                    {
                        if (ignoreKing && Pieces[bottomLeft].type == PieceType::King)
                        {
                            moves.emplace_back(bottomLeft);

                        }
                        else
                        {
                            moves.emplace_back(bottomLeft);
                            break;
                        }
                    }
                    else
                    {
                        if (ignoreKing)
                        {
                            moves.emplace_back(bottomLeft);
                        }
                        break;
                    }
                }
            }

            //Diag top right
            int topRightSquares = std::min(topSquares, rightSquares);
            for (int i = 1; i < topRightSquares; i++)
            {
                int topRight = p - 7 * i;
                if (Pieces[topRight].type == PieceType::Empty)
                {
                    moves.emplace_back(topRight);
                }
                else
                {
                    if (Pieces[topRight].isBlack != Pieces[p].isBlack)
                    {
                        if (ignoreKing && Pieces[topRight].type == PieceType::King)
                        {
                            moves.emplace_back(topRight);

                        }
                        else
                        {
                            moves.emplace_back(topRight);
                            break;
                        }
                    }
                    else
                    {
                        if (ignoreKing)
                        {
                            moves.emplace_back(topRight);
                        }
                        break;
                    }
                }
            }

            //Diag bottom right
            int bottomRightSquares = std::min(bottomSquares, rightSquares);
            for (int i = 1; i < bottomRightSquares; i++)
            {
                int bottomRight = p + 9 * i;
                if (Pieces[bottomRight].type == PieceType::Empty)
                {
                    moves.emplace_back(bottomRight);
                }
                else
                {
                    if (Pieces[bottomRight].isBlack != Pieces[p].isBlack)
                    {
                        if (ignoreKing && Pieces[bottomRight].type == PieceType::King)
                        {
                            moves.emplace_back(bottomRight);

                        }
                        else
                        {
                            moves.emplace_back(bottomRight);
                            break;
                        }
                    }
                    else
                    {
                        if (ignoreKing)
                        {
                            moves.emplace_back(bottomRight);
                        }
                        break;
                    }
                }
            }
        }

        else if (Pieces[p].type == PieceType::King)
        {
            if (p % 8 != 0)
            {
                //Left
                int left = p - 1;
                if (Pieces[left].type == PieceType::Empty)
                {
                    moves.emplace_back(left);
                }
                else
                {
                    if (Pieces[left].isBlack != Pieces[p].isBlack || ignoreKing)
                    {
                        moves.emplace_back(left);
                    }
                }

                //Diag top left
                int topLeft = p - 9;
                if (topLeft >= 0)
                {
                    if (Pieces[topLeft].type == PieceType::Empty)
                    {
                        moves.emplace_back(topLeft);
                    }
                    else
                    {
                        if (Pieces[topLeft].isBlack != Pieces[p].isBlack || ignoreKing)
                        {
                            moves.emplace_back(topLeft);
                        }
                    }
                }

                //Diag bottom left
                int bottomLeft = p + 7;
                if (bottomLeft < 64)
                {
                    if (Pieces[bottomLeft].type == PieceType::Empty)
                    {
                        moves.emplace_back(bottomLeft);
                    }
                    else
                    {
                        if (Pieces[bottomLeft].isBlack != Pieces[p].isBlack || ignoreKing)
                        {
                            moves.emplace_back(bottomLeft);
                        }
                    }
                }
            }

            //Top
            int top = p - 8;
            if (top >= 0)
            {
                if (Pieces[top].type == PieceType::Empty)
                {
                    moves.emplace_back(top);
                }
                else
                {
                    if (Pieces[top].isBlack != Pieces[p].isBlack || ignoreKing)
                    {
                        moves.emplace_back(top);
                    }
                }
            }

            //Bottom
            int bottom = p + 8;
            if (bottom < 64)
            {
                if (Pieces[bottom].type == PieceType::Empty)
                {
                    moves.emplace_back(bottom);
                }
                else
                {
                    if (Pieces[bottom].isBlack != Pieces[p].isBlack || ignoreKing)
                    {
                        moves.emplace_back(bottom);
                    }
                }
            }

            if (p % 8 != 7)
            {
                //Right
                int right = p + 1;
                if (Pieces[right].type == PieceType::Empty)
                {
                    moves.emplace_back(right);
                }
                else
                {
                    if (Pieces[right].isBlack != Pieces[p].isBlack || ignoreKing)
                    {
                        moves.emplace_back(right);
                    }
                }

                //Diag top right
                int topRight = p - 7;

                if (topRight >= 0)
                {
                    if (Pieces[topRight].type == PieceType::Empty)
                    {
                        moves.emplace_back(topRight);
                    }
                    else
                    {
                        if (Pieces[topRight].isBlack != Pieces[p].isBlack || ignoreKing)
                        {
                            moves.emplace_back(topRight);
                        }
                    }
                }

                //Diag bottom right
                int bottomRight = p + 9;
                if (bottomRight < 64)
                {
                    if (Pieces[bottomRight].type == PieceType::Empty)
                    {
                        moves.emplace_back(bottomRight);
                    }
                    else
                    {
                        if (Pieces[bottomRight].isBlack != Pieces[p].isBlack || ignoreKing)
                        {
                            moves.emplace_back(bottomRight);
                        }
                    }
                }
            }

            if (!ignoreKing)
            {
                if (Pieces[p].isBlack)
                {
                    if (bKCastle)
                    {
                        if ((Pieces[5].type == PieceType::Empty && Pieces[6].type == PieceType::Empty) && !(inCheck(4, true) || inCheck(5, true) || inCheck(6, true)))
                        {
                            moves.emplace_back(6);
                        }
                    }
                }
                if (Pieces[p].isBlack)
                {
                    if (bQCastle)
                    {
                        if ((Pieces[3].type == PieceType::Empty && Pieces[2].type == PieceType::Empty && Pieces[1].type == PieceType::Empty) && !(inCheck(4, true) || inCheck(3, true) || inCheck(2, true)))
                        {
                            moves.emplace_back(2);
                        }
                    }
                }

                if (!Pieces[p].isBlack)
                {
                    if (wKCastle)
                    {
                        if ((Pieces[61].type == PieceType::Empty && Pieces[62].type == PieceType::Empty) && !(inCheck(60, false) || inCheck(61, false) || inCheck(62, false)))
                        {
                            moves.emplace_back(62);
                        }
                    }
                }

                if (!Pieces[p].isBlack)
                {
                    if (wQCastle)
                    {
                        if ((Pieces[59].type == PieceType::Empty && Pieces[58].type == PieceType::Empty && Pieces[57].type == PieceType::Empty) && !(inCheck(60, false) || inCheck(59, false) || inCheck(58, false)))
                        {
                            moves.emplace_back(58);
                        }
                    }
                }
            }
        }

        return moves;
    }
    */

    std::vector<move> possibleMoves(bool isBlack)
    {
        debug2++;
        std::vector<move> moves;

        moves.reserve(70);

        for (int p = 0; p < Pieces.size(); p++)
        {
            if (Pieces[p].type == PieceType::Empty)
            {
                //Do nothing
            }
            else
            {
                if (Pieces[p].isBlack == isBlack)
                {
                    if (Pieces[p].type == PieceType::Pawn)
                    {
                        int direction = -1;
                        if (Pieces[p].isBlack)
                        {
                            direction = 1;
                        }

                        //Moving Forward
                        int SquareInfront = p + (8 * direction);
                        if (SquareInfront >= 0 && SquareInfront < 64)
                        {
                            if (Pieces[SquareInfront].type == PieceType::Empty)
                            {
                                moves.emplace_back(move(p, SquareInfront, 0));

                                int Square2Infront = p + (16 * direction);
                                //Checks 2 squares infront if 1 square infront was clear and if its on either 2nd or 7th rank
                                if (Square2Infront >= 0 && Square2Infront < 64)
                                {
                                    if (Pieces[Square2Infront].type == PieceType::Empty && (floor(p / 8) == 1 || floor(p / 8) == 6))
                                    {
                                        moves.emplace_back(move(p, Square2Infront, 0));
                                    }
                                }
                            }
                        }

                        //Attacking diagonally right
                        int SquareDiagRight = p + (8 * direction) + 1;
                        if (Pieces[SquareDiagRight].type != PieceType::Empty)
                        {
                            if (SquareDiagRight < 64 && SquareDiagRight >= 0)
                            {
                                //Checks infront and 1 square to the right
                                if (Pieces[SquareDiagRight].isBlack != Pieces[p].isBlack)
                                {
                                    // limit to edges of the board on ranks 1 and 8
                                    if (p % 8 != 7)
                                    {
                                        moves.emplace_back(move(p, SquareDiagRight, pVal[Pieces[SquareDiagRight].type] - (pVal[PieceType::Pawn] / 10)));
                                    }
                                }
                            }
                        }

                        //Attacking diagonally left
                        int SquareDiagLeft = p + (8 * direction) - 1;
                        if (Pieces[SquareDiagLeft].type != PieceType::Empty)
                        {
                            //Checks infront and 1 square to the left
                            if (Pieces[SquareDiagLeft].isBlack != Pieces[p].isBlack)
                            {
                                if (SquareDiagLeft < 64 && SquareDiagLeft >= 0)
                                {
                                    // limit to edges of the board on ranks 1 and 8
                                    if (p % 8 != 0)
                                    {
                                        moves.emplace_back(move(p, SquareDiagLeft, pVal[Pieces[SquareDiagLeft].type] - (pVal[PieceType::Pawn] / 10)));
                                    }
                                }
                            }
                        }

                        //En passant diagonally right
                        if (SquareDiagRight == enPassant)
                        {
                            // limit to edges of the board on ranks 1 and 8
                            if (p % 8 != 7)
                            {
                                moves.emplace_back(move(p, SquareDiagRight, pVal[PieceType::Pawn] - (pVal[PieceType::Pawn] / 10)));
                            }
                        }

                        //En passant diagonally left
                        if (SquareDiagLeft == enPassant)
                        {
                            // limit to edges of the board on ranks 1 and 8
                            if (p % 8 != 0)
                            {
                                moves.emplace_back(move(p, SquareDiagLeft, pVal[PieceType::Pawn] - (pVal[PieceType::Pawn] / 10)));
                            }
                        }
                    }

                    else if (Pieces[p].type == PieceType::Knight)
                    {
                        //2 up 1 left
                        int topLeft = p - 17;
                        if (topLeft >= 0 && topLeft < 64)
                        {
                            if (p % 8 != 0)
                            {
                                if (Pieces[topLeft].type == PieceType::Empty || (Pieces[topLeft].type != Empty && Pieces[topLeft].isBlack != Pieces[p].isBlack))
                                {
                                    moves.emplace_back(move(p, topLeft, pVal[Pieces[topLeft].type] - (pVal[PieceType::Knight] / 10)));
                                }
                            }
                        }

                        //2 up 1 right
                        int topRight = p - 15;
                        if (topRight >= 0 && topRight < 64)
                        {
                            if (p % 8 != 7)
                            {
                                if (Pieces[topRight].type == PieceType::Empty || (Pieces[topRight].type != Empty && Pieces[topRight].isBlack != Pieces[p].isBlack))
                                {
                                    moves.emplace_back(move(p, topRight, pVal[Pieces[topRight].type] - (pVal[PieceType::Knight] / 10)));
                                }
                            }
                        }

                        //2 left 1 up
                        int leftTop = p - 10;
                        if (leftTop >= 0 && leftTop < 64)
                        {
                            if (p % 8 != 1 && p % 8 != 0)
                            {
                                if (Pieces[leftTop].type == PieceType::Empty || (Pieces[leftTop].type != Empty && Pieces[leftTop].isBlack != Pieces[p].isBlack))
                                {
                                    moves.emplace_back(move(p, leftTop, pVal[Pieces[leftTop].type] - (pVal[PieceType::Knight] / 10)));
                                }
                            }
                        }

                        //2 left 1 down
                        int leftDown = p + 6;
                        if (leftDown >= 0 && leftDown < 64)
                        {
                            if (p % 8 != 1 && p % 8 != 0)
                            {
                                if (Pieces[leftDown].type == PieceType::Empty || (Pieces[leftDown].type != Empty && Pieces[leftDown].isBlack != Pieces[p].isBlack))
                                {
                                    moves.emplace_back(move(p, leftDown, pVal[Pieces[leftDown].type] - (pVal[PieceType::Knight] / 10)));
                                }
                            }
                        }

                        //2 right 1 up
                        int rightUp = p - 6;
                        if (rightUp >= 0 && rightUp < 64)
                        {
                            if (p % 8 != 7 && p % 8 != 6)
                            {
                                if (Pieces[rightUp].type == PieceType::Empty || (Pieces[rightUp].type != Empty && Pieces[rightUp].isBlack != Pieces[p].isBlack))
                                {
                                    moves.emplace_back(move(p, rightUp, pVal[Pieces[rightUp].type] - (pVal[PieceType::Knight] / 10)));
                                }
                            }
                        }

                        //2 right 1 down
                        int rightDown = p + 10;
                        if (rightDown >= 0 && rightDown < 64)
                        {
                            if (p % 8 != 7 && p % 8 != 6)
                            {
                                if (Pieces[rightDown].type == PieceType::Empty || (Pieces[rightDown].type != Empty && Pieces[rightDown].isBlack != Pieces[p].isBlack))
                                {
                                    moves.emplace_back(move(p, rightDown, pVal[Pieces[rightDown].type] - (pVal[PieceType::Knight] / 10)));
                                }
                            }
                        }

                        //2 down 1 left
                        int downLeft = p + 15;
                        if (downLeft >= 0 && downLeft < 64)
                        {
                            if (p % 8 != 0)
                            {
                                if (Pieces[downLeft].type == PieceType::Empty || (Pieces[downLeft].type != Empty && Pieces[downLeft].isBlack != Pieces[p].isBlack))
                                {
                                    moves.emplace_back(move(p, downLeft, pVal[Pieces[downLeft].type] - (pVal[PieceType::Knight] / 10)));
                                }
                            }
                        }

                        //2 down 1 right
                        int downRight = p + 17;
                        if (downRight >= 0 && downRight < 64)
                        {
                            if (p % 8 != 7)
                            {
                                if (Pieces[downRight].type == PieceType::Empty || (Pieces[downRight].type != Empty && Pieces[downRight].isBlack != Pieces[p].isBlack))
                                {
                                    moves.emplace_back(move(p, downRight, pVal[Pieces[downRight].type] - (pVal[PieceType::Knight] / 10)));
                                }
                            }
                        }
                    }

                    else if (Pieces[p].type == PieceType::Bishop)
                    {
                        int topSquares = int(floor(p / 8) + 1);
                        int leftSquares = ((p % 8) + 1);
                        int bottomSquares = 8 - topSquares + 1;
                        int rightSquares = 8 - leftSquares + 1;

                        //Diag top left
                        int topLeftSquares = std::min(leftSquares, topSquares);
                        for (int i = 1; i < topLeftSquares; i++)
                        {
                            int topLeft = p - 9 * i;
                            if (Pieces[topLeft].type == PieceType::Empty)
                            {
                                moves.emplace_back(move(p, topLeft, pVal[Pieces[topLeft].type] - (pVal[PieceType::Bishop] / 10)));
                            }
                            else
                            {
                                if (Pieces[topLeft].isBlack != Pieces[p].isBlack)
                                {
                                    moves.emplace_back(move(p, topLeft, pVal[Pieces[topLeft].type] - (pVal[PieceType::Bishop] / 10)));
                                    break;

                                }
                                else
                                {
                                    break;
                                }
                            }
                        }

                        //Diag bottom left
                        int bottomLeftSquares = std::min(bottomSquares, leftSquares);
                        for (int i = 1; i < bottomLeftSquares; i++)
                        {
                            int bottomLeft = p + 7 * i;
                            if (Pieces[bottomLeft].type == PieceType::Empty)
                            {
                                moves.emplace_back(move(p, bottomLeft, pVal[Pieces[bottomLeft].type] - (pVal[PieceType::Bishop] / 10)));
                            }
                            else
                            {
                                if (Pieces[bottomLeft].isBlack != Pieces[p].isBlack)
                                {
                                    moves.emplace_back(move(p, bottomLeft, pVal[Pieces[bottomLeft].type] - (pVal[PieceType::Bishop] / 10)));
                                    break;
                                }
                                else
                                {
                                    break;
                                }
                            }
                        }

                        //Diag top right
                        int topRightSquares = std::min(topSquares, rightSquares);
                        for (int i = 1; i < topRightSquares; i++)
                        {
                            int topRight = p - 7 * i;
                            if (Pieces[topRight].type == PieceType::Empty)
                            {
                                moves.emplace_back(move(p, topRight, pVal[Pieces[topRight].type] - (pVal[PieceType::Bishop] / 10)));
                            }
                            else
                            {
                                if (Pieces[topRight].isBlack != Pieces[p].isBlack)
                                {
                                    moves.emplace_back(move(p, topRight, pVal[Pieces[topRight].type] - (pVal[PieceType::Bishop] / 10)));
                                    break;
                                }
                                else
                                {
                                    break;
                                }
                            }
                        }

                        //Diag bottom right
                        int bottomRightSquares = std::min(bottomSquares, rightSquares);
                        for (int i = 1; i < bottomRightSquares; i++)
                        {
                            int bottomRight = p + 9 * i;
                            if (Pieces[bottomRight].type == PieceType::Empty)
                            {
                                moves.emplace_back(move(p, bottomRight, pVal[Pieces[bottomRight].type] - (pVal[PieceType::Bishop] / 10)));
                            }
                            else
                            {
                                if (Pieces[bottomRight].isBlack != Pieces[p].isBlack)
                                {
                                    moves.emplace_back(move(p, bottomRight, pVal[Pieces[bottomRight].type] - (pVal[PieceType::Bishop] / 10)));
                                    break;
                                }
                                else
                                {
                                    break;
                                }
                            }
                        }
                    }
                    else if (Pieces[p].type == PieceType::Rook)
                    {
                        //Left
                        int leftSquares = ((p % 8) + 1);
                        for (int i = 1; i < leftSquares; i++)
                        {
                            int left = p - 1 * i;
                            if (Pieces[left].type == PieceType::Empty)
                            {
                                moves.emplace_back(move(p, left, pVal[Pieces[left].type] - (pVal[PieceType::Rook] / 10)));
                            }
                            else
                            {
                                if (Pieces[left].isBlack != Pieces[p].isBlack)
                                {
                                    moves.emplace_back(move(p, left, pVal[Pieces[left].type] - (pVal[PieceType::Rook] / 10)));
                                    break;
                                }
                                else
                                {
                                    break;
                                }
                            }
                        }

                        //Top
                        int topSquares = int(floor(p / 8) + 1);
                        for (int i = 1; i < topSquares; i++)
                        {
                            int top = p - 8 * i;
                            if (Pieces[top].type == PieceType::Empty)
                            {
                                moves.emplace_back(move(p, top, pVal[Pieces[top].type] - (pVal[PieceType::Rook] / 10)));
                            }
                            else
                            {
                                if (Pieces[top].isBlack != Pieces[p].isBlack)
                                {
                                    moves.emplace_back(move(p, top, pVal[Pieces[top].type] - (pVal[PieceType::Rook] / 10)));
                                    break;
                                }
                                else
                                {
                                    break;
                                }
                            }
                        }

                        //Right
                        int rightSquares = 8 - leftSquares + 1;
                        for (int i = 1; i < rightSquares; i++)
                        {
                            int right = p + 1 * i;
                            if (Pieces[right].type == PieceType::Empty)
                            {
                                moves.emplace_back(move(p, right, pVal[Pieces[right].type] - (pVal[PieceType::Rook] / 10)));
                            }
                            else
                            {
                                if (Pieces[right].isBlack != Pieces[p].isBlack)
                                {
                                    moves.emplace_back(move(p, right, pVal[Pieces[right].type] - (pVal[PieceType::Rook] / 10)));
                                    break;
                                }
                                else
                                {
                                    break;
                                }
                            }
                        }

                        //Bottom
                        int bottomSquares = 8 - topSquares + 1;
                        for (int i = 1; i < bottomSquares; i++)
                        {
                            int bottom = p + 8 * i;
                            if (Pieces[bottom].type == PieceType::Empty)
                            {
                                moves.emplace_back(move(p, bottom, pVal[Pieces[bottom].type] - (pVal[PieceType::Rook] / 10)));
                            }
                            else
                            {
                                if (Pieces[bottom].isBlack != Pieces[p].isBlack)
                                {
                                    moves.emplace_back(move(p, bottom, pVal[Pieces[bottom].type] - (pVal[PieceType::Rook] / 10)));
                                    break;
                                }
                                else
                                {
                                    break;
                                }
                            }
                        }
                    }
                    else if (Pieces[p].type == PieceType::Queen)
                    {
                        //Left
                        int leftSquares = ((p % 8) + 1);
                        for (int i = 1; i < leftSquares; i++)
                        {
                            int left = p - 1 * i;
                            if (Pieces[left].type == PieceType::Empty)
                            {
                                moves.emplace_back(move(p, left, pVal[Pieces[left].type] - (pVal[PieceType::Queen] / 10)));
                            }
                            else
                            {
                                if (Pieces[left].isBlack != Pieces[p].isBlack)
                                {
                                    moves.emplace_back(move(p, left, pVal[Pieces[left].type] - (pVal[PieceType::Queen] / 10)));
                                    break;
                                }
                                else
                                {
                                    break;
                                }
                            }
                        }

                        //Top
                        int topSquares = int(floor(p / 8) + 1);
                        for (int i = 1; i < topSquares; i++)
                        {
                            int top = p - 8 * i;
                            if (Pieces[top].type == PieceType::Empty)
                            {
                                moves.emplace_back(move(p, top, pVal[Pieces[top].type] - (pVal[PieceType::Queen] / 10)));
                            }
                            else
                            {
                                if (Pieces[top].isBlack != Pieces[p].isBlack)
                                {
                                    moves.emplace_back(move(p, top, pVal[Pieces[top].type] - (pVal[PieceType::Queen] / 10)));
                                    break;
                                }
                                else
                                {
                                    break;
                                }
                            }
                        }

                        //Right
                        int rightSquares = 8 - leftSquares + 1;
                        for (int i = 1; i < rightSquares; i++)
                        {
                            int right = p + 1 * i;
                            if (Pieces[right].type == PieceType::Empty)
                            {
                                moves.emplace_back(move(p, right, pVal[Pieces[right].type] - (pVal[PieceType::Queen] / 10)));
                            }
                            else
                            {
                                if (Pieces[right].isBlack != Pieces[p].isBlack)
                                {
                                    moves.emplace_back(move(p, right, pVal[Pieces[right].type] - (pVal[PieceType::Queen] / 10)));
                                    break;
                                }
                                else
                                {
                                    break;
                                }
                            }
                        }

                        //Bottom
                        int bottomSquares = 8 - topSquares + 1;
                        for (int i = 1; i < bottomSquares; i++)
                        {
                            int bottom = p + 8 * i;
                            if (Pieces[bottom].type == PieceType::Empty)
                            {
                                moves.emplace_back(move(p, bottom, pVal[Pieces[bottom].type] - (pVal[PieceType::Queen] / 10)));
                            }
                            else
                            {
                                if (Pieces[bottom].isBlack != Pieces[p].isBlack)
                                {
                                    moves.emplace_back(move(p, bottom, pVal[Pieces[bottom].type] - (pVal[PieceType::Queen] / 10)));
                                    break;
                                }
                                else
                                {
                                    break;
                                }
                            }
                        }

                        //Diag top left
                        int topLeftSquares = std::min(leftSquares, topSquares);
                        for (int i = 1; i < topLeftSquares; i++)
                        {
                            int topLeft = p - 9 * i;
                            if (Pieces[topLeft].type == PieceType::Empty)
                            {
                                moves.emplace_back(move(p, topLeft, pVal[Pieces[topLeft].type] - (pVal[PieceType::Queen] / 10)));
                            }
                            else
                            {
                                if (Pieces[topLeft].isBlack != Pieces[p].isBlack)
                                {
                                    moves.emplace_back(move(p, topLeft, pVal[Pieces[topLeft].type] - (pVal[PieceType::Queen] / 10)));
                                    break;

                                }
                                else
                                {
                                    break;
                                }
                            }
                        }

                        //Diag bottom left
                        int bottomLeftSquares = std::min(bottomSquares, leftSquares);
                        for (int i = 1; i < bottomLeftSquares; i++)
                        {
                            int bottomLeft = p + 7 * i;
                            if (Pieces[bottomLeft].type == PieceType::Empty)
                            {
                                moves.emplace_back(move(p, bottomLeft, pVal[Pieces[bottomLeft].type] - (pVal[PieceType::Queen] / 10)));
                            }
                            else
                            {
                                if (Pieces[bottomLeft].isBlack != Pieces[p].isBlack)
                                {
                                    moves.emplace_back(move(p, bottomLeft, pVal[Pieces[bottomLeft].type] - (pVal[PieceType::Queen] / 10)));
                                    break;
                                }
                                else
                                {
                                    break;
                                }
                            }
                        }

                        //Diag top right
                        int topRightSquares = std::min(topSquares, rightSquares);
                        for (int i = 1; i < topRightSquares; i++)
                        {
                            int topRight = p - 7 * i;
                            if (Pieces[topRight].type == PieceType::Empty)
                            {
                                moves.emplace_back(move(p, topRight, pVal[Pieces[topRight].type] - (pVal[PieceType::Queen] / 10)));
                            }
                            else
                            {
                                if (Pieces[topRight].isBlack != Pieces[p].isBlack)
                                {
                                    moves.emplace_back(move(p, topRight, pVal[Pieces[topRight].type] - (pVal[PieceType::Queen] / 10)));
                                    break;
                                }
                                else
                                {
                                    break;
                                }
                            }
                        }

                        //Diag bottom right
                        int bottomRightSquares = std::min(bottomSquares, rightSquares);
                        for (int i = 1; i < bottomRightSquares; i++)
                        {
                            int bottomRight = p + 9 * i;
                            if (Pieces[bottomRight].type == PieceType::Empty)
                            {
                                moves.emplace_back(move(p, bottomRight, pVal[Pieces[bottomRight].type] - (pVal[PieceType::Queen] / 10)));
                            }
                            else
                            {
                                if (Pieces[bottomRight].isBlack != Pieces[p].isBlack)
                                {
                                    moves.emplace_back(move(p, bottomRight, pVal[Pieces[bottomRight].type] - (pVal[PieceType::Queen] / 10)));
                                    break;
                                }
                                else
                                {
                                    break;
                                }
                            }
                        }
                    }

                    else if (Pieces[p].type == PieceType::King)
                    {
                        if (p % 8 != 0)
                        {
                            //Left
                            int left = p - 1;
                            if (Pieces[left].type == PieceType::Empty)
                            {
                                moves.emplace_back(move(p, left, pVal[Pieces[left].type] - (pVal[PieceType::Queen] / 10)));
                            }
                            else
                            {
                                if (Pieces[left].isBlack != Pieces[p].isBlack)
                                {
                                    moves.emplace_back(move(p, left, pVal[Pieces[left].type] - (pVal[PieceType::Queen] / 10)));
                                }
                            }

                            //Diag top left
                            int topLeft = p - 9;
                            if (topLeft >= 0)
                            {
                                if (Pieces[topLeft].type == PieceType::Empty)
                                {
                                    moves.emplace_back(move(p, topLeft, pVal[Pieces[topLeft].type] - (pVal[PieceType::Queen] / 10)));
                                }
                                else
                                {
                                    if (Pieces[topLeft].isBlack != Pieces[p].isBlack)
                                    {
                                        moves.emplace_back(move(p, topLeft, pVal[Pieces[topLeft].type] - (pVal[PieceType::Queen] / 10)));
                                    }
                                }
                            }

                            //Diag bottom left
                            int bottomLeft = p + 7;
                            if (bottomLeft < 64)
                            {
                                if (Pieces[bottomLeft].type == PieceType::Empty)
                                {
                                    moves.emplace_back(move(p, bottomLeft, pVal[Pieces[bottomLeft].type] - (pVal[PieceType::Queen] / 10)));
                                }
                                else
                                {
                                    if (Pieces[bottomLeft].isBlack != Pieces[p].isBlack)
                                    {
                                        moves.emplace_back(move(p, bottomLeft, pVal[Pieces[bottomLeft].type] - (pVal[PieceType::Queen] / 10)));
                                    }
                                }
                            }
                        }

                        //Top
                        int top = p - 8;
                        if (top >= 0)
                        {
                            if (Pieces[top].type == PieceType::Empty)
                            {
                                moves.emplace_back(move(p, top, pVal[Pieces[top].type] - (pVal[PieceType::Queen] / 10)));
                            }
                            else
                            {
                                if (Pieces[top].isBlack != Pieces[p].isBlack)
                                {
                                    moves.emplace_back(move(p, top, pVal[Pieces[top].type] - (pVal[PieceType::Queen] / 10)));
                                }
                            }
                        }

                        //Bottom
                        int bottom = p + 8;
                        if (bottom < 64)
                        {
                            if (Pieces[bottom].type == PieceType::Empty)
                            {
                                moves.emplace_back(move(p, bottom, pVal[Pieces[bottom].type] - (pVal[PieceType::Queen] / 10)));
                            }
                            else
                            {
                                if (Pieces[bottom].isBlack != Pieces[p].isBlack)
                                {
                                    moves.emplace_back(move(p, bottom, pVal[Pieces[bottom].type] - (pVal[PieceType::Queen] / 10)));
                                }
                            }
                        }

                        if (p % 8 != 7)
                        {
                            //Right
                            int right = p + 1;
                            if (Pieces[right].type == PieceType::Empty)
                            {
                                moves.emplace_back(move(p, right, pVal[Pieces[right].type] - (pVal[PieceType::Queen] / 10)));
                            }
                            else
                            {
                                if (Pieces[right].isBlack != Pieces[p].isBlack)
                                {
                                    moves.emplace_back(move(p, right, pVal[Pieces[right].type] - (pVal[PieceType::Queen] / 10)));
                                }
                            }

                            //Diag top right
                            int topRight = p - 7;

                            if (topRight >= 0)
                            {
                                if (Pieces[topRight].type == PieceType::Empty)
                                {
                                    moves.emplace_back(move(p, topRight, pVal[Pieces[topRight].type] - (pVal[PieceType::Queen] / 10)));
                                }
                                else
                                {
                                    if (Pieces[topRight].isBlack != Pieces[p].isBlack)
                                    {
                                        moves.emplace_back(move(p, topRight, pVal[Pieces[topRight].type] - (pVal[PieceType::Queen] / 10)));
                                    }
                                }
                            }

                            //Diag bottom right
                            int bottomRight = p + 9;
                            if (bottomRight < 64)
                            {
                                if (Pieces[bottomRight].type == PieceType::Empty)
                                {
                                    moves.emplace_back(move(p, bottomRight, pVal[Pieces[bottomRight].type] - (pVal[PieceType::Queen] / 10)));
                                }
                                else
                                {
                                    if (Pieces[bottomRight].isBlack != Pieces[p].isBlack)
                                    {
                                        moves.emplace_back(move(p, bottomRight, pVal[Pieces[bottomRight].type] - (pVal[PieceType::Queen] / 10)));
                                    }
                                }
                            }
                        }

                        if (Pieces[p].isBlack)
                        {
                            if (bKCastle)
                            {
                                if ((Pieces[5].type == PieceType::Empty && Pieces[6].type == PieceType::Empty) && !(inCheck(4, true) || inCheck(5, true) || inCheck(6, true)))
                                {
                                    moves.emplace_back(move(p, 6, 0));
                                }
                            }
                        }
                        if (Pieces[p].isBlack)
                        {
                            if (bQCastle)
                            {
                                if ((Pieces[3].type == PieceType::Empty && Pieces[2].type == PieceType::Empty && Pieces[1].type == PieceType::Empty) && !(inCheck(4, true) || inCheck(3, true) || inCheck(2, true)))
                                {
                                    moves.emplace_back(move(p, 2, 0));
                                }
                            }
                        }

                        if (!Pieces[p].isBlack)
                        {
                            if (wKCastle)
                            {
                                if ((Pieces[61].type == PieceType::Empty && Pieces[62].type == PieceType::Empty) && !(inCheck(60, false) || inCheck(61, false) || inCheck(62, false)))
                                {
                                    moves.emplace_back(move(p, 62, 0));
                                }
                            }
                        }

                        if (!Pieces[p].isBlack)
                        {
                            if (wQCastle)
                            {
                                if ((Pieces[59].type == PieceType::Empty && Pieces[58].type == PieceType::Empty && Pieces[57].type == PieceType::Empty) && !(inCheck(60, false) || inCheck(59, false) || inCheck(58, false)))
                                {
                                    moves.emplace_back(move(p, 58, 0));
                                }
                            }
                        }
                    }
                }
            }            
        }
        return moves;
    }

    std::vector<bool> genAttackMap(bool isBlack)
    {
        std::vector<bool> attackMap(64, false);

        for (int p = 0; p < Pieces.size(); p++)
        {
            if (Pieces[p].type == PieceType::Empty)
            {
                //Do nothing
            }
            else
            {
                if (Pieces[p].isBlack == isBlack)
                {
                    if (Pieces[p].type == PieceType::Pawn)
                    {
                        int direction = -1;
                        if (Pieces[p].isBlack)
                        {
                            direction = 1;
                        }

                        int SquareDiagRight = p + (8 * direction) + 1;
                        if (p % 8 != 7)
                        {
                            attackMap[SquareDiagRight] = true;
                        }

                        int SquareDiagLeft = p + (8 * direction) - 1;
                        if (p % 8 != 0)
                        {
                            attackMap[SquareDiagLeft] = true;
                        }

                    }

                    else if (Pieces[p].type == PieceType::Knight)
                    {
                        //2 up 1 left
                        int topLeft = p - 17;
                        if (topLeft >= 0 && topLeft < 64)
                        {
                            if (p % 8 != 0)
                            {
                                attackMap[topLeft] = true;
                            }
                        }

                        //2 up 1 right
                        int topRight = p - 15;
                        if (topRight >= 0 && topRight < 64)
                        {
                            if (p % 8 != 7)
                            {
                                attackMap[topRight] = true;
                            }
                        }

                        //2 left 1 up
                        int leftTop = p - 10;
                        if (leftTop >= 0 && leftTop < 64)
                        {
                            //if (p % 8 != 1 && p % 8 != 0)
                            if (p % 8 > 1)
                            {
                                attackMap[leftTop] = true;
                            }
                        }

                        //2 left 1 down
                        int leftDown = p + 6;
                        if (leftDown >= 0 && leftDown < 64)
                        {
                            if (p % 8 > 1)
                            {
                                attackMap[leftDown] = true;
                            }
                        }

                        //2 right 1 up
                        int rightUp = p - 6;
                        if (rightUp >= 0 && rightUp < 64)
                        {
                            if (p % 8 < 6)
                            {
                                attackMap[rightUp] = true;
                            }
                        }

                        //2 right 1 down
                        int rightDown = p + 10;
                        if (rightDown >= 0 && rightDown < 64)
                        {
                            if (p % 8 < 6)
                            {
                                attackMap[rightDown] = true;
                            }
                        }

                        //2 down 1 left
                        int downLeft = p + 15;
                        if (downLeft >= 0 && downLeft < 64)
                        {
                            if (p % 8 != 0)
                            {
                                attackMap[downLeft] = true;
                            }
                        }

                        //2 down 1 right
                        int downRight = p + 17;
                        if (downRight >= 0 && downRight < 64)
                        {
                            if (p % 8 != 7)
                            {
                                attackMap[downRight] = true;
                            }
                        }
                    }

                    else if (Pieces[p].type == PieceType::Bishop)
                    {
                        int topSquares = int(floor(p / 8) + 1);
                        int leftSquares = ((p % 8) + 1);
                        int bottomSquares = 8 - topSquares + 1;
                        int rightSquares = 8 - leftSquares + 1;

                        //Diag top left
                        int topLeftSquares = std::min(leftSquares, topSquares);
                        for (int i = 1; i < topLeftSquares; i++)
                        {
                            int topLeft = p - 9 * i;

                            if (Pieces[topLeft].type == PieceType::Empty)
                            {
                                attackMap[topLeft] = true;
                            }
                            else
                            {
                                if (Pieces[topLeft].isBlack != Pieces[p].isBlack)
                                {

                                    attackMap[topLeft] = true;

                                    if (Pieces[topLeft].type != PieceType::King)
                                    {
                                        break;
                                    }

                                }
                                else
                                {
                                    attackMap[topLeft] = true;
                                    break;
                                }
                            }
                        }

                        //Diag bottom left
                        int bottomLeftSquares = std::min(bottomSquares, leftSquares);
                        for (int i = 1; i < bottomLeftSquares; i++)
                        {
                            int bottomLeft = p + 7 * i;

                            if (Pieces[bottomLeft].type == PieceType::Empty)
                            {
                                attackMap[bottomLeft] = true;
                            }
                            else
                            {
                                if (Pieces[bottomLeft].isBlack != Pieces[p].isBlack)
                                {

                                    attackMap[bottomLeft] = true;

                                    if (Pieces[bottomLeft].type != PieceType::King)
                                    {
                                        break;
                                    }

                                }
                                else
                                {
                                    attackMap[bottomLeft] = true;
                                    break;
                                }
                            }
                        }

                        //Diag top right
                        int topRightSquares = std::min(topSquares, rightSquares);
                        for (int i = 1; i < topRightSquares; i++)
                        {
                            int topRight = p - 7 * i;
                            
                            if (Pieces[topRight].type == PieceType::Empty)
                            {
                                attackMap[topRight] = true;
                            }
                            else
                            {
                                if (Pieces[topRight].isBlack != Pieces[p].isBlack)
                                {

                                    attackMap[topRight] = true;

                                    if (Pieces[topRight].type != PieceType::King)
                                    {
                                        break;
                                    }

                                }
                                else
                                {
                                    attackMap[topRight] = true;
                                    break;
                                }
                            }
                        }

                        //Diag bottom right
                        int bottomRightSquares = std::min(bottomSquares, rightSquares);
                        for (int i = 1; i < bottomRightSquares; i++)
                        {
                            int bottomRight = p + 9 * i;
                            
                            if (Pieces[bottomRight].type == PieceType::Empty)
                            {
                                attackMap[bottomRight] = true;
                            }
                            else
                            {
                                if (Pieces[bottomRight].isBlack != Pieces[p].isBlack)
                                {

                                    attackMap[bottomRight] = true;

                                    if (Pieces[bottomRight].type != PieceType::King)
                                    {
                                        break;
                                    }
                                }
                                else
                                {
                                    attackMap[bottomRight] = true;
                                    break;
                                }
                            }
                        }
                    }

                    else if (Pieces[p].type == PieceType::Rook)
                    {
                        //Left
                        int leftSquares = ((p % 8) + 1);
                        for (int i = 1; i < leftSquares; i++)
                        {
                            int left = p - 1 * i;

                            if (Pieces[left].type == PieceType::Empty)
                            {
                                attackMap[left] = true;
                            }
                            else
                            {
                                if (Pieces[left].isBlack != Pieces[p].isBlack)
                                {
                                    attackMap[left] = true;

                                    if (Pieces[left].type != PieceType::King)
                                    {
                                        break;
                                    }
                                }
                                else
                                {
                                    attackMap[left] = true;
                                    break;
                                }
                            }
                        }

                        //Top
                        int topSquares = int(floor(p / 8) + 1);
                        for (int i = 1; i < topSquares; i++)
                        {
                            int top = p - 8 * i;
                            
                            if (Pieces[top].type == PieceType::Empty)
                            {
                                attackMap[top] = true;
                            }
                            else
                            {
                                if (Pieces[top].isBlack != Pieces[p].isBlack)
                                {
                                    attackMap[top] = true;

                                    if (Pieces[top].type != PieceType::King)
                                    {
                                        break;
                                    }
                                }
                                else
                                {
                                    attackMap[top] = true;
                                    break;
                                }
                            }
                        }

                        //Right
                        int rightSquares = 8 - leftSquares + 1;
                        for (int i = 1; i < rightSquares; i++)
                        {
                            int right = p + 1 * i;
                            
                            if (Pieces[right].type == PieceType::Empty)
                            {
                                attackMap[right] = true;
                            }
                            else
                            {
                                if (Pieces[right].isBlack != Pieces[p].isBlack)
                                {
                                    attackMap[right] = true;

                                    if (Pieces[right].type != PieceType::King)
                                    {
                                        break;
                                    }
                                }
                                else
                                {
                                    attackMap[right] = true;
                                    break;
                                }
                            }
                        }

                        //Bottom
                        int bottomSquares = 8 - topSquares + 1;
                        for (int i = 1; i < bottomSquares; i++)
                        {
                            int bottom = p + 8 * i;
                            
                            if (Pieces[bottom].type == PieceType::Empty)
                            {
                                attackMap[bottom] = true;
                            }
                            else
                            {
                                if (Pieces[bottom].isBlack != Pieces[p].isBlack)
                                {
                                    attackMap[bottom] = true;

                                    if (Pieces[bottom].type != PieceType::King)
                                    {
                                        break;
                                    }
                                }
                                else
                                {
                                    attackMap[bottom] = true;
                                    break;
                                }
                            }
                        }
                    }

                    else if (Pieces[p].type == PieceType::Queen)
                    {
                        //Left
                        int leftSquares = ((p % 8) + 1);
                        for (int i = 1; i < leftSquares; i++)
                        {
                            int left = p - 1 * i;

                            if (Pieces[left].type == PieceType::Empty)
                            {
                                attackMap[left] = true;
                            }
                            else
                            {
                                if (Pieces[left].isBlack != Pieces[p].isBlack)
                                {
                                    attackMap[left] = true;

                                    if (Pieces[left].type != PieceType::King)
                                    {
                                        break;
                                    }
                                }
                                else
                                {
                                    attackMap[left] = true;
                                    break;
                                }
                            }
                        }

                        //Top
                        int topSquares = int(floor(p / 8) + 1);
                        for (int i = 1; i < topSquares; i++)
                        {
                            int top = p - 8 * i;

                            if (Pieces[top].type == PieceType::Empty)
                            {
                                attackMap[top] = true;
                            }
                            else
                            {
                                if (Pieces[top].isBlack != Pieces[p].isBlack)
                                {
                                    attackMap[top] = true;

                                    if (Pieces[top].type != PieceType::King)
                                    {
                                        break;
                                    }
                                }
                                else
                                {
                                    attackMap[top] = true;
                                    break;
                                }
                            }
                        }

                        //Right
                        int rightSquares = 8 - leftSquares + 1;
                        for (int i = 1; i < rightSquares; i++)
                        {
                            int right = p + 1 * i;

                            if (Pieces[right].type == PieceType::Empty)
                            {
                                attackMap[right] = true;
                            }
                            else
                            {
                                if (Pieces[right].isBlack != Pieces[p].isBlack)
                                {
                                    attackMap[right] = true;

                                    if (Pieces[right].type != PieceType::King)
                                    {
                                        break;
                                    }
                                }
                                else
                                {
                                    attackMap[right] = true;
                                    break;
                                }
                            }
                        }

                        //Bottom
                        int bottomSquares = 8 - topSquares + 1;
                        for (int i = 1; i < bottomSquares; i++)
                        {
                            int bottom = p + 8 * i;

                            if (Pieces[bottom].type == PieceType::Empty)
                            {
                                attackMap[bottom] = true;
                            }
                            else
                            {
                                if (Pieces[bottom].isBlack != Pieces[p].isBlack)
                                {
                                    attackMap[bottom] = true;

                                    if (Pieces[bottom].type != PieceType::King)
                                    {
                                        break;
                                    }
                                }
                                else
                                {
                                    attackMap[bottom] = true;
                                    break;
                                }
                            }
                        }

                        //Diag top left
                        int topLeftSquares = std::min(leftSquares, topSquares);
                        for (int i = 1; i < topLeftSquares; i++)
                        {
                            int topLeft = p - 9 * i;

                            if (Pieces[topLeft].type == PieceType::Empty)
                            {
                                attackMap[topLeft] = true;
                            }
                            else
                            {
                                if (Pieces[topLeft].isBlack != Pieces[p].isBlack)
                                {

                                    attackMap[topLeft] = true;

                                    if (Pieces[topLeft].type != PieceType::King)
                                    {
                                        break;
                                    }

                                }
                                else
                                {
                                    attackMap[topLeft] = true;
                                    break;
                                }
                            }
                        }

                        //Diag bottom left
                        int bottomLeftSquares = std::min(bottomSquares, leftSquares);
                        for (int i = 1; i < bottomLeftSquares; i++)
                        {
                            int bottomLeft = p + 7 * i;

                            if (Pieces[bottomLeft].type == PieceType::Empty)
                            {
                                attackMap[bottomLeft] = true;
                            }
                            else
                            {
                                if (Pieces[bottomLeft].isBlack != Pieces[p].isBlack)
                                {

                                    attackMap[bottomLeft] = true;

                                    if (Pieces[bottomLeft].type != PieceType::King)
                                    {
                                        break;
                                    }

                                }
                                else
                                {
                                    attackMap[bottomLeft] = true;
                                    break;
                                }
                            }
                        }

                        //Diag top right
                        int topRightSquares = std::min(topSquares, rightSquares);
                        for (int i = 1; i < topRightSquares; i++)
                        {
                            int topRight = p - 7 * i;

                            if (Pieces[topRight].type == PieceType::Empty)
                            {
                                attackMap[topRight] = true;
                            }
                            else
                            {
                                if (Pieces[topRight].isBlack != Pieces[p].isBlack)
                                {

                                    attackMap[topRight] = true;

                                    if (Pieces[topRight].type != PieceType::King)
                                    {
                                        break;
                                    }

                                }
                                else
                                {
                                    attackMap[topRight] = true;
                                    break;
                                }
                            }
                        }

                        //Diag bottom right
                        int bottomRightSquares = std::min(bottomSquares, rightSquares);
                        for (int i = 1; i < bottomRightSquares; i++)
                        {
                            int bottomRight = p + 9 * i;

                            if (Pieces[bottomRight].type == PieceType::Empty)
                            {
                                attackMap[bottomRight] = true;
                            }
                            else
                            {
                                if (Pieces[bottomRight].isBlack != Pieces[p].isBlack)
                                {

                                    attackMap[bottomRight] = true;

                                    if (Pieces[bottomRight].type != PieceType::King)
                                    {
                                        break;
                                    }
                                }
                                else
                                {
                                    attackMap[bottomRight] = true;
                                    break;
                                }
                            }
                        }
                    }

                    else if (Pieces[p].type == PieceType::King)
                    {
                        if (p % 8 != 0)
                        {
                            //Left
                            int left = p - 1;

                            attackMap[left] = true;

                            //Diag top left
                            int topLeft = p - 9;
                            if (topLeft >= 0)
                            {
                                attackMap[topLeft] = true;
                            }

                            //Diag bottom left
                            int bottomLeft = p + 7;
                            if (bottomLeft < 64)
                            {
                                attackMap[bottomLeft] = true;
                            }
                        }

                        //Top
                        int top = p - 8;
                        if (top >= 0)
                        {
                            attackMap[top] = true;
                        }

                        //Bottom
                        int bottom = p + 8;
                        if (bottom < 64)
                        {
                            attackMap[bottom] = true;
                        }

                        if (p % 8 != 7)
                        {
                            //Right
                            int right = p + 1;
                            attackMap[right] = true;

                            //Diag top right
                            int topRight = p - 7;

                            if (topRight >= 0)
                            {
                                attackMap[topRight] = true;
                            }

                            //Diag bottom right
                            int bottomRight = p + 9;
                            if (bottomRight < 64)
                            {
                                attackMap[bottomRight] = true;
                            }
                        }
                    }
                }
            }
        }

        /*std::vector<move> temp = possibleMoves(isBlack, true);

        for (int i = 0; i < temp.size(); i++)
        {
            attackMap[temp[i].to] = true;
        }*/

        return attackMap;
    }

    std::vector<bool> genCheckMap(bool isBlack, bool& doubleCheck)
    {
        int checkCount = 0;
        std::vector<bool> CheckMap(64, 0);

        int p;
        if (isBlack)
        {
            p = bKing;
        }
        else
        {
            p = wKing;
        }

        std::vector<int> tempSquares;
        bool hitOurPiece = false;

        //Left
        int leftSquares = ((p % 8) + 1);
        for (int i = 1; i < leftSquares; i++)
        {
            int left = p - 1 * i;
            if (Pieces[left].type == PieceType::Empty)
            {
                tempSquares.emplace_back(left);
            }
            else
            {
                if (Pieces[left].isBlack != Pieces[p].isBlack)
                {
                    //hit enemy piece after seeing 1 of our pieces
                    if (Pieces[left].type == PieceType::Rook || Pieces[left].type == PieceType::Queen)
                    {
                        tempSquares.emplace_back(left);
                        //updates all squares that have been checked to the checkMap
                        //std::cout << "checked from left" << std::endl;
                        checkCount++;
                        for (int i = 0; i < tempSquares.size(); i++)
                        {
                            CheckMap[tempSquares[i]] = true;
                        }
                    }
                    break;
                }
                else
                {
                    break;
                }
            }
        }
        tempSquares.clear();

        //Top
        int topSquares = int(floor(p / 8) + 1);
        for (int i = 1; i < topSquares; i++)
        {
            int top = p - 8 * i;
            if (Pieces[top].type == PieceType::Empty)
            {
                tempSquares.emplace_back(top);
            }
            else
            {
                if (Pieces[top].isBlack != Pieces[p].isBlack)
                {
                    //hit enemy piece after seeing 1 of our pieces
                    if (Pieces[top].type == PieceType::Rook || Pieces[top].type == PieceType::Queen)
                    {
                        tempSquares.emplace_back(top);
                        //updates all squares that have been checked to the checkMap
                        //std::cout << "checked from top" << std::endl;
                        checkCount++;
                        for (int i = 0; i < tempSquares.size(); i++)
                        {
                            CheckMap[tempSquares[i]] = true;
                        }
                    }
                    break;
                }
                else
                {
                    break;
                }
            }
        }
        tempSquares.clear();

        //Right
        int rightSquares = 8 - leftSquares + 1;
        for (int i = 1; i < rightSquares; i++)
        {
            int right = p + 1 * i;
            if (Pieces[right].type == PieceType::Empty)
            {
                tempSquares.emplace_back(right);
            }
            else
            {
                if (Pieces[right].isBlack != Pieces[p].isBlack)
                {
                    //hit enemy piece after seeing 1 of our pieces
                    if (Pieces[right].type == PieceType::Rook || Pieces[right].type == PieceType::Queen)
                    {
                        tempSquares.emplace_back(right);
                        //updates all squares that have been checked to the checkMap
                        //std::cout << "checked from right " << right << std::endl;
                        checkCount++;
                        for (int i = 0; i < tempSquares.size(); i++)
                        {
                            CheckMap[tempSquares[i]] = true;
                        }
                    }
                    break;
                }
                else
                {
                    break;
                }
            }
        }
        tempSquares.clear();

        //Bottom
        int bottomSquares = 8 - topSquares + 1;
        for (int i = 1; i < bottomSquares; i++)
        {
            int bottom = p + 8 * i;
            if (Pieces[bottom].type == PieceType::Empty)
            {
                tempSquares.emplace_back(bottom);
            }
            else
            {
                if (Pieces[bottom].isBlack != Pieces[p].isBlack)
                {
                    //hit enemy piece after seeing 1 of our pieces
                    if (Pieces[bottom].type == PieceType::Rook || Pieces[bottom].type == PieceType::Queen)
                    {
                        tempSquares.emplace_back(bottom);
                        //updates all squares that have been checked to the checkMap
                        //std::cout << "checked from bottom" << std::endl;
                        checkCount++;
                        for (int i = 0; i < tempSquares.size(); i++)
                        {
                            CheckMap[tempSquares[i]] = true;
                        }
                    }
                    break;
                }
                else
                {
                    break;
                }
            }
        }
        tempSquares.clear();

        //Diag top left
        int topLeftSquares = std::min(leftSquares, topSquares);
        for (int i = 1; i < topLeftSquares; i++)
        {
            int topLeft = p - 9 * i;
            if (Pieces[topLeft].type == PieceType::Empty)
            {
                tempSquares.emplace_back(topLeft);
            }
            else
            {
                if (Pieces[topLeft].isBlack != Pieces[p].isBlack)
                {
                    //hit enemy piece after seeing 1 of our pieces
                    if (Pieces[topLeft].type == PieceType::Bishop || Pieces[topLeft].type == PieceType::Queen)
                    {
                        tempSquares.emplace_back(topLeft);
                        //updates all squares that have been checked to the checkMap
                        //std::cout << "checked from top left" << std::endl;
                        checkCount++;
                        for (int i = 0; i < tempSquares.size(); i++)
                        {
                            CheckMap[tempSquares[i]] = true;
                        }
                    }
                    break;
                }
                else
                {
                    break;
                }
            }
        }
        tempSquares.clear();

        //Diag bottom left
        int bottomLeftSquares = std::min(bottomSquares, leftSquares);
        for (int i = 1; i < bottomLeftSquares; i++)
        {
            int bottomLeft = p + 7 * i;
            if (Pieces[bottomLeft].type == PieceType::Empty)
            {
                tempSquares.emplace_back(bottomLeft);
            }
            else
            {
                if (Pieces[bottomLeft].isBlack != Pieces[p].isBlack)
                {
                    //hit enemy piece after seeing 1 of our pieces
                    if (Pieces[bottomLeft].type == PieceType::Bishop || Pieces[bottomLeft].type == PieceType::Queen)
                    {
                        tempSquares.emplace_back(bottomLeft);
                        //updates all squares that have been checked to the checkMap
                        //std::cout << "checked from bottom left" << std::endl;
                        checkCount++;
                        for (int i = 0; i < tempSquares.size(); i++)
                        {
                            CheckMap[tempSquares[i]] = true;
                        }
                    }
                    break;
                }
                else
                {
                    break;
                }
            }
        }
        tempSquares.clear();

        //Diag top right
        int topRightSquares = std::min(topSquares, rightSquares);
        for (int i = 1; i < topRightSquares; i++)
        {
            int topRight = p - 7 * i;
            if (Pieces[topRight].type == PieceType::Empty)
            {
                tempSquares.emplace_back(topRight);
            }
            else
            {
                if (Pieces[topRight].isBlack != Pieces[p].isBlack)
                {
                    //hit enemy piece after seeing 1 of our pieces
                    if (Pieces[topRight].type == PieceType::Bishop || Pieces[topRight].type == PieceType::Queen)
                    {
                        tempSquares.emplace_back(topRight);
                        //updates all squares that have been checked to the checkMap
                        //std::cout << "checked from top right" << std::endl;
                        checkCount++;
                        for (int i = 0; i < tempSquares.size(); i++)
                        {
                            CheckMap[tempSquares[i]] = true;
                        }
                    }
                    break;
                }
                else
                {
                    break;
                }
            }
        }
        tempSquares.clear();

        //Diag bottom right
        int bottomRightSquares = std::min(bottomSquares, rightSquares);
        for (int i = 1; i < bottomRightSquares; i++)
        {
            int bottomRight = p + 9 * i;
            if (Pieces[bottomRight].type == PieceType::Empty)
            {
                tempSquares.emplace_back(bottomRight);
            }
            else
            {
                if (Pieces[bottomRight].isBlack != Pieces[p].isBlack)
                {
                    //hit enemy piece after seeing 1 of our pieces
                    if (Pieces[bottomRight].type == PieceType::Bishop || Pieces[bottomRight].type == PieceType::Queen)
                    {
                        tempSquares.emplace_back(bottomRight);
                        //updates all squares that have been checked to the checkMap
                        //std::cout << "checked from bottom right" << std::endl;
                        checkCount++;
                        for (int i = 0; i < tempSquares.size(); i++)
                        {
                            CheckMap[tempSquares[i]] = true;
                        }
                    }
                    break;
                }
                else
                {
                    break;
                }
            }
        }

        int direction = -1;
        if (isBlack)
        {
            direction = 1;
        }

        //Attacking diagonally right
        int SquareDiagRight = p + (8 * direction) + 1;
        if (Pieces[SquareDiagRight].type != PieceType::Empty)
        {
            if (SquareDiagRight < 64 && SquareDiagRight >= 0)
            {
                //Checks infront and 1 square to the right
                if (Pieces[SquareDiagRight].isBlack != Pieces[p].isBlack)
                {
                    // limit to edges of the board on ranks 1 and 8
                    if (p % 8 != 7)
                    {
                        if (Pieces[SquareDiagRight].type == PieceType::Pawn)
                        {
                            checkCount++;
                            CheckMap[SquareDiagRight] = true;

                            int SquareInfront = SquareDiagRight + (8 * direction);
                            if (SquareInfront == enPassant)
                            {
                                CheckMap[SquareInfront] = true;
                            }
                        }
                    }
                }
            }
        }

        //Attacking diagonally left
        int SquareDiagLeft = p + (8 * direction) - 1;
        if (Pieces[SquareDiagLeft].type != PieceType::Empty)
        {
            //Checks infront and 1 square to the left
            if (Pieces[SquareDiagLeft].isBlack != Pieces[p].isBlack)
            {
                if (SquareDiagLeft < 64 && SquareDiagLeft >= 0)
                {
                    // limit to edges of the board on ranks 1 and 8
                    if (p % 8 != 0)
                    {
                        if (Pieces[SquareDiagLeft].type == PieceType::Pawn)
                        {
                            checkCount++;
                            CheckMap[SquareDiagLeft] = true;

                            int SquareInfront = SquareDiagLeft + (8 * direction);
                            if (SquareInfront == enPassant)
                            {
                                CheckMap[SquareInfront] = true;
                            }
                        }
                    }
                }
            }
        }

        //2 up 1 left
        int topLeft = p - 17;
        if (topLeft >= 0 && topLeft < 64)
        {
            if (p % 8 != 0)
            {
                if (Pieces[topLeft].type == PieceType::Knight && Pieces[topLeft].isBlack != Pieces[p].isBlack)
                {
                    checkCount++;
                    CheckMap[topLeft] = true;
                }
            }
        }

        //2 up 1 right
        int topRight = p - 15;
        if (topRight >= 0 && topRight < 64)
        {
            if (p % 8 != 7)
            {
                if (Pieces[topRight].type == PieceType::Knight && Pieces[topRight].isBlack != Pieces[p].isBlack)
                {
                    checkCount++;
                    CheckMap[topRight] = true;
                }
            }
        }

        //2 left 1 up
        int leftTop = p - 10;
        if (leftTop >= 0 && leftTop < 64)
        {
            if (p % 8 != 1 && p % 8 != 0)
            {
                if (Pieces[leftTop].type == PieceType::Knight && Pieces[leftTop].isBlack != Pieces[p].isBlack)
                {
                    checkCount++;
                    CheckMap[leftTop] = true;
                }
            }
        }

        //2 left 1 down
        int leftDown = p + 6;
        if (leftDown >= 0 && leftDown < 64)
        {
            if (p % 8 != 1 && p % 8 != 0)
            {
                if (Pieces[leftDown].type == PieceType::Knight && Pieces[leftDown].isBlack != Pieces[p].isBlack)
                {
                    checkCount++;
                    CheckMap[leftDown] = true;
                }
            }
        }

        //2 right 1 up
        int rightUp = p - 6;
        if (rightUp >= 0 && rightUp < 64)
        {
            if (p % 8 != 7 && p % 8 != 6)
            {
                if (Pieces[rightUp].type == PieceType::Knight && Pieces[rightUp].isBlack != Pieces[p].isBlack)
                {
                    checkCount++;
                    CheckMap[rightUp] = true;
                }
            }
        }

        //2 right 1 down
        int rightDown = p + 10;
        if (rightDown >= 0 && rightDown < 64)
        {
            if (p % 8 != 7 && p % 8 != 6)
            {
                if (Pieces[rightDown].type == PieceType::Knight && Pieces[rightDown].isBlack != Pieces[p].isBlack)
                {
                    checkCount++;
                    CheckMap[rightDown] = true;
                }
            }
        }

        //2 down 1 left
        int downLeft = p + 15;
        if (downLeft >= 0 && downLeft < 64)
        {
            if (p % 8 != 0)
            {
                if (Pieces[downLeft].type == PieceType::Knight && Pieces[downLeft].isBlack != Pieces[p].isBlack)
                {
                    checkCount++;
                    CheckMap[downLeft] = true;
                }
            }
        }

        //2 down 1 right
        int downRight = p + 17;
        if (downRight >= 0 && downRight < 64)
        {
            if (p % 8 != 7)
            {
                if (Pieces[downRight].type == PieceType::Knight && Pieces[downRight].isBlack != Pieces[p].isBlack)
                {
                    checkCount++;
                    CheckMap[downRight] = true;
                }
            }
        }

        if (checkCount > 1)
        {
            doubleCheck = true;
        }
        else
        {
            doubleCheck = false;
        }

        return CheckMap;
    }

    bool inCheck(int p, bool isBlack)
    {
        bool checked = false;

        std::vector<bool> temp = genAttackMap(!isBlack);

        if (temp[p])
        {
            checked = true;
        }
        return checked;
    }

    std::vector<int> genPinMap(bool isBlack)
    {
        std::vector<int> PinMap(64, 0);

        int p;
        if (isBlack)
        {
            p = bKing;
        }
        else
        {
            p = wKing;
        }

        std::vector<int> tempSquares;
        tempSquares.reserve(8);
        bool hitOurPiece = false;

        //Left
        int leftSquares = ((p % 8) + 1);
        hitOurPiece = false;

        int enPassantSquare = 0;
        bool hitEnPassantPawn = false;
        bool hitOurPawn = false;
        bool useEnPassant = false;
        for (int i = 1; i < leftSquares; i++)
        {
            int left = p - 1 * i;
            if (Pieces[left].type == PieceType::Empty)
            {
                tempSquares.emplace_back(left);
                hitEnPassantPawn = false;
                hitOurPawn = false;
            }
            else
            {
                if (Pieces[left].isBlack != Pieces[p].isBlack)
                {
                    if (Pieces[left].type == PieceType::Pawn && abs(left - enPassant) == 8)
                    {
                        if (!hitOurPawn)
                        {
                            hitEnPassantPawn = true;
                        }
                        else
                        {
                            useEnPassant = true;
                        }
                    }
                    else
                    {
                        //hit enemy piece after seeing 1 of our pieces
                        if (hitOurPiece && (Pieces[left].type == PieceType::Rook || Pieces[left].type == PieceType::Queen))
                        {
                            if (useEnPassant)
                            {
                                PinMap[enPassantSquare] = 9;
                            }
                            else
                            {
                                tempSquares.emplace_back(left);
                                //updates all squares that have been checked to the pinmap
                                for (int i = 0; i < tempSquares.size(); i++)
                                {
                                    PinMap[tempSquares[i]] = 1;
                                }
                            }
                        }
                        break;
                    }
                }
                else
                {
                    if (Pieces[left].type == PieceType::Pawn && !hitEnPassantPawn)
                    {
                        enPassantSquare = left;
                        hitOurPawn = true;
                    }
                    else if (Pieces[left].type == PieceType::Pawn && hitEnPassantPawn)
                    {
                        enPassantSquare = left;
                        useEnPassant = true;
                    }
                    else
                    {
                        hitOurPawn = false;
                    }

                    if (!hitOurPiece)
                    {
                        //passed through first of our pieces
                        tempSquares.emplace_back(left);
                        hitOurPiece = true;
                    }
                    else
                    {
                        //passed through 2 of our pieces
                        break;
                    }
                }
            }
        }
        tempSquares.clear();

        //Top
        int topSquares = int(floor(p / 8) + 1);
        hitOurPiece = false;
        for (int i = 1; i < topSquares; i++)
        {
            int top = p - 8 * i;
            if (Pieces[top].type == PieceType::Empty)
            {
                tempSquares.emplace_back(top);
            }
            else
            {
                if (Pieces[top].isBlack != Pieces[p].isBlack)
                {
                    //hit enemy piece after seeing 1 of our pieces
                    if (hitOurPiece && (Pieces[top].type == PieceType::Rook || Pieces[top].type == PieceType::Queen))
                    {
                        tempSquares.emplace_back(top);
                        //updates all squares that have been checked to the pinmap
                        for (int i = 0; i < tempSquares.size(); i++)
                        {
                            PinMap[tempSquares[i]] = 2;
                        }
                    }
                    break;
                }
                else
                {
                    if (!hitOurPiece)
                    {
                        //passed through first of our pieces
                        tempSquares.emplace_back(top);
                        hitOurPiece = true;
                    }
                    else
                    {
                        //passed through 2 of our pieces
                        break;
                    }
                }
            }
        }
        tempSquares.clear();

        //Right

        int rightSquares = 8 - leftSquares + 1;
        hitOurPiece = false;

        enPassantSquare = 0;
        hitEnPassantPawn = false;
        hitOurPawn = false;
        useEnPassant = false;
        for (int i = 1; i < rightSquares; i++)
        {
            int right = p + 1 * i;
            if (Pieces[right].type == PieceType::Empty)
            {
                tempSquares.emplace_back(right);
                hitEnPassantPawn = false;
                hitOurPawn = false;
            }
            else
            {
                if (Pieces[right].isBlack != Pieces[p].isBlack)
                {
                    if (Pieces[right].type == PieceType::Pawn && abs(right - enPassant) == 8)
                    {
                        if (!hitOurPawn)
                        {
                            hitEnPassantPawn = true;
                        }
                        else
                        {
                            useEnPassant = true;
                        }
                    }
                    else
                    {
                        //hit enemy piece after seeing 1 of our pieces
                        if (hitOurPiece && (Pieces[right].type == PieceType::Rook || Pieces[right].type == PieceType::Queen))
                        {
                            if (useEnPassant)
                            {
                                PinMap[enPassantSquare] = 9;
                            }
                            else
                            {
                                tempSquares.emplace_back(right);
                                //updates all squares that have been checked to the pinmap
                                for (int i = 0; i < tempSquares.size(); i++)
                                {
                                    PinMap[tempSquares[i]] = 1;
                                }
                            }
                        }
                        break;
                    }
                }
                else
                {
                    if (Pieces[right].type == PieceType::Pawn && !hitEnPassantPawn)
                    {
                        enPassantSquare = right;
                        hitOurPawn = true;
                    }
                    else if (Pieces[right].type == PieceType::Pawn && hitEnPassantPawn)
                    {
                        enPassantSquare = right;
                        useEnPassant = true;
                    }
                    else
                    {
                        hitOurPawn = false;
                    }

                    if (!hitOurPiece)
                    {
                        //passed through first of our pieces
                        tempSquares.emplace_back(right);
                        hitOurPiece = true;
                    }
                    else
                    {
                        //passed through 2 of our pieces
                        break;
                    }
                }
            }
        }


        /*int rightSquares = 8 - leftSquares + 1;
        hitOurPiece = false;

        enPassantSquare = 0;
        hitEnPassantPawn = false;
        hitOurPawn = false;
        useEnPassant = false;
        for (int i = 1; i < rightSquares; i++)
        {
            int right = p + 1 * i;
            if (Pieces[right].type == PieceType::Empty)
            {
                hitEnPassantPawn = false;
                hitOurPawn = false;
                tempSquares.emplace_back(right);
            }
            else
            {
                if (Pieces[right].isBlack != Pieces[p].isBlack)
                {
                    if (Pieces[right].type == PieceType::Pawn && abs(right - enPassant) == 8)
                    {
                        if (!hitOurPawn)
                        {
                            hitEnPassantPawn = true;
                        }
                        else
                        {
                            useEnPassant = true;
                        }
                    }
                    else
                    {
                        //hit enemy piece after seeing 1 of our pieces
                        if (hitOurPiece && (Pieces[right].type == PieceType::Rook || Pieces[right].type == PieceType::Queen))
                        {
                            tempSquares.emplace_back(right);
                            //updates all squares that have been checked to the pinmap
                            for (int i = 0; i < tempSquares.size(); i++)
                            {
                                PinMap[tempSquares[i]] = 3;
                            }
                        }
                        break;
                    }
                }
                else
                {
                    if (Pieces[right].type == PieceType::Pawn && !hitEnPassantPawn)
                    {
                        enPassantSquare = right;
                        hitOurPawn = true;
                    }
                    else if (Pieces[right].type == PieceType::Pawn && hitEnPassantPawn)
                    {
                        enPassantSquare = right;
                        useEnPassant = true;
                    }
                    else
                    {
                        hitOurPawn = false;
                    }

                    if (!hitOurPiece)
                    {
                        //passed through first of our pieces
                        tempSquares.emplace_back(right);
                        hitOurPiece = true;
                    }
                    else
                    {
                        //passed through 2 of our pieces
                        break;
                    }
                }
            }
        }*/
        tempSquares.clear();

        //Bottom
        int bottomSquares = 8 - topSquares + 1;
        hitOurPiece = false;
        for (int i = 1; i < bottomSquares; i++)
        {
            int bottom = p + 8 * i;
            if (Pieces[bottom].type == PieceType::Empty)
            {
                tempSquares.emplace_back(bottom);
            }
            else
            {
                if (Pieces[bottom].isBlack != Pieces[p].isBlack)
                {
                    //hit enemy piece after seeing 1 of our pieces
                    if (hitOurPiece && (Pieces[bottom].type == PieceType::Rook || Pieces[bottom].type == PieceType::Queen))
                    {
                        tempSquares.emplace_back(bottom);
                        //updates all squares that have been checked to the pinmap
                        for (int i = 0; i < tempSquares.size(); i++)
                        {
                            PinMap[tempSquares[i]] = 4;
                        }
                    }
                    break;
                }
                else
                {
                    if (!hitOurPiece)
                    {
                        //passed through first of our pieces
                        tempSquares.emplace_back(bottom);
                        hitOurPiece = true;
                    }
                    else
                    {
                        //passed through 2 of our pieces
                        break;
                    }
                }
            }
        }
        tempSquares.clear();

        //Diag top left
        int topLeftSquares = std::min(leftSquares, topSquares);
        hitOurPiece = false;
        for (int i = 1; i < topLeftSquares; i++)
        {
            int topLeft = p - 9 * i;
            if (Pieces[topLeft].type == PieceType::Empty)
            {
                tempSquares.emplace_back(topLeft);
            }
            else
            {
                if (Pieces[topLeft].isBlack != Pieces[p].isBlack)
                {
                    //hit enemy piece after seeing 1 of our pieces
                    if (hitOurPiece && (Pieces[topLeft].type == PieceType::Bishop || Pieces[topLeft].type == PieceType::Queen))
                    {
                        tempSquares.emplace_back(topLeft);
                        //updates all squares that have been checked to the pinmap
                        for (int i = 0; i < tempSquares.size(); i++)
                        {
                            PinMap[tempSquares[i]] = 5;
                        }
                    }
                    break;
                }
                else
                {
                    if (!hitOurPiece)
                    {
                        //passed through first of our pieces
                        tempSquares.emplace_back(topLeft);
                        hitOurPiece = true;
                    }
                    else
                    {
                        //passed through 2 of our pieces
                        break;
                    }
                }
            }
        }
        tempSquares.clear();

        //Diag bottom left
        int bottomLeftSquares = std::min(bottomSquares, leftSquares);
        hitOurPiece = false;
        for (int i = 1; i < bottomLeftSquares; i++)
        {
            int bottomLeft = p + 7 * i;
            if (Pieces[bottomLeft].type == PieceType::Empty)
            {
                tempSquares.emplace_back(bottomLeft);
            }
            else
            {
                if (Pieces[bottomLeft].isBlack != Pieces[p].isBlack)
                {
                    //hit enemy piece after seeing 1 of our pieces
                    if (hitOurPiece && (Pieces[bottomLeft].type == PieceType::Bishop || Pieces[bottomLeft].type == PieceType::Queen))
                    {
                        tempSquares.emplace_back(bottomLeft);
                        //updates all squares that have been checked to the pinmap
                        for (int i = 0; i < tempSquares.size(); i++)
                        {
                            PinMap[tempSquares[i]] = 6;
                        }
                    }
                    break;
                }
                else
                {
                    if (!hitOurPiece)
                    {
                        //passed through first of our pieces
                        tempSquares.emplace_back(bottomLeft);
                        hitOurPiece = true;
                    }
                    else
                    {
                        //passed through 2 of our pieces
                        break;
                    }
                }
            }
        }
        tempSquares.clear();

        //Diag top right
        int topRightSquares = std::min(topSquares, rightSquares);
        hitOurPiece = false;
        for (int i = 1; i < topRightSquares; i++)
        {
            int topRight = p - 7 * i;
            if (Pieces[topRight].type == PieceType::Empty)
            {
                tempSquares.emplace_back(topRight);
            }
            else
            {
                if (Pieces[topRight].isBlack != Pieces[p].isBlack)
                {
                    //hit enemy piece after seeing 1 of our pieces
                    if (hitOurPiece && (Pieces[topRight].type == PieceType::Bishop || Pieces[topRight].type == PieceType::Queen))
                    {
                        tempSquares.emplace_back(topRight);
                        //updates all squares that have been checked to the pinmap
                        for (int i = 0; i < tempSquares.size(); i++)
                        {
                            PinMap[tempSquares[i]] = 7;
                        }
                    }
                    break;
                }
                else
                {
                    if (!hitOurPiece)
                    {
                        //passed through first of our pieces
                        tempSquares.emplace_back(topRight);
                        hitOurPiece = true;
                    }
                    else
                    {
                        //passed through 2 of our pieces
                        break;
                    }
                }
            }
        }
        tempSquares.clear();

        //Diag bottom right
        int bottomRightSquares = std::min(bottomSquares, rightSquares);
        hitOurPiece = false;
        for (int i = 1; i < bottomRightSquares; i++)
        {
            int bottomRight = p + 9 * i;
            if (Pieces[bottomRight].type == PieceType::Empty)
            {
                tempSquares.emplace_back(bottomRight);
            }
            else
            {
                if (Pieces[bottomRight].isBlack != Pieces[p].isBlack)
                {
                    //hit enemy piece after seeing 1 of our pieces
                    if (hitOurPiece && (Pieces[bottomRight].type == PieceType::Bishop || Pieces[bottomRight].type == PieceType::Queen))
                    {
                        tempSquares.emplace_back(bottomRight);
                        //updates all squares that have been checked to the pinmap
                        for (int i = 0; i < tempSquares.size(); i++)
                        {
                            PinMap[tempSquares[i]] = 8;
                        }
                    }
                    break;
                }
                else
                {
                    if (!hitOurPiece)
                    {
                        //passed through first of our pieces
                        tempSquares.emplace_back(bottomRight);
                        hitOurPiece = true;
                    }
                    else
                    {
                        //passed through 2 of our pieces
                        break;
                    }
                }
            }
        }
        return PinMap;
    }

    float evaluate()
    {
        float pieceTableMult = 0.02f;

        float eval = 0.0f;

        eval += pieceDiffenece;

        if (!blackTurn)
        {
            eval += (blackPieceTable * pieceTableMult * -1);
            
        }
        else
        {
            eval += whitePieceTable * pieceTableMult;
        }

        return eval;
    }

    void initHash()
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> randomNums(0, UINT32_MAX);

        //0 == white king side   1 == white queen side  2 = black king side 3 = black queen side
        for (int i = 0; i < 4; i++)
        {
            randomHashValuesCastle.emplace_back(randomNums(gen));
            hash ^= randomHashValuesCastle[i];
        }

        for (int i = 0; i < 8; i++)
        {
            randomHashValuesEP.emplace_back(randomNums(gen));
        }

        randomHashValuesIsBlack = randomNums(gen);

        for (int i = 0; i < Pieces.size(); i++)
        {
            std::vector<uint32_t> temp;
            for (int j = 0; j < 12; j++)
            {
                uint32_t num = randomNums(gen);
                temp.emplace_back(num);
            }
            if (Pieces[i].type != PieceType::Empty)
            {
                int t = Pieces[i].type - 1;
                if (Pieces[i].isBlack)
                {
                    t += 6;
                }
                hash ^= temp[t];
            }
            randomHashValues.emplace_back(temp);
        }
    }

    std::vector<move> LegalMoves(bool isBlack)
    {
        debug = 0;
        debug2 = 0;
        std::vector<move> moves;
        moves.reserve(50);

        bool checked = false;
        bool doubleChecked = false;

        std::vector<bool> checkMap;

        if (isBlack)
        {
            checked = inCheck(bKing, true);
        }
        else
        {
            checked = inCheck(wKing, false);
        }

        std::vector<int> pin = genPinMap(isBlack);

        std::vector<move> posMoves = possibleMoves(isBlack);

        for (int i = 0; i < posMoves.size(); i++)
        {
            if (doubleChecked)
            {
                if (Pieces[posMoves[i].from].type == PieceType::King)
                {
                    std::vector<bool> attack = genAttackMap(!isBlack);

                    if (!attack[posMoves[i].to])
                    {
                        moves.emplace_back(posMoves[i]);
                    }
                }
            }
            else if (checked)
            {
                if (Pieces[posMoves[i].from].type == PieceType::King)
                {
                    std::vector<bool> attack = genAttackMap(!isBlack);

                    if (!attack[posMoves[i].to])
                    {
                        moves.emplace_back(posMoves[i]);
                    }
                }
                else
                {
                    checkMap = genCheckMap(isBlack, doubleChecked);

                    if (pin[posMoves[i].from] == 0) // if not pinned
                    {
                        //moving between king and the checking piece or taking checking piece
                        if (checkMap[posMoves[i].to] == true)
                        {
                            if (posMoves[i].to == enPassant)
                            {
                                if (Pieces[posMoves[i].from].type == PieceType::Pawn)
                                {
                                    moves.emplace_back(posMoves[i]);
                                    // PROBPAOSKDPAOWDKQPWOEKQPWOEK QWFIX FIX FIX WHY AM I DUMB IT DOESNT ALLOW NONE PAWNS TO BLOCK CHECK ON THE ENPASSANT SQUARE
                                    //MAYBE NOT?
                                }
                            }
                            else
                            {
                                if (Pieces[posMoves[i].from].type == PieceType::Pawn && (posMoves[i].to >= 56 || posMoves[i].to <= 7))
                                {
                                    int direction = -1;
                                    if (Pieces[i].isBlack)
                                    {
                                        direction = 1;
                                    }

                                    moves.emplace_back(move(posMoves[i].from, posMoves[i].to + (8 * direction * 1), 0));
                                    moves.emplace_back(move(posMoves[i].from, posMoves[i].to + (8 * direction * 2), 0));
                                    moves.emplace_back(move(posMoves[i].from, posMoves[i].to + (8 * direction * 3), 0));
                                }
                                moves.emplace_back(posMoves[i]);
                            }
                        }
                    }
                }
            }
            else
            {
                if (Pieces[posMoves[i].from].type == PieceType::King)
                {
                    std::vector<bool> attack = genAttackMap(!isBlack);

                    if (!attack[posMoves[i].to])
                    {
                        moves.emplace_back(posMoves[i]);
                    }
                }
                else
                {
                    
                    if (pin[posMoves[i].from] == 0)
                    {
                        if (Pieces[posMoves[i].from].type == PieceType::Pawn && (posMoves[i].to >= 56 || posMoves[i].to <= 7))
                        {
                            int direction = -1;
                            if (Pieces[i].isBlack)
                            {
                                direction = 1;
                            }

                            moves.emplace_back(move(posMoves[i].from, posMoves[i].to + (8 * direction * 1), 0));
                            moves.emplace_back(move(posMoves[i].from, posMoves[i].to + (8 * direction * 2), 0));
                            moves.emplace_back(move(posMoves[i].from, posMoves[i].to + (8 * direction * 3), 0));
                        }
                        moves.emplace_back(posMoves[i]);
                    }
                    else if (pin[posMoves[i].from] != 0)
                    {
                        //special case for en passant
                        if (pin[posMoves[i].from] == 9)
                        {
                            if (posMoves[i].to != enPassant)
                            {
                                moves.emplace_back(posMoves[i]);
                            }
                        }
                        if (pin[posMoves[i].from] == pin[posMoves[i].to])
                        {
                            moves.emplace_back(posMoves[i]);
                        }
                    }
                }
            }
        }
        return moves;
    }

    void Move(int from, int to)
    {
        /*std::cout << "moved from " << from << " to " << to << std::endl;
        std::cout << "black " << blackPieceTable << std::endl;
        std::cout << "white " << whitePieceTable << std::endl;*/

        if (Pieces[from].isBlack)
        {
            blackPieceTable -= pTables[Pieces[from].type - 1][63 - from];
        }
        else
        {
            whitePieceTable -= pTables[Pieces[from].type - 1][from];
        }

        

        //white promoting
        if (Pieces[from].type == PieceType::Pawn &&  to < 8)
        {
            if (to >= 0)
            {
                hash ^= randomHashValues[from][0];
                hash ^= randomHashValues[from][4];
                pieceDiffenece += pVal[PieceType::Queen] - pVal[PieceType::Pawn];
                Pieces[from] = Piece(PieceType::Queen, Pieces[from].isBlack);
                to = (to % 8);
            }
            else if (to >= -9)
            {
                hash ^= randomHashValues[from][0];
                hash ^= randomHashValues[from][1];
                pieceDiffenece += pVal[PieceType::Knight] - pVal[PieceType::Pawn];
                Pieces[from] = Piece(PieceType::Knight, Pieces[from].isBlack);
                to = ((to % 8) + 8) % 8;
            }
            else if (to >= -17)
            {
                hash ^= randomHashValues[from][0];
                hash ^= randomHashValues[from][3];
                pieceDiffenece += pVal[PieceType::Rook] - pVal[PieceType::Pawn];
                Pieces[from] = Piece(PieceType::Rook, Pieces[from].isBlack);
                to = ((to % 8) + 8) % 8;
            }
            else if (to >= -25)
            {
                hash ^= randomHashValues[from][0];
                hash ^= randomHashValues[from][2];
                pieceDiffenece += pVal[PieceType::Bishop] - pVal[PieceType::Pawn];
                Pieces[from] = Piece(PieceType::Bishop, Pieces[from].isBlack);
                to = ((to % 8) + 8) % 8;
            }
        }

        //black promoting
        if (Pieces[from].type == PieceType::Pawn && to > 55)
        {
            
            if (to <= 63)
            {
                hash ^= randomHashValues[from][6];
                hash ^= randomHashValues[from][10];
                pieceDiffenece -= pVal[PieceType::Queen] - pVal[PieceType::Pawn];
                Pieces[from] = Piece(PieceType::Queen, Pieces[from].isBlack);
            }
            else if (to <= 71)
            {
                hash ^= randomHashValues[from][6];
                hash ^= randomHashValues[from][7];
                pieceDiffenece -= pVal[PieceType::Knight] - pVal[PieceType::Pawn];
                Pieces[from] = Piece(PieceType::Knight, Pieces[from].isBlack);
            }
            else if (to <= 79)
            {
                hash ^= randomHashValues[from][6];
                hash ^= randomHashValues[from][9];
                pieceDiffenece -= pVal[PieceType::Rook] - pVal[PieceType::Pawn];
                Pieces[from] = Piece(PieceType::Rook, Pieces[from].isBlack);
            }
            else if (to <= 87)
            {
                hash ^= randomHashValues[from][6];
                hash ^= randomHashValues[from][8];
                pieceDiffenece -= pVal[PieceType::Bishop] - pVal[PieceType::Pawn];
                Pieces[from] = Piece(PieceType::Bishop, Pieces[from].isBlack);
            }
            to = (to % 8) + 56;
        }

        if (Pieces[to].type != PieceType::Empty)
        {
            if (Pieces[to].isBlack)
            {
                pieceDiffenece += pVal[Pieces[to].type];
                blackPieceTable -= pTables[Pieces[from].type - 1][63 - from];
            }
            else
            {
                pieceDiffenece -= pVal[Pieces[to].type];
                whitePieceTable -= pTables[Pieces[from].type - 1][from];
            }
        }

        //Move rook if castling
        if (from == 60 && to == 62 && Pieces[from].type == PieceType::King)
        {
            hash ^= randomHashValues[63][3];
            hash ^= randomHashValues[61][3];
            Pieces[61] = Pieces[63];
            Pieces[63] = Piece();
        }
        if (from == 60 && to == 58 && Pieces[from].type == PieceType::King)
        {
            hash ^= randomHashValues[56][3];
            hash ^= randomHashValues[59][3];
            Pieces[59] = Pieces[56];
            Pieces[56] = Piece();
        }
        if (from == 4 && to == 6 && Pieces[from].type == PieceType::King)
        {
            hash ^= randomHashValues[5][9];
            hash ^= randomHashValues[7][9];
            Pieces[5] = Pieces[7];
            Pieces[7] = Piece();
        }
        if (from == 4 && to == 2 && Pieces[from].type == PieceType::King)
        {
            hash ^= randomHashValues[3][9];
            hash ^= randomHashValues[0][9];
            Pieces[3] = Pieces[0];
            Pieces[0] = Piece();
        }

        //Prevents castling if rook gets taken
        if (to == 63)
        {
            hash ^= randomHashValuesCastle[0];
            wKCastle = false;
        }
        if (to == 56)
        {
            hash ^= randomHashValuesCastle[1];
            wQCastle = false;
        }
        if (to == 7)
        {
            hash ^= randomHashValuesCastle[2];
            bKCastle = false;
        }
        if (to == 0)
        {
            hash ^= randomHashValuesCastle[3];
            bQCastle = false;
        }

        //Prevents castling if rook moves
        if (from == 63 && Pieces[from].type == PieceType::Rook)
        {
            hash ^= randomHashValuesCastle[0];
            wKCastle = false;
        }
        if (from == 56 && Pieces[from].type == PieceType::Rook)
        {
            hash ^= randomHashValuesCastle[1];
            wQCastle = false;
        }
        if (from == 7 && Pieces[from].type == PieceType::Rook)
        {
            hash ^= randomHashValuesCastle[2];
            bKCastle = false;
        }
        if (from == 0 && Pieces[from].type == PieceType::Rook)
        {
            hash ^= randomHashValuesCastle[3];
            bQCastle = false;
        }

        //Prevents castling if king moves
        if (Pieces[from].isBlack && Pieces[from].type == PieceType::King)
        {
            bKing = to;
            bKCastle = false;
            bQCastle = false;
            hash ^= randomHashValuesCastle[2];
            hash ^= randomHashValuesCastle[3];
        }
        if (!Pieces[from].isBlack && Pieces[from].type == PieceType::King)
        {
            wKing = to;
            wKCastle = false;
            wQCastle = false;
            hash ^= randomHashValuesCastle[0];
            hash ^= randomHashValuesCastle[1];
        }

        

        if (blackTurn)
        {
            if (Pieces[to].type != PieceType::Empty)
            {
                hash ^= randomHashValues[to][Pieces[to].type - 1];
            }
            hash ^= randomHashValues[from][Pieces[from].type - 1 + 6];
            hash ^= randomHashValues[to][Pieces[from].type - 1 + 6];
        }
        else
        {
            if (Pieces[to].type != PieceType::Empty)
            {
                hash ^= randomHashValues[to][Pieces[to].type - 1 + 6];
            }
            hash ^= randomHashValues[from][Pieces[from].type - 1];
            hash ^= randomHashValues[to][Pieces[from].type - 1];
        }

        Pieces[to] = Pieces[from];

        if (Pieces[to].isBlack)
        {
            blackPieceTable += pTables[Pieces[from].type - 1][63 - to];
        }
        else
        {
            whitePieceTable += pTables[Pieces[from].type - 1][to];
        }


        if (Pieces[from].type == PieceType::Pawn && to == enPassant)
        {
            //std::cout << "en passanted" << std::endl;
            if (Pieces[from].isBlack)
            {
                hash ^= randomHashValues[enPassant - 8][0];
                Pieces[enPassant - 8] = Piece(PieceType::Empty);
            }
            else
            {
                hash ^= randomHashValues[enPassant - 8][6];
                Pieces[enPassant + 8] = Piece(PieceType::Empty);
            }
        }

        if (enPassant >= 0)
        {
            hash ^= randomHashValuesEP[enPassant % 8];
        }

        enPassant = -10;

        if (Pieces[from].type == PieceType::Pawn && abs(to - from) == 16)
        {
            if (Pieces[from].isBlack)
            {
                hash ^= randomHashValuesEP[from % 8];
                enPassant = from + 8;
            }
            else
            {
                hash ^= randomHashValuesEP[from % 8];
                enPassant = from - 8;
            }
        }
        Pieces[from] = Piece(PieceType::Empty);

        
        hash ^= randomHashValuesIsBlack;
        blackTurn = !blackTurn;

    }
};

int perft(Board b, int depth)
{
    //std::vector<std::vector<int>> moves = b.LegalMoves(b.blackTurn);

    std::vector<move> moves = b.LegalMoves(b.blackTurn);

    if (depth == 0)
    {
        int size = 0;

        size += moves.size();
        
        return size;
    }

    int count = 0;

    for (int i = 0; i < moves.size(); i++)
    {
        Board bc = b;
        bc.Move(moves[i].from, moves[i].to);
        count += perft(bc, depth - 1);
    }
    return count;
}

float Minimax(float alpha, float beta, int depth, Board board, int qDepth)
{
    std::vector<move> moves;

    //std::cout << depth << " : " << qDepth << std::endl;

    float maxEval = -INFINITY;
    float minEval = INFINITY;

    if (board.transpositionTable->find(board.hash) != board.transpositionTable->end() && board.transpositionTable->at(board.hash).depth >= depth)
    {
        return board.transpositionTable->at(board.hash).eval;
    }
    else
    {
        if (board.transpositionTable->find(board.hash) != board.transpositionTable->end())
        {
            moves = board.transpositionTable->at(board.hash).moves;

            std::sort(moves.begin(), moves.end(), compare);
        }
        if (depth == 0)
        {
            if (qDepth == 0)
            {
                float eval = board.evaluate();

                moveInfo temp;
                temp.eval = eval;
                temp.depth = 0;
                temp.moves = moves;

                board.transpositionTable->insert(std::make_pair(board.hash, temp));
                return eval;
            }
            else
            {
                if (!board.blackTurn)
                {
                    moves = board.LegalMoves(board.blackTurn);

                    std::sort(moves.begin(), moves.end(), compare);

                    maxEval = -INFINITY;

                    for (int i = 0; i < moves.size(); i++)
                    {
                        // PROBLEM WITH PROMOTION NEGATIVE NUMBERS.TYPE BEING CHECKED IF EMPTY WHEN USING QUIENCESE SEARCH
                        
                        if (board.Pieces[moves[i].to].type != PieceType::Empty)
                        {
                            Board b = board;
                            b.Move(moves[i].from, moves[i].to);
                            float eval = Minimax(alpha, beta, depth, b, qDepth - 1);

                            maxEval = std::max(maxEval, eval);

                            alpha = std::max(alpha, maxEval);
                            if (beta <= alpha)
                            {
                                break;
                            }
                        }
                    }

                    moveInfo temp;
                    temp.eval = maxEval;
                    temp.depth = depth;
                    temp.moves = moves;
                    board.transpositionTable->insert(std::make_pair(board.hash, temp));

                    if (maxEval == -INFINITY)
                    {
                        return board.evaluate();
                    }
                    return maxEval;
                }
                else
                {
                    moves = board.LegalMoves(board.blackTurn);

                    std::sort(moves.begin(), moves.end(), compare);

                    minEval = INFINITY;

                    for (int i = 0; i < moves.size(); i++)
                    {
                        if (board.Pieces[moves[i].to].type != PieceType::Empty)
                        {
                            Board b = board;
                            b.Move(moves[i].from, moves[i].to);
                            float eval = Minimax(alpha, beta, depth, b, qDepth - 1);

                            minEval = std::min(minEval, eval);
                            beta = std::min(beta, minEval);
                            if (beta <= alpha)
                            {
                                break;
                            }
                        }

                    }
                    moveInfo temp;
                    temp.eval = minEval;
                    temp.depth = depth;
                    temp.moves = moves;
                    board.transpositionTable->insert(std::make_pair(board.hash, temp));

                    if (minEval == INFINITY)
                    {
                        return board.evaluate();
                    }
                    return minEval;
                }
                return board.evaluate();
            }
        }
        else if (!board.blackTurn)
        {
            moves = board.LegalMoves(board.blackTurn);

            std::sort(moves.begin(), moves.end(), compare);

            maxEval = -INFINITY;

            for (int i = 0; i < moves.size(); i++)
            {
                Board b = board;
                b.Move(moves[i].from, moves[i].to);
                float eval = Minimax(alpha, beta, depth - 1, b, qDepth);

                maxEval = std::max(maxEval, eval);

                alpha = std::max(alpha, maxEval);
                if (beta <= alpha)
                {
                    break;
                }
            }

            moveInfo temp;
            temp.eval = maxEval;
            temp.depth = depth;
            temp.moves = moves;
            board.transpositionTable->insert(std::make_pair(board.hash, temp));

            return maxEval;
        }
        else
        {
            moves = board.LegalMoves(board.blackTurn);

            std::sort(moves.begin(), moves.end(), compare);

            minEval = INFINITY;

            for (int i = 0; i < moves.size(); i++)
            {
                Board b = board;
                b.Move(moves[i].from, moves[i].to);
                float eval = Minimax(alpha, beta, depth - 1, b, qDepth);

                minEval = std::min(minEval, eval);
                beta = std::min(beta, minEval);
                if (beta <= alpha)
                {
                    break;
                }

            }
            moveInfo temp;
            temp.eval = minEval;
            temp.depth = depth;
            temp.moves = moves;
            board.transpositionTable->insert(std::make_pair(board.hash, temp));

            return minEval;
        }
    }
}

int main()
{
    srand(time(0));
    RenderWindow app(VideoMode(512, 512), "Chess", sf::Style::Close);
    app.setFramerateLimit(60);
    Texture bp, bn, bb, br, bq, bk, wp, wn, wb, wr, wq, wk;
    bp.loadFromFile("images/BlackPawn.png");
    bn.loadFromFile("images/BlackKnight.png");
    bb.loadFromFile("images/BlackBishop.png");
    br.loadFromFile("images/BlackRook.png");
    bq.loadFromFile("images/BlackQueen.png");
    bk.loadFromFile("images/BlackKing.png");
    wp.loadFromFile("images/WhitePawn.png");
    wn.loadFromFile("images/WhiteKnight.png");
    wb.loadFromFile("images/WhiteBishop.png");
    wr.loadFromFile("images/WhiteRook.png");
    wq.loadFromFile("images/WhiteQueen.png");
    wk.loadFromFile("images/WhiteKing.png");

    Sprite bPawn(bp), bKnight(bn), bBishop(bb), bRook(br), bQueen(bq), bKing(bk), wPawn(wp), wKnight(wn), wBishop(wb), wRook(wr), wQueen(wq), wKing(wk);
    std::vector<Sprite> sprites;
    sprites.emplace_back(wPawn);
    sprites.emplace_back(wKnight);
    sprites.emplace_back(wBishop);
    sprites.emplace_back(wRook);
    sprites.emplace_back(wQueen);
    sprites.emplace_back(wKing);
    sprites.emplace_back(bPawn);
    sprites.emplace_back(bKnight);
    sprites.emplace_back(bBishop);
    sprites.emplace_back(bRook);
    sprites.emplace_back(bQueen);
    sprites.emplace_back(bKing);

    for (int i = 0; i < sprites.size(); i++)
    {
        sprites[i].setScale(0.5, 0.5);
    }
    std::unordered_map<uint32_t, moveInfo> transpositionTable;

    Board board;

    board.transpositionTable = &transpositionTable;

    board.loadPosition("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    board.initHash();

    bool prevlClick = false; 

    int selectedSquare = -1;

    std::vector<int> posMoves;
    bool lClick = false;

    for (int i = 0; i < 3; i++)
    {
        auto t1 = std::chrono::high_resolution_clock::now();

        std::cout << "perft " << i << ": " << perft(board, i);

        for (int i = 0; i < 100; i++)
        {
            //std::vector<int> pin = board.genPinMap(true); //100 0.08~ // emplace 100 = 0.02~
            //std::vector<move> asdqw = board.LegalMoves(true); //100 = 0.35~ // emplace 100 = 0.17~
            //std::vector<move> asdqw = board.possibleMoves(true); //100 = 0.13~ // emplace 100 = 0.05~
        }
        
        

        auto t2 = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double, std::milli> ms_double = t2 - t1;

        std::cout << " in " << ms_double.count() << "ms\n";
    }

    while (app.isOpen())
    {
        Event e;
        while (app.pollEvent(e))
        {
            if (e.type == Event::Closed)
                app.close();
        }

        if (app.hasFocus())
        {
            lClick = sf::Mouse::isButtonPressed(sf::Mouse::Left);
        }

        if (board.blackTurn)
        {
            std::vector<move> moves = board.LegalMoves(board.blackTurn);
            std::sort(moves.begin(), moves.end(), compare);

            std::vector<int> d;

            std::vector<std::vector<int>> allMoves(64, d);

            for (int i = 0; i < moves.size(); i++)
            {
                allMoves[moves[i].from].emplace_back(moves[i].to);
            }

            bool hasMoves = false;

            for (int i = 0; i < allMoves.size(); i++)
            {
                if (allMoves[i].size() > 0)
                {
                    hasMoves = true;
                    break;
                }
            }

            if (hasMoves)
            {
                debug = 0;
                debug2 = 0;

                int bestMoveA;
                int bestMoveB;
                float bestMoveE = INFINITY;

                auto t1 = std::chrono::high_resolution_clock::now();

                for (int i = 0; i < allMoves.size(); i++)
                {
                    for (int j = 0; j < allMoves[i].size(); j++)
                    {

                        Board b = board;
                        b.Move(i, allMoves[i][j]);
                        float temp = Minimax(-INFINITY, INFINITY, 4, b, 1);
                        if (temp <= bestMoveE)
                        {
                            bestMoveA = i;
                            bestMoveB = allMoves[i][j];
                            bestMoveE = temp;
                        }
                    }
                }
                auto t2 = std::chrono::high_resolution_clock::now();

                std::chrono::duration<double, std::milli> ms_double = t2 - t1;

                std::cout << "played move " << bestMoveA << " to " << bestMoveB << " in " << ms_double.count() << "ms\n";
                board.Move(bestMoveA, bestMoveB);

                

                std::cout << "debug: " << debug << std::endl;
                std::cout << "debug2 : " << debug2 << std::endl;

                transpositionTable.clear();

                //board.Move(rand1, allMoves[rand1][rand2]);
            }
            else
            {
                int k;
                if (board.blackTurn)
                {
                    k = board.bKing;
                }
                else
                {
                    k = board.wKing;
                }
                if (board.inCheck(k, board.blackTurn))
                {
                    std::cout << "Checkmate" << std::endl;
                }
                else
                {
                    std::cout << "Stalemate" << std::endl;
                }
            }
        }

        if (lClick != prevlClick && lClick)
        {
            std::cout << board.hash << std::endl;

            std::cout << "hash table size: " << transpositionTable.size() << std::endl;
            int x = floor(sf::Mouse::getPosition(app).x / 64);
            int y = floor(sf::Mouse::getPosition(app).y / 64);
            /*
            bool a;
            
            std::vector<bool> check = board.genCheckMap(false, a);

            std::cout << "check map:" << std::endl;

            for (int i = 0; i < check.size(); i++)
            {
                std::cout << check[i] << " ";
                if (i % 8 == 7)
                {
                    std::cout << std::endl;
                }
            }
            std::cout << "end of check map" << std::endl;

            
            
            if (board.inCheck(board.wKing, false))
            {
                std::cout << "your in check, mate" << std::endl;
            }
            else
            {
                std::cout << "your not in check, mate" << std::endl;
            }

            

            std::vector<bool> attack = board.genAttackMap(true);

            std::cout << "attack map:" << std::endl;

            for (int i = 0; i < attack.size(); i++)
            {
                std::cout << attack[i] << " ";
                if (i % 8 == 7)
                {
                    std::cout << std::endl;
                }
            }
            std::cout << "end of attack map" << std::endl;

            std::vector<int> pin = board.genPinMap(false);

            std::cout << "Pin map:" << std::endl;

            for (int i = 0; i < pin.size(); i++)
            {
                std::cout << pin[i] << " ";
                if (i % 8 == 7)
                {
                    std::cout << std::endl;
                }
            }
            std::cout << "end of pin map" << std::endl;
            
            */

            //7 less with f2 - f3
                //Nh6 off by 1
                //Nf6 off by 2
                //e6 off by 2 
                    //Kf2 off by 2 
                        //Bc5 doesnt allow pawns to block check 
                //e5 off by 2 


            //std::cout << perft(board, 1) << " moves" << std::endl;


            //board.loadPosition("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
            //262 more
                //h1g1 46 more
                    //e8c8 46 more
                    
            //std::cout << "eval: "<< std::to_string(board.evaluate()) << std::endl;


            if (x < 8 && y < 8)
            {
                if (selectedSquare == -1)
                {
                    std::vector<move> moves = board.LegalMoves(board.blackTurn);
                    std::vector<int> d;

                    std::vector<std::vector<int>> allMoves(64, d);

                    for (int i = 0; i < moves.size(); i++)
                    {
                        allMoves[moves[i].from].emplace_back(moves[i].to);
                    }

                    posMoves = allMoves[(y * 8) + x];
                    //posMoves = board.possibleMoves((y * 8) + x, false);
                    selectedSquare = (y * 8) + x;
                }
                else
                {
                    bool wasMove = false;
                    for (int i = 0; i < posMoves.size(); i++)
                    {
                        if (posMoves[i] == (y * 8) + x)
                        {
                            wasMove = true;
                        }
                    }
                    if (wasMove)
                    {
                        board.Move(selectedSquare, (y * 8) + x);
                        

                        posMoves.clear();
                        selectedSquare = -1;
                    }
                    else
                    {
                        if ((y * 8) + x == selectedSquare)
                        {
                            posMoves.clear();
                            selectedSquare = -1;
                        }
                        else
                        {
                            std::vector<move> moves = board.LegalMoves(board.blackTurn);

                            std::vector<int> d;

                            std::vector<std::vector<int>> allMoves(64, d);

                            for (int i = 0; i < moves.size(); i++)
                            {
                                allMoves[moves[i].from].emplace_back(moves[i].to);
                            }

                            posMoves = allMoves[(y * 8) + x];
                            selectedSquare = (y * 8) + x;
                        }
                        
                    }
                }
            }
        }

        

        prevlClick = lClick;

        app.clear(Color::White);

        int size = board.Pieces.size();
        for (int i = 0; i < size; i++)
        {
            RectangleShape s;
            s.setSize(Vector2f(64,64));

            float x = (i % 8);
            float y = floor(i / 8);

            s.setPosition(x * 64, y * 64);

            if (int(x + 1 + y + 1) % 2 == 0)
            {
                if (i == selectedSquare)
                {
                    s.setFillColor(Color(157, 215, 213, 255));
                }
                else if (std::find(posMoves.begin(), posMoves.end(), i) != posMoves.end())
                {
                    s.setFillColor(Color(133, 243, 124, 255));
                }
                else
                {
                    s.setFillColor(Color(221, 234, 234, 255));
                }
                
            }
            else
            {
                if (i == selectedSquare)
                {
                    s.setFillColor(Color(123, 181, 182, 255));
                }
                else if (std::find(posMoves.begin(), posMoves.end(), i) != posMoves.end())
                {
                    s.setFillColor(Color(49, 159, 49, 255));
                }
                else
                {
                    s.setFillColor(Color(63, 76, 92, 255));
                }
            }

            app.draw(s);

            if (board.Pieces[i].type != PieceType::Empty)
            {
                int sprite = (6 * board.Pieces[i].isBlack) + board.Pieces[i].type - 1;
                sprites[sprite].setPosition((i % 8) * 64, floor(i / 8) * 64);

                app.draw(sprites[sprite]);
            }
        }
        
        app.display();
    }
    return 0;
}