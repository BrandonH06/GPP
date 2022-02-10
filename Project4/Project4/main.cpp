#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <chrono>

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

struct Moves
{
    int from;
    std::vector<int> to;
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

struct Board
{
    std::vector<Piece> Pieces;
    int enPassant = -10;

    bool blackTurn = false;

    bool bKCastle = true;
    bool bQCastle = true;
    bool wKCastle = true;
    bool wQCastle = true;

    int bKing = 4;
    int wKing = 60;

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
                std::cout << "find lewtter in enpassant " << fen[i] << " " << epSquare << std::endl;
            }
            else
            {
                epSquare += 8 * (fen[i] - '1');
                std::cout << "find num in enpassant " << fen[i] << " " << epSquare << std::endl;
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

        if (Pieces[p].type == PieceType::Pawn)
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
                        if ((floor(SquareInfront / 8) == 0 || floor(SquareInfront / 8) == 7) && !ignoreKing)
                        {
                            moves.push_back(SquareInfront + (8 * direction * 1));
                            moves.push_back(SquareInfront + (8 * direction * 2));
                            moves.push_back(SquareInfront + (8 * direction * 3));
                        }

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
                            if ((floor(SquareDiagRight / 8) == 0 || floor(SquareDiagRight / 8) == 7) && !ignoreKing)
                            {
                                moves.push_back(SquareDiagRight + (8 * direction * 1));
                                moves.push_back(SquareDiagRight + (8 * direction * 2));
                                moves.push_back(SquareDiagRight + (8 * direction * 3));
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
                            moves.push_back(SquareDiagLeft);

                            //if moving to either the first rank or eighth rank
                            if ((floor(SquareDiagLeft / 8) == 0 || floor(SquareDiagLeft / 8) == 7) && !ignoreKing)
                            {
                                moves.push_back(SquareDiagLeft + (8 * direction * 1));
                                moves.push_back(SquareDiagLeft + (8 * direction * 2));
                                moves.push_back(SquareDiagLeft + (8 * direction * 3));
                            }
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

        if (Pieces[p].type == PieceType::Knight)
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

        if (Pieces[p].type == PieceType::Bishop)
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

        if (Pieces[p].type == PieceType::Rook)
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

        if (Pieces[p].type == PieceType::Queen)
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

        if (Pieces[p].type == PieceType::King)
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
                        if (!(inCheck(4, true) || inCheck(5, true) || inCheck(6, true)) && !(Pieces[5].type != PieceType::Empty || Pieces[6].type != PieceType::Empty))
                        {
                            moves.push_back(6);
                        }
                    }
                }
                if (Pieces[p].isBlack)
                {
                    if (bQCastle)
                    {
                        if (!(inCheck(4, true) || inCheck(3, true) || inCheck(2, true)) && !(Pieces[3].type != PieceType::Empty || Pieces[2].type != PieceType::Empty || Pieces[1].type != PieceType::Empty))
                        {
                            moves.push_back(2);
                        }
                    }
                }

                if (!Pieces[p].isBlack)
                {
                    if (wKCastle)
                    {
                        if (!(inCheck(60, false) || inCheck(61, false) || inCheck(62, false)) && !(Pieces[61].type != PieceType::Empty || Pieces[62].type != PieceType::Empty))
                        {
                            moves.push_back(62);
                        }
                    }
                }

                if (!Pieces[p].isBlack)
                {
                    if (wQCastle)
                    {
                        if (!(inCheck(60, false) || inCheck(59, false) || inCheck(58, false)) && !(Pieces[59].type != PieceType::Empty || Pieces[58].type != PieceType::Empty || Pieces[57].type != PieceType::Empty))
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

        /*for (int i = 0; i < temp.size(); i++)
        {
            std::cout << temp[i] << std::endl;
        }*/

        if (temp[p])
        {
            checked = true;
        }


        /*
        //Left
        int leftSquares = ((p % 8) + 1);
        for (int i = 1; i < leftSquares; i++)
        {
            int left = p - 1 * i;
            if (Pieces[left].type != PieceType::Empty && Pieces[left].isBlack == Pieces[p].isBlack)
            {
                break;
            }
            else if (Pieces[left].type != PieceType::Empty && Pieces[left].isBlack != Pieces[p].isBlack && (Pieces[left].type == PieceType::Rook || Pieces[left].type == PieceType::Queen))
            {
                std::cout << "chedked left" << std::endl;
                checked = true;
                break;
            }
        }

        //Top
        int topSquares = int(floor(p / 8) + 1);
        for (int i = 1; i < topSquares; i++)
        {
            int top = p - 8 * i;
            if (Pieces[top].type != PieceType::Empty && Pieces[top].isBlack == Pieces[p].isBlack)
            {
                break;
            }
            else if (Pieces[top].type != PieceType::Empty && Pieces[top].isBlack != Pieces[p].isBlack && (Pieces[top].type == PieceType::Rook || Pieces[top].type == PieceType::Queen))
            {
                std::cout << "checked from top" << std::endl;
                std::cout << p << " " << top << std::endl;
                checked = true;
                break;
            }
        }

        //Right
        int rightSquares = 8 - leftSquares + 1;
        for (int i = 1; i < rightSquares; i++)
        {
            int right = p + 1 * i;
            if (Pieces[right].type != PieceType::Empty && Pieces[right].isBlack == Pieces[p].isBlack)
            {
                break;
            }
            else if (Pieces[right].type != PieceType::Empty && Pieces[right].isBlack != Pieces[p].isBlack && (Pieces[right].type == PieceType::Rook || Pieces[right].type == PieceType::Queen))
            {
                std::cout << "chec3453fgke ddown right" << std::endl;
                checked = true;
                break;
            }
        }

        //Bottom
        int bottomSquares = 8 - topSquares + 1;
        for (int i = 1; i < bottomSquares; i++)
        {
            int bottom = p + 8 * i;
            if (Pieces[bottom].type != PieceType::Empty && Pieces[bottom].isBlack == Pieces[p].isBlack)
            {
                break;
            }
            else if (Pieces[bottom].type != PieceType::Empty && Pieces[bottom].isBlack != Pieces[p].isBlack && (Pieces[bottom].type == PieceType::Rook || Pieces[bottom].type == PieceType::Queen))
            {
                std::cout << "checksdfgher4e ddown right" << std::endl;
                checked = true;
                break;
            }
        }

        //Diag top left
        int topLeftSquares = std::min(leftSquares, topSquares);
        for (int i = 1; i < topLeftSquares; i++)
        {
            int topLeft = p - 9 * i;
            if (Pieces[topLeft].type != PieceType::Empty && Pieces[topLeft].isBlack == Pieces[p].isBlack)
            {
                break;
            }
            else if (Pieces[topLeft].type != PieceType::Empty && Pieces[topLeft].isBlack != Pieces[p].isBlack && (Pieces[topLeft].type == PieceType::Bishop || Pieces[topLeft].type == PieceType::Queen))
            {
                std::cout << "checked from top left" << std::endl;
                checked = true;
                break;
            }
        }

        //Diag bottom left
        int bottomLeftSquares = std::min(bottomSquares, leftSquares);
        for (int i = 1; i < bottomLeftSquares; i++)
        {
            int bottomLeft = p + 7 * i;
            if (Pieces[bottomLeft].type != PieceType::Empty && Pieces[bottomLeft].isBlack == Pieces[p].isBlack)
            {
                break;
            }
            else if (Pieces[bottomLeft].type != PieceType::Empty && Pieces[bottomLeft].isBlack != Pieces[p].isBlack && (Pieces[bottomLeft].type == PieceType::Bishop || Pieces[bottomLeft].type == PieceType::Queen))
            {
                std::cout << "checke ddown sdfg" << std::endl;
                checked = true;
                break;
            }
        }

        //Diag top right
        int topRightSquares = std::min(topSquares, rightSquares);
        for (int i = 1; i < topRightSquares; i++)
        {
            int topRight = p - 7 * i;
            if (Pieces[topRight].type != PieceType::Empty && Pieces[topRight].isBlack == Pieces[p].isBlack)
            {
                break;
            }
            else if (Pieces[topRight].type != PieceType::Empty && Pieces[topRight].isBlack != Pieces[p].isBlack && (Pieces[topRight].type == PieceType::Bishop || Pieces[topRight].type == PieceType::Queen))
            {
                std::cout << "checke d4g56h4down right" << std::endl;
                checked = true;
                break;
            }
        }

        //Diag bottom right
        int bottomRightSquares = std::min(bottomSquares, rightSquares);
        for (int i = 1; i < bottomRightSquares; i++)
        {
            int bottomRight = p + 9 * i;
            if (Pieces[bottomRight].type != PieceType::Empty && Pieces[bottomRight].isBlack == Pieces[p].isBlack)
            {
                break;
            }
            else if (Pieces[bottomRight].type != PieceType::Empty && Pieces[bottomRight].isBlack != Pieces[p].isBlack && (Pieces[bottomRight].type == PieceType::Bishop || Pieces[bottomRight].type == PieceType::Queen))
            {
                std::cout << "checke ddonrt64wn right" << std::endl;
                checked = true;
                break;
            }
        }

        //2 up 1 left
        int topLeft = p - 17;
        if (topLeft >= 0 && topLeft < 64)
        {
            if (p % 8 != 0)
            {
                if (Pieces[topLeft].type != PieceType::Empty && Pieces[topLeft].isBlack != Pieces[p].isBlack && Pieces[topLeft].type == PieceType::Knight)
                {
                    std::cout << "checke ddowndfgsb right" << std::endl;
                    checked = true;
                }
            }
        }

        //2 up 1 right
        int topRight = p - 15;
        if (topRight >= 0 && topRight < 64)
        {
            if (p % 8 != 7)
            {
                if (Pieces[topRight].type != PieceType::Empty && Pieces[topRight].isBlack != Pieces[p].isBlack && Pieces[topRight].type == PieceType::Knight)
                {
                    std::cout << "chec523462gke ddown right" << std::endl;
                    checked = true;
                }
            }
        }

        //2 left 1 up
        int leftTop = p - 10;
        if (leftTop >= 0 && leftTop < 64)
        {
            if (p % 8 != 1 && p % 8 != 0)
            {
                if (Pieces[leftTop].type != PieceType::Empty && Pieces[leftTop].isBlack != Pieces[p].isBlack && Pieces[leftTop].type == PieceType::Knight)
                {
                    std::cout << "check435252 ddown right" << std::endl;
                    checked = true;
                }
            }
        }

        //2 left 1 down
        int leftDown = p + 6;
        if (leftDown >= 0 && leftDown < 64)
        {
            if (p % 8 != 1 && p % 8 != 0)
            {
                if (Pieces[leftDown].type != PieceType::Empty && Pieces[leftDown].isBlack != Pieces[p].isBlack && Pieces[leftDown].type == PieceType::Knight)
                {
                    std::cout << "checke ddown rig7589ht" << std::endl;
                    checked = true;
                }
            }
        }

        //2 right 1 up
        int rightUp = p - 6;
        if (rightUp >= 0 && rightUp < 64)
        {
            if (p % 8 != 7 && p % 8 != 6)
            {
                if (Pieces[rightUp].type != PieceType::Empty && Pieces[rightUp].isBlack != Pieces[p].isBlack && Pieces[rightUp].type == PieceType::Knight)
                {
                    std::cout << "checke ddown6785 right" << std::endl;
                    checked = true;
                }
            }
        }

        //2 right 1 down
        int rightDown = p + 10;
        if (rightDown >= 0 && rightDown < 64)
        {
            if (p % 8 != 7 && p % 8 != 6)
            {
                if (Pieces[rightDown].type != PieceType::Empty && Pieces[rightDown].isBlack != Pieces[p].isBlack && Pieces[rightDown].type == PieceType::Knight)
                {
                    std::cout << "chec3453ke ddown right" << std::endl;
                    checked = true;
                }
            }
        }

        //2 down 1 left
        int downLeft = p + 15;
        if (downLeft >= 0 && downLeft < 64)
        {
            if (p % 8 != 0)
            {
                if (Pieces[downLeft].type != PieceType::Empty && Pieces[downLeft].isBlack != Pieces[p].isBlack && Pieces[downLeft].type == PieceType::Knight)
                {
                    std::cout << "checke ddsdfgown right" << std::endl;
                    checked = true;
                }
            }
        }

        //2 down 1 right
        int downRight = p + 17;
        if (downRight >= 0 && downRight < 64)
        {
            if (p % 8 != 7)
            {
                if (Pieces[downRight].type != PieceType::Empty && Pieces[downRight].isBlack != Pieces[p].isBlack && Pieces[downRight].type == PieceType::Knight)
                {
                    std::cout << "checke ddown right" << std::endl;
                    checked = true;
                }
            }
        }*/

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
        }
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

        if (checked)
        {
            checkMap = genCheckMap(isBlack, doubleChecked);
        }

        std::vector<bool> attack = genAttackMap(!isBlack);

        std::vector<int> pin = genPinMap(isBlack);

        for (int i = 0; i < Pieces.size(); i++)
        {
            if (Pieces[i].isBlack == isBlack)
            {
                int p = i;

                std::vector<int> moves;

                if (doubleChecked)
                {
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
                        std::vector<int> posMoves = possibleMoves(i, false);
                        for (int j = 0; j < posMoves.size(); j++)
                        {
                            //not pinned
                            if (pin[i] == 0)
                            {
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
        //white promoting
        if (Pieces[from].type == PieceType::Pawn &&  to < 8)
        {
            if (to >= 0)
            {
                Pieces[from] = Piece(PieceType::Queen, Pieces[from].isBlack);
            }
            else if (to >= -9)
            {
                Pieces[from] = Piece(PieceType::Knight, Pieces[from].isBlack);
            }
            else if (to >= -17)
            {
                Pieces[from] = Piece(PieceType::Rook, Pieces[from].isBlack);
            }
            else if (to >= -25)
            {
                Pieces[from] = Piece(PieceType::Bishop, Pieces[from].isBlack);
            }
            to = to % 8;
        }

        //black promoting
        if (Pieces[from].type == PieceType::Pawn && to > 55)
        {
            if (to <= 63)
            {
                Pieces[from] = Piece(PieceType::Queen, Pieces[from].isBlack);
            }
            else if (to <= 71)
            {
                Pieces[from] = Piece(PieceType::Knight, Pieces[from].isBlack);
            }
            else if (to <= 79)
            {
                Pieces[from] = Piece(PieceType::Rook, Pieces[from].isBlack);
            }
            else if (to <= 87)
            {
                Pieces[from] = Piece(PieceType::Bishop, Pieces[from].isBlack);
            }
            to = (to % 8) + 56;
        }

        //Move rook if castling
        if (from == 60 && to == 62 && Pieces[from].type == PieceType::King)
        {
            Pieces[61] = Pieces[63];
            Pieces[63] = Piece();
        }
        if (from == 60 && to == 58 && Pieces[from].type == PieceType::King)
        {
            Pieces[59] = Pieces[56];
            Pieces[56] = Piece();
        }
        if (from == 4 && to == 6 && Pieces[from].type == PieceType::King)
        {
            Pieces[5] = Pieces[7];
            Pieces[7] = Piece();
        }
        if (from == 4 && to == 2 && Pieces[from].type == PieceType::King)
        {
            Pieces[3] = Pieces[0];
            Pieces[0] = Piece();
        }

        //Prevents castling if rook gets taken
        if (to == 63)
        {
            wKCastle = false;
        }
        if (to == 56)
        {
            wQCastle = false;
        }
        if (to == 7)
        {
            bKCastle = false;
        }
        if (to == 0)
        {
            bQCastle = false;
        }

        //Prevents castling if rook moves
        if (from == 63 && Pieces[from].type == PieceType::Rook)
        {
            wKCastle = false;
        }
        if (from == 56 && Pieces[from].type == PieceType::Rook)
        {
            wQCastle = false;
        }
        if (from == 7 && Pieces[from].type == PieceType::Rook)
        {
            bKCastle = false;
        }
        if (from == 0 && Pieces[from].type == PieceType::Rook)
        {
            bQCastle = false;
        }

        //Prevents castling if king moves
        if (Pieces[from].isBlack && Pieces[from].type == PieceType::King)
        {
            bKing = to;
            bKCastle = false;
            bQCastle = false;
        }
        if (!Pieces[from].isBlack && Pieces[from].type == PieceType::King)
        {
            wKing = to;
            wKCastle = false;
            wQCastle = false;
        }

        Pieces[to] = Pieces[from];

        if (Pieces[from].type == PieceType::Pawn && to == enPassant)
        {
            //std::cout << "en passanted" << std::endl;
            if (Pieces[from].isBlack)
            {
                Pieces[enPassant - 8] = Piece(PieceType::Empty);
            }
            else
            {
                Pieces[enPassant + 8] = Piece(PieceType::Empty);
            }
        }

        enPassant = -10;

        if (Pieces[from].type == PieceType::Pawn && abs(to - from) == 16)
        {
            if (Pieces[from].isBlack)
            {
                enPassant = from + 8;
            }
            else
            {
                enPassant = from - 8;
            }
        }
        Pieces[from] = Piece(PieceType::Empty);
        blackTurn = !blackTurn;
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
            for (int j = 0; j < moves[i].size(); j++)
            {
                //std::cout << "piece " << i << " can move to " << moves[i][j] << std::endl;
                size++;
            }
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
            //bc.blackTurn = !bc.blackTurn;
            count += perft(bc, depth - 1);
        }
    }
    return count;
}

int main()
{
    srand(time(0));
    RenderWindow app(VideoMode(512, 512), "Chess");
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

    Board board;

    //board.loadPosition("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    board.loadPosition("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ");

    bool prevlClick = false;

    int selectedSquare = -1;

    std::vector<int> posMoves;
    bool lClick = false;

    for (int i = 0; i < 4; i++)
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

        if (lClick != prevlClick && lClick)
        {
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

            //4 less with f2 - f4
            std::cout << board.enPassant << " moves" << std::endl;
            

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

        //Random move bot
        /*if (board.blackTurn)
        {
            std::vector<std::vector<int>> allMoves = board.LegalMoves(board.blackTurn);
            int rand1 = rand() % allMoves.size();

            while (allMoves[rand1].size() <= 0)
            {
                rand1 = rand() % allMoves.size();
            }
            //std::cout << rand1 << " has a size of " << allMoves[rand1].size() << std::endl;
            

            int rand2 = rand() % allMoves[rand1].size();

            std::cout << rand1 << " : " << rand2 << std::endl;
            board.Move(rand1, allMoves[rand1][rand2]);
        }*/

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