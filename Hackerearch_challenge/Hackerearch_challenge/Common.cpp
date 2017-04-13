//
//  Common.cpp
//  Hackerearch_challenge
//
//  Created by trongbangvp@gmail.com on 3/23/17.
//  Copyright © 2017 trongbangvp. All rights reserved.
//

#include "Common.hpp"
#include <stdio.h>
#include <iostream>
#include <assert.h>
#include <math.h>
#include <algorithm>

Cell* _map = NULL;
const int _mapSizeInByte = N*M*sizeof(Cell);
int _totalNumMove = 0;
int _playerId = -1;
int _enemyId = -1;

bool isValidPosition(int row, int col)
{
    return row >= 0 && row < N && col >= 0 && col < M;
}
bool isValidPosition2(Position& pos)
{
    return pos.row >= 0 && pos.row < N && pos.col >= 0 && pos.col < M;
}

Cell* allocMap()
{
    return (Cell*)
    //malloc(_mapSizeInByte);
    calloc(_mapSizeInByte, 1);
}

void initMap()
{
    _map = allocMap();
}

Move getInvalidMove()
{
    return Move(0, 0, 0, {0,0}, {0,0}, {0, 0}, -1);
}

bool checkMoveOver(vector<Move>& moves, Position&& pos)
{
    bool moveOver = false;
    for(int i = 0; i<moves.size(); ++i)
    {
        Move* mv = &moves[i];
        if(mv->fromPos.isEqual(pos) ||
           mv->overPos.isEqual(pos) ||
           mv->toPos.isEqual(pos)) {
            moveOver = true;
            break;
        }
    }
    return moveOver;
}

void freeMap(Cell* map)
{
    free(map);
}

void getInput()
{
    for(int i=0; i<N; ++i)
    {
        int j = 0;
        while (j < M) {
            int a = getchar();
            if(!isspace(a))
            {
                Cell* cell = &_map[i*M + j];
                cell->playerId = a - '0';
                cell->pieceType = getchar() - '0';
                cell->direction = getchar() - '0';
                cell->showDescription();
                ++j;
            }
        }
        printf("\n");
    }
    printf("\n");
    cin >> _totalNumMove;
    cin >> _playerId;
    if(_playerId == 1)
        _enemyId = 2;
    else
        _enemyId = 1;
    printf("Total move: %d \nPlayer id: %d\n", _totalNumMove, _playerId);
    fflush(stdout);
}

void fillNextMap(Cell* currentMap, Move* move, Cell* nextMap)
{
    assert(move->validMove);
    memcpy(nextMap, currentMap, _mapSizeInByte);
    assert(_mapSizeInByte == N*M*sizeof(Cell));
    
    Cell* fromCell = &nextMap[move->fromPos.row*M + move->fromPos.col];
    Cell* toCell = &nextMap[move->toPos.row*M + move->toPos.col];
    if(move->eat)
    {
        if(isValidPosition2(move->overPos))
        {
            Cell* overCell = &nextMap[move->overPos.row*M + move->overPos.col];
            overCell->clean();
        }
    }
    toCell->clone(*fromCell);
    if(move->changeDirection)
    {
        toCell->invertDirection();
    }
    fromCell->clean();
}

int getOpponentId(int playerId)
{
    if(playerId == 1)
        return 2;
    else
        return 1;
}

vector<Move> findAllMoveOfACell(int i, int j, Cell* map, vector<Move>& moves)
{
    moves.clear();
    Cell* cellToMove = &map[i*M + j];
    int moverId = cellToMove->playerId;
    int opponentId = getOpponentId(moverId);
    
    int targetRow, targetCol;
    switch (cellToMove->pieceType) {
        case 1:
        {
            for(int k=-1; k<=1; ++k)
            {
                for(int l=-1; l<=1; ++l)
                {
                    if(abs(k) != abs(l))
                    {
                        targetRow = i+k;
                        targetCol = j+l;
                        //Note: this block of code is identical in many place (should create function)
                        if(isValidPosition(targetRow, targetCol))
                        {
                            Cell* targetCell = &map[targetRow*M + targetCol];
                            if(targetCell->playerId != moverId)
                            {
                                int eatEnemyPiece = (targetCell->playerId == opponentId);
                                moves.push_back(Move(1, eatEnemyPiece, 0, {i,j}, {targetRow, targetCol}, {-1,-1}, cellToMove->pieceType));
                            }
                        }
                    }
                }
            }
        }
            break;
        case 2:
        {
            //Check can move to horizontally adjacent
            targetRow = i;
            int rowMoveOver = -1;
            
            //Check move horizontally
            for(int k=-1; k<=1; ++k)
            {
                if(k != 0)
                {
                    targetCol = j+k;
                    if(isValidPosition(targetRow, targetCol))
                    {
                        Cell* targetCell = &map[targetRow*M + targetCol];
                        if(targetCell->playerId != moverId)
                        {
                            bool eatEnemyPiece = (targetCell->playerId == opponentId);
                            moves.push_back(Move(1, eatEnemyPiece, 0, {i,j}, {targetRow,targetCol}, {-1,-1}, cellToMove->pieceType));
                        }
                    }
                }
            }
            
            //Check move forward
            targetCol = j;
            if(cellToMove->direction == DIRECTION_UPWARD)
            {
                assert(i>=1);
                targetRow = i - 2;
                rowMoveOver = i - 1;
            } else if(cellToMove->direction == DIRECTION_DOWNWARD)
            {
                assert(i<=N-2);
                targetRow = i + 2;
                rowMoveOver = i + 1;
            } else
            {
                assert(0);
            }
            
            //Check if there are enemy on the way
            int numEat = 0;
            Cell* moveOverCell = &map[rowMoveOver*M + targetCol];
            if(moveOverCell->playerId == opponentId)
                ++numEat;

            assert(isValidPosition(rowMoveOver, targetCol));
            if(isValidPosition(targetRow, targetCol))
            {
                Cell* targetCell = &map[targetRow*M + targetCol];
                if(targetCell->playerId != moverId)
                {
                    if(targetCell->playerId == opponentId)
                        ++numEat;
                    bool changeDirection = false;
                    if(targetRow == 0 || targetRow == N-1)
                    {
                        changeDirection = true;
                    }
                    moves.push_back(Move(1, numEat, changeDirection, {i,j}, {targetRow,targetCol}, {rowMoveOver,targetCol}, cellToMove->pieceType));
                }
            } else //move to its own position and change direction
            {
                moves.push_back(Move(1, numEat, 1, {i,j}, {i,j}, {rowMoveOver, targetCol}, cellToMove->pieceType));
                printf("Piece 2 valid move: %d %d -> %d %d\n", i, j, i, j);
            }
        }
            break;
        case 3:
        {
            int currPosi;
            int currPosj;
            int startVi;
            if(cellToMove->direction == DIRECTION_UPWARD)
            {
                startVi = -1;
                if(i == 0) //should change direction previously
                {
                    assert(0);
                }
            } else if(cellToMove->direction == DIRECTION_DOWNWARD)
            {
                startVi = 1;
                if(i == N-1) //should change direction previously
                {
                    assert(0);
                }
            } else
            {
                assert(0);
            }
            
            int vi, vj;
            
            //2 branch vj = +-1
            vi = startVi;
            vj = -1;
            currPosi = i + vi;
            currPosj = j + vj;
            bool changeDirection = false;
            int numEat = 0;
            if(isValidPosition(currPosi, currPosj))
            {
                int moveOveri = currPosi;
                int moveOverj = currPosj;
                
                //Check if have enemy on the way -> eat
                Cell* moveOverCell = &map[moveOveri*M + moveOverj];
                if(moveOverCell->playerId == opponentId)
                    ++numEat;
                
                //Check reflection
                if((vi < 0 && currPosi <= 0) ||
                   (vi > 0 && currPosi >= N-1))
                {
                    vi = -vi;
                    changeDirection = true;
                }
                if(currPosj == 0) // vj > 0
                {
                    vj = -vj;
                }
                currPosi += vi;
                currPosj += vj;
                if((vi < 0 && currPosi == 0) ||
                   (vi > 0 && currPosi == N-1))
                {
                    changeDirection = true;
                }
                assert(isValidPosition(currPosi, currPosj));
                if(currPosi == i && currPosj == j) //move to its own position
                {
                    moves.push_back(Move(1, numEat, changeDirection, {i,j}, {i,j}, {moveOveri, moveOverj}, cellToMove->pieceType));
                } else
                {
                    Cell* targetCell = &map[currPosi*M + currPosj];
                    if(targetCell->playerId != moverId)
                    {
                        if(targetCell->playerId == opponentId)
                            ++numEat;
                        moves.push_back(Move(1, numEat, changeDirection, {i,j}, {currPosi,currPosj}, {moveOveri, moveOverj}, cellToMove->pieceType));
                    }
                }
                assert(currPosi > 0 || (currPosi == 0 && changeDirection));
            }
            
            vi = startVi;
            vj = 1;
            currPosi = i + vi;
            currPosj = j + vj;
            changeDirection = false;
            numEat = 0;
            if(isValidPosition(currPosi, currPosj))
            {
                int moveOveri = currPosi;
                int moveOverj = currPosj;
                
                //Check if have enemy on the way -> eat
                Cell* moveOverCell = &map[moveOveri*M + moveOverj];
                if(moveOverCell->playerId == opponentId)
                    ++numEat;
                
                //Check reflection
                if((vi < 0 && currPosi <= 0) ||
                   (vi > 0 && currPosi >= N-1))
                {
                    vi = -vi;
                    changeDirection = true;
                }
                if(currPosj == M-1) // vj > 0
                {
                    vj = -vj;
                }
                currPosi += vi;
                currPosj += vj;
                if((vi < 0 && currPosi == 0) ||
                   (vi > 0 && currPosi == N-1))
                {
                    changeDirection = true;
                }
                
                assert(isValidPosition(currPosi, currPosj));
                if(currPosi == i && currPosj == j) //move to its own position
                {
                    moves.push_back(Move(1, numEat, changeDirection, {i,j}, {i,j}, {moveOveri, moveOverj}, cellToMove->pieceType));
                } else
                {
                    Cell* targetCell = &map[currPosi*M + currPosj];
                    if(targetCell->playerId != moverId)
                    {
                        if(targetCell->playerId == opponentId)
                            ++numEat;
                        moves.push_back(Move(1, numEat, changeDirection, {i,j}, {currPosi,currPosj}, {moveOveri, moveOverj}, cellToMove->pieceType));
                    }
                }
                assert(currPosi > 0 || (currPosi == 0 && changeDirection));
            }
        }
            break;
        default:
            break;
    }
    return moves;
}

void findAllMove(Cell* map, int moverId, vector<Move>& moves)
{
    moves.clear();
    for(int i=0; i<N; ++i)
    {
        for(int j=0; j<M; ++j)
        {
            Cell* cell = &map[i*M + j];
            if(cell->isOwner(moverId))
            {
                vector<Move> thisCellMoves;
                findAllMoveOfACell(i, j, map, thisCellMoves);
                moves.insert(moves.end(), thisCellMoves.begin(), thisCellMoves.end());
            }
        }
    }
    return;
}

bool checkCanEat(Cell* map, int moverId, Position&& opponentPos)
{
    for(int i=0; i<N; ++i)
    {
        for(int j=0; j<M; ++j)
        {
            Cell* cell = &map[i*M + j];
            if(cell->isOwner(moverId))
            {
                vector<Move> thisCellMoves;
                findAllMoveOfACell(i, j, map, thisCellMoves);
                if(checkMoveOver(thisCellMoves, {opponentPos.row, opponentPos.col}))
                    return true;
            }
        }
    }
    return false;
}

int countNumPiece(Cell* map, int playerId)
{
    int count = 0;
    for(int i=0; i<N; ++i)
    {
        for(int j=0; j<M; ++j)
        {
            Cell* cell = &map[i*M + j];
            if(cell->playerId == playerId)
                ++count;
        }
    }
    return count;
}

int findMaxEat(vector<Move> moves)
{
    int maxEat = 0;
    for(vector<Move>::iterator it = moves.begin(); it != moves.end(); ++it)
    {
        Move* mov = &(*it);
        maxEat = max(maxEat, mov->eat);
    }
    return maxEat;
}

void logMoves(vector<Move>& moves)
{
    for(vector<Move>::iterator it = moves.begin(); it != moves.end(); ++it)
    {
        Move* mv = &(*it);
        printf("[%d %d] -> [%d %d], ", mv->fromPos.row, mv->fromPos.col, mv->toPos.row, mv->toPos.col);
    }
    printf("\n");
}

void showMap(Cell* map)
{
    for(int i=0; i<N; ++i)
    {
        for(int j=0; j<M; ++j)
        {
            Cell* c = &map[i*M + j];
            c->showDescription();
        }
        printf("\n");
    }
}

void showMapWithIndent(Cell* map, int indent)
{
    for(int i=0; i<N; ++i)
    {
        for(int x = 0; x<indent; ++x)
            printf(" ");
        for(int j=0; j<M; ++j)
        {
            Cell* c = &map[i*M + j];
            c->showDescription();
        }
        printf("\n");
    }
}

