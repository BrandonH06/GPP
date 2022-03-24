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

struct moveInfo
{
    int depth;
    float eval;
};

struct Move
{
    int from;
    int to;
    int priority = 0;
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

int compare(const void* a, const void* b)
{
    /*int main()
{
    int n;
    qsort(values, 6, sizeof(Move), compare);
    for (n = 0; n < 6; n++)
        printf("%d ", values[n]);
    return 0;
}*/

    Move* MoveA = (Move*)a;
    Move* MoveB = (Move*)b;

    return (MoveB->priority - MoveA->priority);
}

struct Board
{
    std::unordered_map<uint32_t, moveInfo>* transpositionTable;

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

    std::vector<int> pinnedMask;

    void loadPosition(std::string fen)
    {
        //add the move counter stuff for 50 move rule ect

        int temp = 0;
        for (int i = 0; i < fen.length(); i++)
        {
            if (fen[i] == 'r')
            {
                Pieces.push_back(Piece(PieceType::Rook, true));
                
            }
            else if (fen[i] == 'n')
            {
                Pieces.push_back(Piece(PieceType::Knight, true));
                
            }
            else if (fen[i] == 'b')
            {
                Pieces.push_back(Piece(PieceType::Bishop, true));
                
            }
            else if (fen[i] == 'q')
            {
                Pieces.push_back(Piece(PieceType::Queen, true));
                
            }
            else if (fen[i] == 'k')
            {
                bKing = Pieces.size();
                Pieces.push_back(Piece(PieceType::King, true));
                
            }
            else if (fen[i] == 'p')
            {
                Pieces.push_back(Piece(PieceType::Pawn, true));
                
            }
            else if (fen[i] == 'P')
            {
                Pieces.push_back(Piece(PieceType::Pawn, false));
                
            }
            else if (fen[i] == 'K')
            {
                wKing = Pieces.size();
                Pieces.push_back(Piece(PieceType::King, false));
                
            }
            else if (fen[i] == 'Q')
            {
                Pieces.push_back(Piece(PieceType::Queen, false));
                
            }
            else if (fen[i] == 'B')
            {
                Pieces.push_back(Piece(PieceType::Bishop, false));
                
            }
            else if (fen[i] == 'N')
            {
                Pieces.push_back(Piece(PieceType::Knight, false));
                
            }
            else if (fen[i] == 'R')
            {
                Pieces.push_back(Piece(PieceType::Rook, false));
                
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
                    Pieces.push_back(Piece(PieceType::Empty, false));
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
                        moves.push_back(SquareInfront);

                        //if moving to either the first rank or eighth rank
                        /*if ((floor(SquareInfront / 8) == 0 || floor(SquareInfront / 8) == 7) && !ignoreKing)
                        {
                            moves.push_back(SquareInfront + (8 * direction * 1));
                            moves.push_back(SquareInfront + (8 * direction * 2));
                            moves.push_back(SquareInfront + (8 * direction * 3));
                        }*/

                        int Square2Infront = p + (16 * direction);

                        //Checks 2 squares infront if 1 square infront was clear and if its on either 2nd or 7th rank
                        if (Square2Infront >= 0 && Square2Infront < 64)
                        {
                            if (Pieces[Square2Infront].type == PieceType::Empty && (floor(p / 8) == 1 || floor(p / 8) == 6))
                            {
                                moves.push_back(Square2Infront);
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
                            moves.push_back(SquareDiagRight);

                            //if moving to either the first rank or eighth rank
                            /*if ((floor(SquareDiagRight / 8) == 0 || floor(SquareDiagRight / 8) == 7) && !ignoreKing)
                            {
                                moves.push_back(SquareDiagRight + (8 * direction * 1));
                                moves.push_back(SquareDiagRight + (8 * direction * 2));
                                moves.push_back(SquareDiagRight + (8 * direction * 3));
                            }*/
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
                            moves.push_back(SquareDiagLeft);

                            //if moving to either the first rank or eighth rank
                            /*if ((floor(SquareDiagLeft / 8) == 0 || floor(SquareDiagLeft / 8) == 7) && !ignoreKing)
                            {
                                moves.push_back(SquareDiagLeft + (8 * direction * 1));
                                moves.push_back(SquareDiagLeft + (8 * direction * 2));
                                moves.push_back(SquareDiagLeft + (8 * direction * 3));
                            }*/
                        }
                    }
                }
            }

            if (ignoreKing)
            {
                // limit to edges of the board on ranks 1 and 8
                if (p % 8 != 7)
                {
                    moves.push_back(SquareDiagRight);
                }
                // limit to edges of the board on ranks 1 and 8
                if (p % 8 != 0)
                {
                    moves.push_back(SquareDiagLeft);
                }
            }

            //En passant diagonally right
            if (SquareDiagRight == enPassant)
            {
                // limit to edges of the board on ranks 1 and 8
                if (p % 8 != 7)
                {
                    moves.push_back(SquareDiagRight);
                }
            }

            //En passant diagonally left
            if (SquareDiagLeft == enPassant)
            {
                // limit to edges of the board on ranks 1 and 8
                if (p % 8 != 0)
                {
                    moves.push_back(SquareDiagLeft);
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
                            moves.push_back(topLeft);
                        }
                    }
                    else
                    {
                        moves.push_back(topLeft);
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
                            moves.push_back(topRight);
                        }
                    }
                    else
                    {
                        moves.push_back(topRight);
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
                            moves.push_back(leftTop);
                        }
                    }
                    else
                    {
                        moves.push_back(leftTop);
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
                            moves.push_back(leftDown);
                        }
                    }
                    else
                    {
                        moves.push_back(leftDown);
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
                            moves.push_back(rightUp);
                        }
                    }
                    else
                    {
                        moves.push_back(rightUp);
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
                            moves.push_back(rightDown);
                        }
                    }
                    else
                    {
                        moves.push_back(rightDown);
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
                            moves.push_back(downLeft);
                        }
                    }
                    else
                    {
                        moves.push_back(downLeft);
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
                            moves.push_back(downRight);
                        }
                    }
                    else
                    {
                        moves.push_back(downRight);
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
                    moves.push_back(topLeft);
                }
                else
                {
                    if (Pieces[topLeft].isBlack != Pieces[p].isBlack)
                    {
                        if (ignoreKing && Pieces[topLeft].type == PieceType::King)
                        {
                            moves.push_back(topLeft);
                            
                        }
                        else
                        {
                            moves.push_back(topLeft);
                            break;
                        }
                        
                    }
                    else
                    {
                        if (ignoreKing)
                        {
                            moves.push_back(topLeft);
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
                    moves.push_back(bottomLeft);
                }
                else
                {
                    if (Pieces[bottomLeft].isBlack != Pieces[p].isBlack)
                    {
                        if (ignoreKing && Pieces[bottomLeft].type == PieceType::King)
                        {
                            moves.push_back(bottomLeft);

                        }
                        else
                        {
                            moves.push_back(bottomLeft);
                            break;
                        }
                    }
                    else
                    {
                        if (ignoreKing)
                        {
                            moves.push_back(bottomLeft);
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
                    moves.push_back(topRight);
                }
                else
                {
                    if (Pieces[topRight].isBlack != Pieces[p].isBlack)
                    {
                        if (ignoreKing && Pieces[topRight].type == PieceType::King)
                        {
                            moves.push_back(topRight);

                        }
                        else
                        {
                            moves.push_back(topRight);
                            break;
                        }
                    }
                    else
                    {
                        if (ignoreKing)
                        {
                            moves.push_back(topRight);
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
                    moves.push_back(bottomRight);
                }
                else
                {
                    if (Pieces[bottomRight].isBlack != Pieces[p].isBlack)
                    {
                        if (ignoreKing && Pieces[bottomRight].type == PieceType::King)
                        {
                            moves.push_back(bottomRight);

                        }
                        else
                        {
                            moves.push_back(bottomRight);
                            break;
                        }
                    }
                    else
                    {
                        if (ignoreKing)
                        {
                            moves.push_back(bottomRight);
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
                    moves.push_back(left);
                }
                else
                {
                    if (Pieces[left].isBlack != Pieces[p].isBlack)
                    {
                        if (ignoreKing && Pieces[left].type == PieceType::King)
                        {
                            moves.push_back(left);

                        }
                        else
                        {
                            moves.push_back(left);
                            break;
                        }
                    }
                    else
                    {
                        if (ignoreKing)
                        {
                            moves.push_back(left);
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
                    moves.push_back(top);
                }
                else
                {
                    if (Pieces[top].isBlack != Pieces[p].isBlack)
                    {
                        if (ignoreKing && Pieces[top].type == PieceType::King)
                        {
                            moves.push_back(top);

                        }
                        else
                        {
                            moves.push_back(top);
                            break;
                        }
                    }
                    else
                    {
                        if (ignoreKing)
                        {
                            moves.push_back(top);
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
                    moves.push_back(right);
                }
                else
                {
                    if (Pieces[right].isBlack != Pieces[p].isBlack)
                    {
                        if (ignoreKing && Pieces[right].type == PieceType::King)
                        {
                            moves.push_back(right);

                        }
                        else
                        {
                            moves.push_back(right);
                            break;
                        }
                    }
                    else
                    {
                        if (ignoreKing)
                        {
                            moves.push_back(right);
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
                    moves.push_back(bottom);
                }
                else
                {
                    if (Pieces[bottom].isBlack != Pieces[p].isBlack)
                    {
                        if (ignoreKing && Pieces[bottom].type == PieceType::King)
                        {
                            moves.push_back(bottom);

                        }
                        else
                        {
                            moves.push_back(bottom);
                            break;
                        }
                    }
                    else
                    {
                        if (ignoreKing)
                        {
                            moves.push_back(bottom);
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
                    moves.push_back(left);
                }
                else
                {
                    if (Pieces[left].isBlack != Pieces[p].isBlack)
                    {
                        if (ignoreKing && Pieces[left].type == PieceType::King)
                        {
                            moves.push_back(left);

                        }
                        else
                        {
                            moves.push_back(left);
                            break;
                        }
                    }
                    else
                    {
                        if (ignoreKing)
                        {
                            moves.push_back(left);
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
                    moves.push_back(top);
                }
                else
                {
                    if (Pieces[top].isBlack != Pieces[p].isBlack)
                    {
                        if (ignoreKing && Pieces[top].type == PieceType::King)
                        {
                            moves.push_back(top);

                        }
                        else
                        {
                            moves.push_back(top);
                            break;
                        }
                    }
                    else
                    {
                        if (ignoreKing)
                        {
                            moves.push_back(top);
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
                    moves.push_back(right);
                }
                else
                {
                    if (Pieces[right].isBlack != Pieces[p].isBlack)
                    {
                        if (ignoreKing && Pieces[right].type == PieceType::King)
                        {
                            moves.push_back(right);

                        }
                        else
                        {
                            moves.push_back(right);
                            break;
                        }
                    }
                    else
                    {
                        if (ignoreKing)
                        {
                            moves.push_back(right);
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
                    moves.push_back(bottom);
                }
                else
                {
                    if (Pieces[bottom].isBlack != Pieces[p].isBlack)
                    {
                        if (ignoreKing && Pieces[bottom].type == PieceType::King)
                        {
                            moves.push_back(bottom);

                        }
                        else
                        {
                            moves.push_back(bottom);
                            break;
                        }
                    }
                    else
                    {
                        if (ignoreKing)
                        {
                            moves.push_back(bottom);
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
                    moves.push_back(topLeft);
                }
                else
                {
                    if (Pieces[topLeft].isBlack != Pieces[p].isBlack)
                    {
                        if (ignoreKing && Pieces[topLeft].type == PieceType::King)
                        {
                            moves.push_back(topLeft);

                        }
                        else
                        {
                            moves.push_back(topLeft);
                            break;
                        }
                    }
                    else
                    {
                        if (ignoreKing)
                        {
                            moves.push_back(topLeft);
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
                    moves.push_back(bottomLeft);
                }
                else
                {
                    if (Pieces[bottomLeft].isBlack != Pieces[p].isBlack)
                    {
                        if (ignoreKing && Pieces[bottomLeft].type == PieceType::King)
                        {
                            moves.push_back(bottomLeft);

                        }
                        else
                        {
                            moves.push_back(bottomLeft);
                            break;
                        }
                    }
                    else
                    {
                        if (ignoreKing)
                        {
                            moves.push_back(bottomLeft);
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
                    moves.push_back(topRight);
                }
                else
                {
                    if (Pieces[topRight].isBlack != Pieces[p].isBlack)
                    {
                        if (ignoreKing && Pieces[topRight].type == PieceType::King)
                        {
                            moves.push_back(topRight);

                        }
                        else
                        {
                            moves.push_back(topRight);
                            break;
                        }
                    }
                    else
                    {
                        if (ignoreKing)
                        {
                            moves.push_back(topRight);
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
                    moves.push_back(bottomRight);
                }
                else
                {
                    if (Pieces[bottomRight].isBlack != Pieces[p].isBlack)
                    {
                        if (ignoreKing && Pieces[bottomRight].type == PieceType::King)
                        {
                            moves.push_back(bottomRight);

                        }
                        else
                        {
                            moves.push_back(bottomRight);
                            break;
                        }
                    }
                    else
                    {
                        if (ignoreKing)
                        {
                            moves.push_back(bottomRight);
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
                    moves.push_back(left);
                }
                else
                {
                    if (Pieces[left].isBlack != Pieces[p].isBlack || ignoreKing)
                    {
                        moves.push_back(left);
                    }
                }

                //Diag top left
                int topLeft = p - 9;
                if (topLeft >= 0)
                {
                    if (Pieces[topLeft].type == PieceType::Empty)
                    {
                        moves.push_back(topLeft);
                    }
                    else
                    {
                        if (Pieces[topLeft].isBlack != Pieces[p].isBlack || ignoreKing)
                        {
                            moves.push_back(topLeft);
                        }
                    }
                }

                //Diag bottom left
                int bottomLeft = p + 7;
                if (bottomLeft < 64)
                {
                    if (Pieces[bottomLeft].type == PieceType::Empty)
                    {
                        moves.push_back(bottomLeft);
                    }
                    else
                    {
                        if (Pieces[bottomLeft].isBlack != Pieces[p].isBlack || ignoreKing)
                        {
                            moves.push_back(bottomLeft);
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
                    moves.push_back(top);
                }
                else
                {
                    if (Pieces[top].isBlack != Pieces[p].isBlack || ignoreKing)
                    {
                        moves.push_back(top);
                    }
                }
            }

            //Bottom
            int bottom = p + 8;
            if (bottom < 64)
            {
                if (Pieces[bottom].type == PieceType::Empty)
                {
                    moves.push_back(bottom);
                }
                else
                {
                    if (Pieces[bottom].isBlack != Pieces[p].isBlack || ignoreKing)
                    {
                        moves.push_back(bottom);
                    }
                }
            }

            if (p % 8 != 7)
            {
                //Right
                int right = p + 1;
                if (Pieces[right].type == PieceType::Empty)
                {
                    moves.push_back(right);
                }
                else
                {
                    if (Pieces[right].isBlack != Pieces[p].isBlack || ignoreKing)
                    {
                        moves.push_back(right);
                    }
                }

                //Diag top right
                int topRight = p - 7;

                if (topRight >= 0)
                {
                    if (Pieces[topRight].type == PieceType::Empty)
                    {
                        moves.push_back(topRight);
                    }
                    else
                    {
                        if (Pieces[topRight].isBlack != Pieces[p].isBlack || ignoreKing)
                        {
                            moves.push_back(topRight);
                        }
                    }
                }

                //Diag bottom right
                int bottomRight = p + 9;
                if (bottomRight < 64)
                {
                    if (Pieces[bottomRight].type == PieceType::Empty)
                    {
                        moves.push_back(bottomRight);
                    }
                    else
                    {
                        if (Pieces[bottomRight].isBlack != Pieces[p].isBlack || ignoreKing)
                        {
                            moves.push_back(bottomRight);
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
                            moves.push_back(6);
                        }
                    }
                }
                if (Pieces[p].isBlack)
                {
                    if (bQCastle)
                    {
                        if ((Pieces[3].type == PieceType::Empty && Pieces[2].type == PieceType::Empty && Pieces[1].type == PieceType::Empty) && !(inCheck(4, true) || inCheck(3, true) || inCheck(2, true)))
                        {
                            moves.push_back(2);
                        }
                    }
                }

                if (!Pieces[p].isBlack)
                {
                    if (wKCastle)
                    {
                        if ((Pieces[61].type == PieceType::Empty && Pieces[62].type == PieceType::Empty) && !(inCheck(60, false) || inCheck(61, false) || inCheck(62, false)))
                        {
                            moves.push_back(62);
                        }
                    }
                }

                if (!Pieces[p].isBlack)
                {
                    if (wQCastle)
                    {
                        if ((Pieces[59].type == PieceType::Empty && Pieces[58].type == PieceType::Empty && Pieces[57].type == PieceType::Empty) && !(inCheck(60, false) || inCheck(59, false) || inCheck(58, false)))
                        {
                            moves.push_back(58);
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
        for (int i = 0; i < Pieces.size(); i++)
        {
            if (Pieces[i].isBlack == isBlack && Pieces[i].type != PieceType::Empty)
            {
                std::vector<int> temp = possibleMoves(i, true);

                for (int j = 0; j < temp.size(); j++)
                {
                    attackMap[temp[j]] = true;
                }
            }
        }
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
                tempSquares.push_back(left);
            }
            else
            {
                if (Pieces[left].isBlack != Pieces[p].isBlack)
                {
                    //hit enemy piece after seeing 1 of our pieces
                    if (Pieces[left].type == PieceType::Rook || Pieces[left].type == PieceType::Queen)
                    {
                        tempSquares.push_back(left);
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
                tempSquares.push_back(top);
            }
            else
            {
                if (Pieces[top].isBlack != Pieces[p].isBlack)
                {
                    //hit enemy piece after seeing 1 of our pieces
                    if (Pieces[top].type == PieceType::Rook || Pieces[top].type == PieceType::Queen)
                    {
                        tempSquares.push_back(top);
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
                tempSquares.push_back(right);
            }
            else
            {
                if (Pieces[right].isBlack != Pieces[p].isBlack)
                {
                    //hit enemy piece after seeing 1 of our pieces
                    if (Pieces[right].type == PieceType::Rook || Pieces[right].type == PieceType::Queen)
                    {
                        tempSquares.push_back(right);
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
                tempSquares.push_back(bottom);
            }
            else
            {
                if (Pieces[bottom].isBlack != Pieces[p].isBlack)
                {
                    //hit enemy piece after seeing 1 of our pieces
                    if (Pieces[bottom].type == PieceType::Rook || Pieces[bottom].type == PieceType::Queen)
                    {
                        tempSquares.push_back(bottom);
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
                tempSquares.push_back(topLeft);
            }
            else
            {
                if (Pieces[topLeft].isBlack != Pieces[p].isBlack)
                {
                    //hit enemy piece after seeing 1 of our pieces
                    if (Pieces[topLeft].type == PieceType::Bishop || Pieces[topLeft].type == PieceType::Queen)
                    {
                        tempSquares.push_back(topLeft);
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
                tempSquares.push_back(bottomLeft);
            }
            else
            {
                if (Pieces[bottomLeft].isBlack != Pieces[p].isBlack)
                {
                    //hit enemy piece after seeing 1 of our pieces
                    if (Pieces[bottomLeft].type == PieceType::Bishop || Pieces[bottomLeft].type == PieceType::Queen)
                    {
                        tempSquares.push_back(bottomLeft);
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
                tempSquares.push_back(topRight);
            }
            else
            {
                if (Pieces[topRight].isBlack != Pieces[p].isBlack)
                {
                    //hit enemy piece after seeing 1 of our pieces
                    if (Pieces[topRight].type == PieceType::Bishop || Pieces[topRight].type == PieceType::Queen)
                    {
                        tempSquares.push_back(topRight);
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
                tempSquares.push_back(bottomRight);
            }
            else
            {
                if (Pieces[bottomRight].isBlack != Pieces[p].isBlack)
                {
                    //hit enemy piece after seeing 1 of our pieces
                    if (Pieces[bottomRight].type == PieceType::Bishop || Pieces[bottomRight].type == PieceType::Queen)
                    {
                        tempSquares.push_back(bottomRight);
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
                tempSquares.push_back(left);
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
                                tempSquares.push_back(left);
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
                        tempSquares.push_back(left);
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
                tempSquares.push_back(top);
            }
            else
            {
                if (Pieces[top].isBlack != Pieces[p].isBlack)
                {
                    //hit enemy piece after seeing 1 of our pieces
                    if (hitOurPiece && (Pieces[top].type == PieceType::Rook || Pieces[top].type == PieceType::Queen))
                    {
                        tempSquares.push_back(top);
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
                        tempSquares.push_back(top);
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
                tempSquares.push_back(right);
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
                                tempSquares.push_back(right);
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
                        tempSquares.push_back(right);
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
                tempSquares.push_back(right);
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
                            tempSquares.push_back(right);
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
                        tempSquares.push_back(right);
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
                tempSquares.push_back(bottom);
            }
            else
            {
                if (Pieces[bottom].isBlack != Pieces[p].isBlack)
                {
                    //hit enemy piece after seeing 1 of our pieces
                    if (hitOurPiece && (Pieces[bottom].type == PieceType::Rook || Pieces[bottom].type == PieceType::Queen))
                    {
                        tempSquares.push_back(bottom);
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
                        tempSquares.push_back(bottom);
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
                tempSquares.push_back(topLeft);
            }
            else
            {
                if (Pieces[topLeft].isBlack != Pieces[p].isBlack)
                {
                    //hit enemy piece after seeing 1 of our pieces
                    if (hitOurPiece && (Pieces[topLeft].type == PieceType::Bishop || Pieces[topLeft].type == PieceType::Queen))
                    {
                        tempSquares.push_back(topLeft);
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
                        tempSquares.push_back(topLeft);
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
                tempSquares.push_back(bottomLeft);
            }
            else
            {
                if (Pieces[bottomLeft].isBlack != Pieces[p].isBlack)
                {
                    //hit enemy piece after seeing 1 of our pieces
                    if (hitOurPiece && (Pieces[bottomLeft].type == PieceType::Bishop || Pieces[bottomLeft].type == PieceType::Queen))
                    {
                        tempSquares.push_back(bottomLeft);
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
                        tempSquares.push_back(bottomLeft);
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
                tempSquares.push_back(topRight);
            }
            else
            {
                if (Pieces[topRight].isBlack != Pieces[p].isBlack)
                {
                    //hit enemy piece after seeing 1 of our pieces
                    if (hitOurPiece && (Pieces[topRight].type == PieceType::Bishop || Pieces[topRight].type == PieceType::Queen))
                    {
                        tempSquares.push_back(topRight);
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
                        tempSquares.push_back(topRight);
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
                tempSquares.push_back(bottomRight);
            }
            else
            {
                if (Pieces[bottomRight].isBlack != Pieces[p].isBlack)
                {
                    //hit enemy piece after seeing 1 of our pieces
                    if (hitOurPiece && (Pieces[bottomRight].type == PieceType::Bishop || Pieces[bottomRight].type == PieceType::Queen))
                    {
                        tempSquares.push_back(bottomRight);
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
                        tempSquares.push_back(bottomRight);
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
        debug++;
        float pieceTableMult = 0.01f;

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
            randomHashValuesCastle.push_back(randomNums(gen));
            hash ^= randomHashValuesCastle[i];
        }

        for (int i = 0; i < 8; i++)
        {
            randomHashValuesEP.push_back(randomNums(gen));
        }

        randomHashValuesIsBlack = randomNums(gen);

        for (int i = 0; i < Pieces.size(); i++)
        {
            std::vector<uint32_t> temp;
            for (int j = 0; j < 12; j++)
            {
                uint32_t num = randomNums(gen);
                temp.push_back(num);
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
            randomHashValues.push_back(temp);
        }
    }

    std::vector<std::vector<int>> LegalMoves(bool isBlack)
    {
        std::vector<std::vector<int>> legalMoves(64, std::vector<int>());

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
        //optimize VV

        
        /*if (checked)
        {
            checkMap = genCheckMap(isBlack, doubleChecked);
        }*/

        //optimize VV
        /*std::vector<bool> attack = genAttackMap(!isBlack);*/

        std::vector<int> pin = genPinMap(isBlack);

        /*std::cout << "white " << whitePieces.size();
        for (int i = 0; i < whitePieces.size(); i++)
        {
            std::cout << i << " = " << whitePieces[i] << std::endl;
        }*/



        for (int i = 0; i < Pieces.size(); i++)
        {
            //int p = i;
            if (Pieces[i].isBlack == blackTurn)
            {
                std::vector<int> moves;

                if (doubleChecked)
                {
                    std::vector<bool> attack = genAttackMap(!isBlack);
                    if (Pieces[i].type == PieceType::King)
                    {
                        std::vector<int> posMoves = possibleMoves(i, false);
                        for (int j = 0; j < posMoves.size(); j++)
                        {
                            if (!attack[posMoves[j]])
                            {
                                moves.push_back(posMoves[j]);
                            }
                        }
                        legalMoves[i] = moves;
                        break;
                    }
                }
                else if (checked)
                {
                    std::vector<bool> attack = genAttackMap(!isBlack);
                    if (Pieces[i].type == PieceType::King)
                    {
                        std::vector<int> posMoves = possibleMoves(i, false);
                        for (int j = 0; j < posMoves.size(); j++)
                        {
                            if (!attack[posMoves[j]])
                            {
                                moves.push_back(posMoves[j]);
                            }
                        }
                    }
                    else
                    {
                        checkMap = genCheckMap(isBlack, doubleChecked);
                        std::vector<int> posMoves = possibleMoves(i, false);
                        for (int j = 0; j < posMoves.size(); j++)
                        {
                            //not pinned
                            if (pin[i] == 0)
                            {
                                
                                //moving between king and the checking piece or taking checking piece
                                if (checkMap[posMoves[j]] == true)
                                {
                                    if (posMoves[j] == enPassant)
                                    {
                                        if (Pieces[i].type == PieceType::Pawn)
                                        {
                                            moves.push_back(posMoves[j]);
                                        }
                                    }
                                    else
                                    {
                                        if (Pieces[i].type == PieceType::Pawn && (posMoves[j] >= 56 || posMoves[j] <= 7))
                                        {
                                            int direction = -1;
                                            if (Pieces[i].isBlack)
                                            {
                                                direction = 1;
                                            }

                                            moves.push_back(posMoves[j] + (8 * direction * 1));
                                            moves.push_back(posMoves[j] + (8 * direction * 2));
                                            moves.push_back(posMoves[j] + (8 * direction * 3));
                                        }
                                        moves.push_back(posMoves[j]);
                                    }
                                }
                            }
                        }
                    }
                }
                else
                {
                    if (Pieces[i].type == PieceType::King)
                    {
                        std::vector<bool> attack = genAttackMap(!isBlack);
                        std::vector<int> posMoves = possibleMoves(i, false);
                        for (int j = 0; j < posMoves.size(); j++)
                        {
                            if (!attack[posMoves[j]])
                            {
                                moves.push_back(posMoves[j]);
                            }
                        }
                    }
                    else
                    {
                        //optimize VV
                        std::vector<int> posMoves = possibleMoves(i, false);
                        //optimize VV
                        for (int j = 0; j < posMoves.size(); j++)
                        {
                            //not pinned
                            if (pin[i] == 0)
                            {
                                if (Pieces[i].type == PieceType::Pawn && (posMoves[j] >= 56 || posMoves[j] <= 7))
                                {
                                    int direction = -1;
                                    if (Pieces[i].isBlack)
                                    {
                                        direction = 1;
                                    }

                                    moves.push_back(posMoves[j] + (8 * direction * 1));
                                    moves.push_back(posMoves[j] + (8 * direction * 2));
                                    moves.push_back(posMoves[j] + (8 * direction * 3));
                                }

                                moves.push_back(posMoves[j]);
                            }
                            else if (pin[i] != 0)
                            {
                                //special case for en passant
                                if (pin[i] == 9)
                                {
                                    if (posMoves[j] != enPassant)
                                    {
                                        moves.push_back(posMoves[j]);
                                    }
                                }
                                if (pin[i] == pin[posMoves[j]])
                                {
                                    moves.push_back(posMoves[j]);
                                }
                            }
                        }
                    }
                }

                legalMoves[i] = moves;
            }
        }
        return legalMoves;
    }

    void Move(int from, int to)
    {
        /*std::cout << "moved from " << from << " to " << to << std::endl;
        std::cout << "black " << blackPieceTable << std::endl;
        std::cout << "white " << whitePieceTable << std::endl;*/

        debug2++;
        if (Pieces[from].isBlack)
        {
            blackPieceTable -= pTables[Pieces[from].type - 1][63 - from];
        }
        else
        {
            whitePieceTable -= pTables[Pieces[from].type - 1][from];
        }

        if (Pieces[to].type != PieceType::Empty)
        {
            if (Pieces[to].isBlack)
            {
                blackPieceTable -= pTables[Pieces[from].type - 1][63 - from];
            }
            else
            {
                whitePieceTable -= pTables[Pieces[from].type - 1][from];
            }
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
            }
            else
            {
                pieceDiffenece -= pVal[Pieces[to].type];
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

        /*std::cout << "new black " << blackPieceTable << std::endl;
        std::cout << "new white " << whitePieceTable << std::endl;*/
    }
};

int perft(Board b, int depth)
{
    std::vector<std::vector<int>> moves = b.LegalMoves(b.blackTurn);
   

    if (depth == 0)
    {
        int size = 0;

        for (int i = 0; i < moves.size(); i++)
        {
            size += moves[i].size();
        }
        return size;
    }

    int count = 0;

    for (int i = 0; i < moves.size(); i++)
    {
        for (int j = 0; j < moves[i].size(); j++)
        {
            Board bc = b;
            bc.Move(i, moves[i][j]);
            count += perft(bc, depth - 1);
        }
    }
    return count;
}

float Minimax(float alpha, float beta, int depth, Board board, int qDepth)
{
    std::vector<std::vector<int>> moves;

    float maxEval = -INFINITY;
    float minEval = INFINITY;

    if (depth == 0)
    {
        if (qDepth == 0)
        {
            
            return board.evaluate();
        }
        else
        {

            moves = board.LegalMoves(board.blackTurn);
            for (int i = 0; i < moves.size(); i++)
            {
                for (int j = 0; j < moves[i].size(); j++)
                {
                    if (board.Pieces[moves[i][j]].type != PieceType::Empty && board.Pieces[moves[i][j]].isBlack != board.blackTurn)
                    {
                        if (!board.blackTurn)
                        {
                            Board b = board;
                            b.Move(i, moves[i][j]);
                            float eval = Minimax(alpha, beta, 0, b, qDepth - 1);

                            maxEval = std::max(maxEval, eval);

                            alpha = std::max(alpha, eval);
                            if (beta <= alpha)
                            {
                                break;
                            }
                            return maxEval;
                        }
                        else
                        {
                            Board b = board;
                            b.Move(i, moves[i][j]);
                            float eval = Minimax(alpha, beta, 0, b, qDepth - 1);

                            minEval = std::min(minEval, eval);
                            alpha = std::min(beta, eval);
                            if (beta <= alpha)
                            {
                                break;
                            }
                            return minEval;
                        }
                    }
                }
            }

            return board.evaluate();
        }
    }
    else if (!board.blackTurn)
    {
        moves = board.LegalMoves(board.blackTurn);
        for (int i = 0; i < moves.size(); i++)
        {
            for (int j = 0; j < moves[i].size(); j++)
            {
                Board b = board;
                b.Move(i, moves[i][j]);
                float eval = Minimax(alpha, beta, depth - 1, b, qDepth);

                maxEval = std::max(maxEval, eval);

                alpha = std::max(alpha, eval);
                if (beta <= alpha)
                {
                    break;
                }
                
            }
        }
        return maxEval;
    }
    else
    {
        moves = board.LegalMoves(board.blackTurn);
        for (int i = 0; i < moves.size(); i++)
        {
            for (int j = 0; j < moves[i].size(); j++)
            {
                Board b = board;
                b.Move(i, moves[i][j]);
                float eval = Minimax(alpha, beta, depth - 1, b, qDepth);

                minEval = std::min(minEval, eval);
                alpha = std::min(beta, eval);
                if (beta <= alpha)
                {
                    break;
                }
            }
        }
        return minEval;
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
    sprites.push_back(wPawn);
    sprites.push_back(wKnight);
    sprites.push_back(wBishop);
    sprites.push_back(wRook);
    sprites.push_back(wQueen);
    sprites.push_back(wKing);
    sprites.push_back(bPawn);
    sprites.push_back(bKnight);
    sprites.push_back(bBishop);
    sprites.push_back(bRook);
    sprites.push_back(bQueen);
    sprites.push_back(bKing);

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

    for (int i = 0; i < 0; i++)
    {
        auto t1 = std::chrono::high_resolution_clock::now();

        std::cout << "perft " << i << ": " << perft(board, i);

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
        /*
        if (board.blackTurn)
        {
            std::vector<std::vector<int>> allMoves = board.LegalMoves(board.blackTurn);

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
                        float temp = Minimax(-INFINITY, INFINITY, 3, b, 0);
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

        */

        if (lClick != prevlClick && lClick)
        {
            std::cout << board.hash << std::endl;
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
                    posMoves = board.LegalMoves(board.blackTurn)[(y * 8) + x];
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
                            posMoves = board.LegalMoves(board.blackTurn)[(y * 8) + x];
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