#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>

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

    std::vector<int> possibleMoves(int p)
    {
        std::vector<int> moves;

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
                    moves.push_back(SquareInfront);

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
                    if (Pieces[topLeft].type == PieceType::Empty || (Pieces[topLeft].type != Empty && Pieces[topLeft].isBlack != Pieces[p].isBlack))
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
                    if (Pieces[topRight].type == PieceType::Empty || (Pieces[topRight].type != Empty && Pieces[topRight].isBlack != Pieces[p].isBlack))
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
                    if (Pieces[leftTop].type == PieceType::Empty || (Pieces[leftTop].type != Empty && Pieces[leftTop].isBlack != Pieces[p].isBlack))
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
                    if (Pieces[leftDown].type == PieceType::Empty || (Pieces[leftDown].type != Empty && Pieces[leftDown].isBlack != Pieces[p].isBlack))
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
                    if (Pieces[rightUp].type == PieceType::Empty || (Pieces[rightUp].type != Empty && Pieces[rightUp].isBlack != Pieces[p].isBlack))
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
                    if (Pieces[rightDown].type == PieceType::Empty || (Pieces[rightDown].type != Empty && Pieces[rightDown].isBlack != Pieces[p].isBlack))
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
                    if (Pieces[downLeft].type == PieceType::Empty || (Pieces[downLeft].type != Empty && Pieces[downLeft].isBlack != Pieces[p].isBlack))
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
                    if (Pieces[downRight].type == PieceType::Empty || (Pieces[downRight].type != Empty && Pieces[downRight].isBlack != Pieces[p].isBlack))
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
                        moves.push_back(topLeft);
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
                    moves.push_back(bottomLeft);
                }
                else
                {
                    if (Pieces[bottomLeft].isBlack != Pieces[p].isBlack)
                    {
                        moves.push_back(bottomLeft);
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
                    moves.push_back(topRight);
                }
                else
                {
                    if (Pieces[topRight].isBlack != Pieces[p].isBlack)
                    {
                        moves.push_back(topRight);
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
                    moves.push_back(bottomRight);
                }
                else
                {
                    if (Pieces[bottomRight].isBlack != Pieces[p].isBlack)
                    {
                        moves.push_back(bottomRight);
                        break;
                    }
                    else
                    {
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
                        moves.push_back(left);
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
                    moves.push_back(top);
                }
                else
                {
                    if (Pieces[top].isBlack != Pieces[p].isBlack)
                    {
                        moves.push_back(top);
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
                    moves.push_back(right);
                }
                else
                {
                    if (Pieces[right].isBlack != Pieces[p].isBlack)
                    {
                        moves.push_back(right);
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
                    moves.push_back(bottom);
                }
                else
                {
                    if (Pieces[bottom].isBlack != Pieces[p].isBlack)
                    {
                        moves.push_back(bottom);
                        break;
                    }
                    else
                    {
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
                        moves.push_back(left);
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
                    moves.push_back(top);
                }
                else
                {
                    if (Pieces[top].isBlack != Pieces[p].isBlack)
                    {
                        moves.push_back(top);
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
                    moves.push_back(right);
                }
                else
                {
                    if (Pieces[right].isBlack != Pieces[p].isBlack)
                    {
                        moves.push_back(right);
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
                    moves.push_back(bottom);
                }
                else
                {
                    if (Pieces[bottom].isBlack != Pieces[p].isBlack)
                    {
                        moves.push_back(bottom);
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
                    moves.push_back(topLeft);
                }
                else
                {
                    if (Pieces[topLeft].isBlack != Pieces[p].isBlack)
                    {
                        moves.push_back(topLeft);
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
                    moves.push_back(bottomLeft);
                }
                else
                {
                    if (Pieces[bottomLeft].isBlack != Pieces[p].isBlack)
                    {
                        moves.push_back(bottomLeft);
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
                    moves.push_back(topRight);
                }
                else
                {
                    if (Pieces[topRight].isBlack != Pieces[p].isBlack)
                    {
                        moves.push_back(topRight);
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
                    moves.push_back(bottomRight);
                }
                else
                {
                    if (Pieces[bottomRight].isBlack != Pieces[p].isBlack)
                    {
                        moves.push_back(bottomRight);
                        break;
                    }
                    else
                    {
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
                    if (Pieces[left].isBlack != Pieces[p].isBlack)
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
                        if (Pieces[topLeft].isBlack != Pieces[p].isBlack)
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
                        if (Pieces[bottomLeft].isBlack != Pieces[p].isBlack)
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
                    if (Pieces[top].isBlack != Pieces[p].isBlack)
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
                    if (Pieces[bottom].isBlack != Pieces[p].isBlack)
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
                    if (Pieces[right].isBlack != Pieces[p].isBlack)
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
                        if (Pieces[topRight].isBlack != Pieces[p].isBlack)
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
                        if (Pieces[bottomRight].isBlack != Pieces[p].isBlack)
                        {
                            moves.push_back(bottomRight);
                        }
                    }
                }
            }
        }

        /*std::cout << "type " << Pieces[p].type << std::endl;
        for (int i = 0; i < moves.size(); i++)
        {
            std::cout << moves[i] << std::endl;
        }*/
        return moves;
    }

    std::vector<bool> genAttackMap(bool isBlack)
    {
        std::vector<bool> attackMap(63, false);
        for (int i = 0; i < Pieces.size(); i++)
        {
            if (Pieces[i].isBlack == isBlack && Pieces[i].type != PieceType::Empty)
            {

                std::vector<int> temp = possibleMoves(i);
                for (int j = 0; j < temp.size(); j++)
                {
                    attackMap[temp[j]] = true;
                }
            }
        }
        return attackMap;
    }

    std::vector<int> checkMap;

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

    std::vector<int> genPinMap()
    {

    }

    void Move(int from, int to)
    {
        //update attack mask
        //update pinned mask

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



int main()
{
    RenderWindow app(VideoMode(512, 512), "Chess");
    app.setFramerateLimit(60);
    Texture t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12;
    t1.loadFromFile("images/BlackPawn.png");
    t2.loadFromFile("images/BlackKnight.png");
    t3.loadFromFile("images/BlackBishop.png");
    t4.loadFromFile("images/BlackRook.png");
    t5.loadFromFile("images/BlackQueen.png");
    t6.loadFromFile("images/BlackKing.png");
    t7.loadFromFile("images/WhitePawn.png");
    t8.loadFromFile("images/WhiteKnight.png");
    t9.loadFromFile("images/WhiteBishop.png");
    t10.loadFromFile("images/WhiteRook.png");
    t11.loadFromFile("images/WhiteQueen.png");
    t12.loadFromFile("images/WhiteKing.png");

    Sprite bPawn(t1), bKnight(t2), bBishop(t3), bRook(t4), bQueen(t5), bKing(t6), wPawn(t7), wKnight(t8), wBishop(t9), wRook(t10), wQueen(t11), wKing(t12);
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

    board.Pieces.push_back(Piece(PieceType::Rook, true));
    board.Pieces.push_back(Piece(PieceType::Knight, true));
    board.Pieces.push_back(Piece(PieceType::Bishop, true));
    board.Pieces.push_back(Piece(PieceType::Queen, true));
    board.Pieces.push_back(Piece(PieceType::King, true));
    board.Pieces.push_back(Piece(PieceType::Bishop, true));
    board.Pieces.push_back(Piece(PieceType::Knight, true));
    board.Pieces.push_back(Piece(PieceType::Rook, true));

    for (int i = 0; i < 8; i++)
    {
        board.Pieces.push_back(Piece(PieceType::Pawn, true));
    }

    for (int i = 0; i < 32; i++)
    {
        board.Pieces.push_back(Piece(PieceType::Empty, true));
    }

    for (int i = 0; i < 8; i++)
    {
        board.Pieces.push_back(Piece(PieceType::Pawn, false));
    }
    board.Pieces.push_back(Piece(PieceType::Rook, false));
    board.Pieces.push_back(Piece(PieceType::Knight, false));
    board.Pieces.push_back(Piece(PieceType::Bishop, false));
    board.Pieces.push_back(Piece(PieceType::Queen, false));
    board.Pieces.push_back(Piece(PieceType::King, false));
    board.Pieces.push_back(Piece(PieceType::Bishop, false));
    board.Pieces.push_back(Piece(PieceType::Knight, false));
    board.Pieces.push_back(Piece(PieceType::Rook, false));

    //board.enPassant = 36;

    bool prevlClick = false;

    int selectedSquare = -1;

    std::vector<int> posMoves;

    while (app.isOpen())
    {
        Event e;
        while (app.pollEvent(e))
        {
            if (e.type == Event::Closed)
                app.close();
        }
        bool Up = 0, Right = 0, Down = 0, Left = 0;
        if (Keyboard::isKeyPressed(Keyboard::Up)) Up = 1;
        if (Keyboard::isKeyPressed(Keyboard::Right)) Right = 1;
        if (Keyboard::isKeyPressed(Keyboard::Down)) Down = 1;
        if (Keyboard::isKeyPressed(Keyboard::Left)) Left = 1;

        bool lClick = sf::Mouse::isButtonPressed(sf::Mouse::Left);        

        if (lClick != prevlClick && lClick)
        {
            int x = floor(sf::Mouse::getPosition(app).x / 64);
            int y = floor(sf::Mouse::getPosition(app).y / 64);

            if (board.inCheck(board.wKing, false))
            {
                std::cout << "your in check, mate" << std::endl;
            }
            else
            {
                std::cout << "your not in check, mate" << std::endl;
            }

            if (x < 8 && y < 8)
            {
                if (selectedSquare == -1)
                {
                    posMoves = board.possibleMoves((y * 8) + x);
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
                            posMoves = board.possibleMoves((y * 8) + x);
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