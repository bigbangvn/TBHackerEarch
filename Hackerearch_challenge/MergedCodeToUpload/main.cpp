//
//  Taunt2.cpp
//  Hackerearch_challenge
//
//  Created by trongbangvp@gmail.com on 3/23/17.
//  Copyright © 2017 trongbangvp. All rights reserved.
//
#include <iostream>
#include <assert.h>
#include <vector>
#include <math.h>
#include <algorithm>
#include <sys/time.h>
#include <stdio.h>
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

static const int maxNumMove = 100;
static int DEPTH_SEARCH = 5;

void adjustDepthSearch(Cell* map)
{
    DEPTH_SEARCH = min(DEPTH_SEARCH, maxNumMove - _totalNumMove);
}

int calculateRemainPieceScore(Cell* map, int playerId)
{
    int score = 0;
    for(int i=0; i<N; ++i)
    {
        for(int j=0; j<M; ++j)
        {
            Cell* cell = &map[i*M + j];
            if(cell->playerId == playerId)
                score += cell->pieceType;
        }
    }
    return score;
}

//return percent
float calculateCentreControlScore(Cell* map, int playerId)
{
    int total = 0;
    int playerControl = 0;
    for(int i=2; i<N-2; ++i)
    {
        for(int j=1; j<M-1; ++j)
        {
            Cell* cell = &map[i*M + j];
            if(cell->playerId != 0)
            {
                ++total;
                if(cell->playerId == playerId)
                {
                    ++playerControl;
                }
            }
        }
    }
    return ((float)playerControl)/total;
}

bool possibleInDanger(Cell* map, Position&& cellPos)
{
    if(isValidPosition2(cellPos))
        return false;
    Cell* cellToCheck = &map[cellPos.row*M + cellPos.col];
    int opponentId = getOpponentId(cellToCheck->playerId);
    bool foundEnemy = false;
    for(int i = 2; i>=2; --i)
    {
        for(int j = -2; j<=2; ++j)
        {
            if(i != 0 || j != 0)
            {
                int row = cellPos.row + i;
                int col = cellPos.col + j;
                Position pos{row, col};
                if(isValidPosition2(pos))
                {
                    Cell* cell = &map[row*M + col];
                    if(cell->playerId == opponentId)
                    {
                        foundEnemy = true;
                        break;
                    }
                }
            }
        }
    }
    return foundEnemy;
}

int evaluateMap(Cell* map, int playerId)
{
    int opponentId = getOpponentId(playerId);
    
    //Score evaluation
    //Player don't have move: -inf
    //Enemy don't have move: +inf
    
    //Num player's piece score - enemy's piece score (score depends on piece type)
    int score1 = 0;
    
    //Num piece of Player which can eat enemy's piece
    int score2 = 0;
    
    //Num piece of Enemy that can eat player's piece
    int score3 = 0;
    
    bool playerHasMove = false;
    bool enemyHasMove = false;
    for(int i=0; i<N; ++i)
    {
        for(int j=0; j<M; ++j)
        {
            Cell* cell = &map[i*M + j];
            if(cell->playerId != 0)
            {
                vector<Move> thisCellMoves;
                findAllMoveOfACell(i, j, map, thisCellMoves);
                if(cell->isOwner(playerId))
                {
                    if(thisCellMoves.size() > 0)
                        playerHasMove = true;
                    for(int k = 0; k<thisCellMoves.size(); ++k)
                    {
                        Move* mv = &thisCellMoves[i];
                        if(mv->eat)
                            ++score2;
                    }
                } else if(cell->isOwner(opponentId))
                {
                    if(thisCellMoves.size() > 0)
                        enemyHasMove = true;
                    for(int k = 0; k<thisCellMoves.size(); ++k)
                    {
                        Move* mv = &thisCellMoves[i];
                        if(mv->eat)
                            ++score3;
                    }
                }
            }
        }
    }
    if(!playerHasMove)
    {
        return INT_MIN;
    }
    if(!enemyHasMove)
    {
        return INT_MAX;
    }
    score1 = calculateRemainPieceScore(map, playerId) - calculateRemainPieceScore(map, opponentId);
    
    //protected score: pieces was protected by others
    int pieceCount = 0;
    int protectedPieceCount = 0;
    Cell testMap[M*N*sizeof(Cell)];
    for(int i=0; i<N; ++i)
    {
        for(int j=0; j<M; ++j)
        {
            if(possibleInDanger(map, {i,j}))
            {
                Cell* cell = &map[i*M + j];
                if(cell->playerId == playerId)
                {
                    ++pieceCount;
                    
                    //Simulate that this piece be ate
                    memcpy(testMap, map, _mapSizeInByte);
                    cell = &testMap[i*M + j];
                    cell->playerId = opponentId;
                    if(checkCanEat(testMap, playerId, {i,j}))
                    {
                        ++protectedPieceCount;
                    }
                }
            }
        }
    }
    float score4 = 0;
    if(pieceCount > 0)
    {
        score4 = ((float)protectedPieceCount) / pieceCount;
    }
    //float score5 = calculateCentreControlScore(map, playerId);
    
    return score1*1000 + score2*100 - score3*400 + score4*20;// + score5*5;
}

static Move _bestMove = getInvalidMove();

int minimax(Cell* map, int depth, int maximizingPlayer, int playerId)
{
    if(depth > 2)
        printf("%s: %d\n", __func__, depth);
    if(depth <= 0)
    {
        return evaluateMap(map, playerId);
    }
    
    int bestValue;
    int value;
    Move* bestMove = NULL;
    vector<Move> moves;
    if(maximizingPlayer)
    {
        bestValue = INT_MIN;
        int depthGain = DEPTH_SEARCH - depth;
        findAllMove(map, playerId, moves);
        if(depth > 2)
        {
            printf("Depth: %d Num move = %ld %p\n", depth, moves.size(), moves.data()); fflush(stdout);
            logMoves(moves);
        }
        if(moves.size() == 0) //player don't have any move
        {
            printf("Found way to lose\n");
            return INT_MIN;
        }
        Cell* nextMap = allocMap();
        for(int i = 0; i<moves.size(); ++i)
        {
            Move* mv = &moves[i];
            if(depth > 2)
                printf("Depth: %d chose move: [%d %d] -> [%d %d]\n", depth, mv->fromPos.row, mv->fromPos.col, mv->toPos.row, mv->toPos.col);
            fillNextMap(map, mv, nextMap);
            value = minimax(nextMap, depth - 1, false, playerId) + depthGain;
            if(depth > 2)
                printf("Depth: %d player: %d Move [%d %d] -> [%d %d], score: %d\n", depth, maximizingPlayer, mv->fromPos.row, mv->fromPos.col, mv->toPos.row, mv->toPos.col, value);
            if(bestValue < value)
            {
                bestValue = value;
                bestMove = mv;
            }
        }
        freeMap(nextMap);
    } else //minimizing player
    {
        bestValue = INT_MAX;
        findAllMove(map, getOpponentId(playerId), moves);
        if(depth > 2)
        {
            printf("Depth: %d Num move = %ld %p\n", depth, moves.size(), moves.data()); fflush(stdout);
            logMoves(moves);
        }
        if(moves.size() == 0) //Opponent don't have any move
        {
            printf("Found move to win\n");
            return INT_MAX;
        }
        Cell* nextMap = allocMap();
        for(int i = 0; i<moves.size(); ++i)
        {
            Move* mv = &moves[i];
            if(depth > 2)
                printf("Depth: %d chose move: [%d %d] -> [%d %d]\n", depth, mv->fromPos.row, mv->fromPos.col, mv->toPos.row, mv->toPos.col);
            fillNextMap(map, mv, nextMap);
            value = minimax(nextMap, depth - 1, true, playerId);
            if(depth > 2)
                printf("Depth: %d player: %d Move [%d %d] -> [%d %d], score: %d\n", depth, maximizingPlayer, mv->fromPos.row, mv->fromPos.col, mv->toPos.row, mv->toPos.col, value);
            if(bestValue > value)
            {
                bestValue = value;
                bestMove = mv;
            }
        }
        freeMap(nextMap);
    }
    if(!bestMove)
    {
        bestMove = &moves.front();
    }
    printf("Depth: %d Best score: %d for player: %d, move piece %d [%d %d] -> [%d %d]\n", depth, bestValue, maximizingPlayer, bestMove->pieceType, bestMove->fromPos.row ,bestMove->fromPos.col, bestMove->toPos.row, bestMove->toPos.col);
    _bestMove = *bestMove;
    return bestValue;
}

int alphaBeta(Cell* map, int depth, int alpha, int beta, int maximizingPlayer, int playerId)
{
    if(depth > 2)
        printf("%s: %d\n", __func__, depth);
    if(depth <= 0)
    {
        return evaluateMap(map, playerId);
    }
    int value;
    int bestValue;
    Move* bestMove = NULL;
    vector<Move> moves;
    if(maximizingPlayer)
    {
        bestValue = INT_MIN;
        int depthGain = DEPTH_SEARCH - depth;
        findAllMove(map, playerId, moves);
        if(depth > 2)
        {
            printf("Depth: %d Num move = %ld %p\n", depth, moves.size(), moves.data()); fflush(stdout);
            logMoves(moves);
        }
        if(moves.size() == 0) //player don't have any move
        {
            printf("Found way to lose\n");
            return INT_MIN;
        }
        Cell nextMap[N*M];
        for(int i = 0; i<moves.size(); ++i)
        {
            Move* mv = &moves[i];
            if(depth > 2)
                printf("Depth: %d chose move: [%d %d] -> [%d %d]\n", depth, mv->fromPos.row, mv->fromPos.col, mv->toPos.row, mv->toPos.col);
            fillNextMap(map, mv, nextMap);
            value = alphaBeta(nextMap, depth - 1, alpha, beta, false, playerId) + depthGain;
            if(depth > 2)
                printf("Depth: %d player: %d Move [%d %d] -> [%d %d], score: %d\n", depth, maximizingPlayer, mv->fromPos.row, mv->fromPos.col, mv->toPos.row, mv->toPos.col, value);
            if(bestValue < value)
            {
                bestValue = value;
                bestMove = mv;
            }
            alpha = max(alpha, bestValue);
            if(alpha >= beta) //minimizer don't let maximizer come here
            {
                break;
            }
        }
    } else
    {
        bestValue = INT_MAX;
        findAllMove(map, getOpponentId(playerId), moves);
        if(depth > 2)
        {
            printf("Depth: %d Num move = %ld %p\n", depth, moves.size(), moves.data()); fflush(stdout);
            logMoves(moves);
        }
        if(moves.size() == 0) //Opponent don't have any move
        {
            printf("Found move to win\n");
            return INT_MAX;
        }
        Cell nextMap[N*M];
        for(int i = 0; i<moves.size(); ++i)
        {
            Move* mv = &moves[i];
            if(depth > 2)
                printf("Depth: %d chose move: [%d %d] -> [%d %d]\n", depth, mv->fromPos.row, mv->fromPos.col, mv->toPos.row, mv->toPos.col);
            fillNextMap(map, mv, nextMap);
            value = alphaBeta(nextMap, depth - 1, alpha, beta, true, playerId);
            if(depth > 2)
                printf("Depth: %d player: %d Move [%d %d] -> [%d %d], score: %d\n", depth, maximizingPlayer, mv->fromPos.row, mv->fromPos.col, mv->toPos.row, mv->toPos.col, value);
            if(bestValue > value)
            {
                bestValue = value;
                bestMove = mv;
            }
            beta = min(beta, bestValue);
            if(beta <= alpha) //maximizer don't let come here
            {
                break;
            }
        }
    }
    if(!bestMove)
    {
        bestMove = &moves.front();
    }
    _bestMove = *bestMove;
    printf("Depth: %d Best score: %d for player: %d, move piece %d [%d %d] -> [%d %d]\n", depth, bestValue, maximizingPlayer, bestMove->pieceType, bestMove->fromPos.row ,bestMove->fromPos.col, bestMove->toPos.row, bestMove->toPos.col);
    return bestValue;
}

long long getTimeMsec()
{
    struct timeval tp;
    gettimeofday(&tp, NULL);
    long long mslong = (long long) tp.tv_sec * 1000L + tp.tv_usec / 1000L;
    return mslong;
}

int main(int argc, const char * argv[]) {
    printf("START\n");
    initMap();
    getInput();
    adjustDepthSearch(_map);
    
    long long startTime = getTimeMsec();
#if 1
    alphaBeta(_map, DEPTH_SEARCH, INT_MIN, INT_MAX, true, _playerId);
#else
    minimax(_map, DEPTH_SEARCH, true, _playerId);
#endif
    printf("Elapsed time: %lld msec\n", getTimeMsec() - startTime);
    
    cout << _bestMove.fromPos.row <<" " <<_bestMove.fromPos.col <<endl <<_bestMove.toPos.row <<" " <<_bestMove.toPos.col <<endl;
    printf("FINISHED\n");
    return 0;
}
