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
#include "Common.hpp"

using namespace std;

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
    
    //Num piece that Player can eat
    int score2 = 0;
    
    //Num piece that Enemy can eat
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
            value = minimax(nextMap, depth - 1, false, playerId);
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
            value = alphaBeta(nextMap, depth - 1, alpha, beta, false, playerId);
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
