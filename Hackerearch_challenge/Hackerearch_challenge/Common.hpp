//
//  Common.hpp
//  Hackerearch_challenge
//
//  Created by trongbangvp@gmail.com on 3/23/17.
//  Copyright © 2017 trongbangvp. All rights reserved.
//

#ifndef Common_hpp
#define Common_hpp

#include <stdio.h>
#include <assert.h>
#include <vector>
#include <string.h>
#include <limits.h>
using namespace std;

#define SHOW_DEBUG 1
#if !SHOW_DEBUG
#define printf(...)
#endif

#define M 4     //cols
#define N 10    //rows

typedef enum StrategyType:int //This affect how we compare the evaluation of map
{
    STRATEGY_MAX_MOVE = 1,
    STRATEGY_MAX_PIECE
} StrategyType;

typedef enum Direction:int {
    DIRECTION_UPWARD = 0,
    DIRECTION_DOWNWARD = 1
} Direction;

typedef struct Cell {
    int playerId;
    int pieceType; //1,2,3
    int direction; //0:up, 1 downward
    void clean() //when was ate
    {
        playerId = 0;
        pieceType = 0;
        direction = 0;
    }
    void clone(Cell &c)
    {
        playerId = c.playerId;
        pieceType = c.pieceType;
        direction = c.direction;
    }
    void invertDirection()
    {
        if(direction == DIRECTION_UPWARD)
            direction = DIRECTION_DOWNWARD;
        else
            direction = DIRECTION_UPWARD;
    }
    bool isOwner(int playerIdToCheck)
    {
        return playerIdToCheck == playerId;
    }
    void showDescription()
    {
        printf("%d%d%d ", playerId, pieceType, direction);
    }
} Cell;

typedef struct Position {
    int row;
    int col;
    bool isEqual(Position& p)
    {
        return row == p.row && col == p.col;
    }
} Position;

typedef struct Move {
    bool validMove;
    int eat;
    bool changeDirection;
    Position fromPos;
    Position toPos;
    Position overPos;
    int pieceType;
    Move(bool _validMove, int _eat, bool _changeDirection, Position _fromPos, Position _toPos, Position _overPos, int _pieceType)
    {
        validMove = _validMove;
        eat = _eat;
        changeDirection = _changeDirection;
        fromPos = _fromPos;
        toPos = _toPos;
        overPos = _overPos;
        pieceType = _pieceType;
#if DEBUG
        int maxDistance = 0;
        if(pieceType == -1)
        {
            
        } else if(pieceType == 1)
        {
            maxDistance = 1;
        } else if(pieceType == 2)
        {
            maxDistance = 2;
        } else if(pieceType == 3)
        {
            maxDistance = 4;
        } else
        {
            assert(0);
        }
        assert(abs(_fromPos.row - _toPos.row) + abs(_fromPos.col - _toPos.col) <= maxDistance);
#endif
    }
    //return 1 if this is better than otherMove
    bool worseThan(Move& otherMove)
    {
        assert(validMove && otherMove.validMove);
        if(otherMove.eat > eat)
            return true;
        return false;
    }
    void showDescription()
    {
        printf("Move from [%d %d] -> [%d %d]\n", fromPos.row, fromPos.col, toPos.row, toPos.col);
    }
} Move;


//Game state
extern Cell* _map;
extern const int _mapSizeInByte;
extern int _totalNumMove;
extern int _playerId;
extern int _enemyId;

void initMap();
Move getInvalidMove();
Cell* allocMap();
void freeMap(Cell* map);
void getInput();
int getOpponentId(int playerId);
void fillNextMap(Cell* currentMap, Move* move, Cell* nextMap);
vector<Move> findAllMoveOfACell(int i, int j, Cell* map, vector<Move>& moves);
void findAllMove(Cell* map, int moverId, vector<Move>& moves);
bool checkCanEat(Cell* map, int moverId, Position&& opponentPos);
int countNumPiece(Cell* map, int playerId);
int findMaxEat(vector<Move> moves);
void logMoves(vector<Move>& moves);
void showMap(Cell* map);
void showMapWithIndent(Cell* map, int indent);
bool isValidPosition(int row, int col);
bool isValidPosition2(Position& pos);

#endif /* Common_hpp */
