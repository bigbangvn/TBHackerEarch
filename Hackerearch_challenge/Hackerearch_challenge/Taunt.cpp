//
//  Taunt.cpp
//  Hackerearch_challenge
//
//  Created by trongbangvp@gmail.com on 3/19/17.
//  Copyright © 2017 trongbangvp. All rights reserved.
//

#include <assert.h>
#include <vector>
#include <math.h>
#include <algorithm>
#include <iostream>
#include "Common.hpp"

using namespace std;

//Strategy: find the best state after DEPTH_SEARCH moves
//Algorithm: breadth first search with restricted depth

bool moveComparator(Move a, Move b)
{
    return a.worseThan(b);
}

Move getBestMove(vector<Move>& moves, int& bestIndex)
{
    assert(moves.size() > 0);
    sort(moves.begin(), moves.end(), moveComparator);
    return moves.back();
}

//Solve Taunt problem by breadth searching, with restricted depth
//Return to best move
Move solveTaunt(Cell* map, int searchDepth)
{
    printf("%s: %d\n", __func__, searchDepth);
    vector<Move> availableMove;
    
    //Find all move (and evaluate, but not neccessary)
    findAllMove(map, _playerId, availableMove);
    if(availableMove.size() == 0)
    {
        return getInvalidMove();
    }
    
    if(searchDepth <= 0) //reached restricted depth -> evaluate
    {
        int index;
        return getBestMove(availableMove, index);
    } else
    {
        vector<Move> bestNextMoves; //best move in next depth
        
        Cell* nextMap = allocMap();
        for(vector<Move>::iterator it = availableMove.begin(); it != availableMove.end(); ++it)
        {
            Move* aMove = &(*it);
            fillNextMap(map, aMove, nextMap);
            
            //Try all enemy move available
            swap(_playerId, _enemyId);
            vector<Move> enemyMoves;
            findAllMove(nextMap, _playerId, enemyMoves);
            
            
            swap(_playerId, _enemyId);
            
            
            Move bestMoveOfNext = solveTaunt(nextMap, searchDepth - 1);
            bestNextMoves.push_back(bestMoveOfNext);
        }
        freeMap(nextMap);
        
        if(bestNextMoves.size() == 0)
        {
            //just pick first(or random) from availableMove
            return availableMove.front();
        } else
        {
            int index = -1;
            getBestMove(bestNextMoves, index);
            assert(index >= 0 && index < availableMove.size());
            return availableMove[index];
        }
    }
}

//For testing move, look at Taunt2 use minimax algorithm
int main2(int argc, const char * argv[]) {
    printf("TAUNT GAME\n");    
    initMap();
    getInput();
    showMap(_map);
    
#if 0 //Test random
    vector<Move> availableMoves = findAllMove(_map);
    printf("Available moves:\n");
    logMoves(availableMoves);
    
    srand(time(NULL));
    int idx = rand()%availableMoves.size();
    Move mv = availableMoves[idx];
#else
    Move mv = solveTaunt(_map, 0);
#endif
    
    cout<<mv.fromPos.row<<" "<<mv.fromPos.col<<endl << mv.toPos.row <<" "<< mv.toPos.col <<endl;
    
    return 0;
}
