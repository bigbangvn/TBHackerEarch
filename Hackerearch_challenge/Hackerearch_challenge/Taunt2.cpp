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
#include "Taunt2.hpp"
#include "Common.hpp"

using namespace std;

int evaluateMap(Cell* map, int playerId)
{
    int opponentId = getOpponentId(playerId);
    
    //Score evaluation
    //Player don't have move: -inf
    //Enemy don't have move: +inf
    
    //Num player's piece - enemy's piece
    int score1 = 0;
    
    //Num piece that Player can eat
    int score2 = 0;
    
    //Num piece that Enemy can eat
    int score3 = 0;
    
    vector<Move> playerMoves;
    findAllMove(map, playerId, playerMoves);
    if(playerMoves.size() == 0)
    {
        return INT_MIN;
    }
    vector<Move> enemyMoves;
    findAllMove(map, opponentId, enemyMoves);
    if(enemyMoves.size() == 0)
    {
        return INT_MAX;
    }
    score1 = countNumPiece(map, playerId) - countNumPiece(map, opponentId);
    score2 = findMaxEat(playerMoves);
    score3 = findMaxEat(enemyMoves);
    
    return score1*10 + score2*10 + score3*5;
}

static Move _bestMove = getInvalidMove();

int minimax(Cell* map, int depth, int maximizingPlayer, int playerId)
{
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
        printf("Num move = %ld\n", moves.size()); fflush(stdout);
        if(moves.size() == 0) //player don't have any move
        {
            printf("Found way to lose\n");
            return INT_MIN;
        }
        Cell* nextMap = allocMap();
        for(int i = 0; i<moves.size(); ++i)
        {
            Move* mv = &moves[i];
#if DEBUG
            if(mv->pieceType == 2 && mv->toPos.row == 0 && mv->toPos.col == 0)
            {
                assert(0);
            }
#endif
            fillNextMap(map, mv, nextMap);
            value = minimax(nextMap, depth - 1, false, playerId);
            if(bestValue <= value)
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
        printf("Num movex = %ld\n", moves.size()); fflush(stdout);
        if(moves.size() == 0) //Opponent don't have any move
        {
            printf("Found move to win\n");
            return INT_MAX;
        }
        Cell* nextMap = allocMap();
        for(int i = 0; i<moves.size(); ++i)
        {
            Move* mv = &moves[i];
#if DEBUG
            if(mv->pieceType == 2 && mv->toPos.row == 0 && mv->toPos.col == 0)
            {
                printf("Num move = %ld\n", moves.size()); fflush(stdout);
                logMoves(moves);
                assert(0);
            }
#endif
            fillNextMap(map, mv, nextMap);
            value = minimax(nextMap, depth - 1, true, playerId);
            bestValue = min(bestValue, value);
            if(bestValue >= value)
            {
                bestValue = value;
                bestMove = mv;
            }
        }
        printf("\n");
        freeMap(nextMap);
    }
    assert(isValidPosition2(bestMove->fromPos));
    assert(isValidPosition2(bestMove->toPos));
    showMap(map);
    printf("Best score: %d for player: %d, move piece %d [%d %d] -> [%d %d]\n", bestValue, maximizingPlayer, bestMove->pieceType, bestMove->fromPos.row ,bestMove->fromPos.col, bestMove->toPos.row, bestMove->toPos.col);
    _bestMove = *bestMove;
    return bestValue;
}

int main(int argc, const char * argv[]) {
    printf("START\n");
    initMap();
    getInput();
    showMap(_map);
    int valForCurrentState = minimax(_map, 3, true, _playerId);
    cout << _bestMove.fromPos.row <<" " <<_bestMove.fromPos.col <<endl <<_bestMove.toPos.row <<" " <<_bestMove.toPos.col <<endl;
    _bestMove.showDescription();
    printf("FINISHED: %d\n", valForCurrentState);
    return 0;
}
