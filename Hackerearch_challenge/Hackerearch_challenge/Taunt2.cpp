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
    
    vector<Move> playerMoves = findAllMove(map, playerId);
    if(playerMoves.size() == 0)
    {
        return INT_MIN;
    }
    vector<Move> enemyMoves = findAllMove(map, opponentId);
    if(enemyMoves.size() == 0)
    {
        return INT_MAX;
    }
    score1 = countNumPiece(map, playerId) - countNumPiece(map, opponentId);
    score2 = findMaxEat(playerMoves);
    score3 = findMaxEat(enemyMoves);
    
    return score1*10 + score2*10 + score3*5;
}

int minimax(Cell* map, int depth, int maximizingPlayer, int playerId)
{
    if(depth <= 0)
    {
        return evaluateMap(map, playerId);
    }
    
    int bestValue;
    int value;
    Move* bestMove = NULL;
    if(maximizingPlayer)
    {
        bestValue = INT_MIN;
        vector<Move> moves = findAllMove(map, playerId);
        Cell* nextMap = allocMap();
        for(int i = 0; i<moves.size(); ++i)
        {
            Move* mv = &moves[i];
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
        vector<Move> opponentMoves = findAllMove(map, getOpponentId(playerId));
        Cell* nextMap = allocMap();
        for(int i = 0; i<opponentMoves.size(); ++i)
        {
            Move* mv = &opponentMoves[i];
            fillNextMap(map, mv, nextMap);
            value = minimax(nextMap, depth - 1, true, playerId);
            bestValue = min(bestValue, value);
            if(bestValue >= value)
            {
                bestValue = value;
                bestMove = mv;
            }
        }
        freeMap(nextMap);
    }
    printf("Best score: %d for move: [%d %d] -> [%d %d]\n", bestValue, bestMove->fromPos.row ,bestMove->fromPos.col, bestMove->toPos.row, bestMove->toPos.col);
    return bestValue;
}

int main(int argc, const char * argv[]) {
    printf("TAUNT GAME\n");
    initMap();
    getInput();
    showMap(_map);
    int valForCurrentState = minimax(_map, 4, true, _playerId);
    printf("FINISHED: %d\n", valForCurrentState);
    return 0;
}
