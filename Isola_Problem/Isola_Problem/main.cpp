//
//  main.cpp
//  Isola_Problem
//
//  Created by trongbangvp@gmail.com on 3/19/17.
//  Copyright © 2017 trongbangvp. All rights reserved.
//

/*
 * BangNT
 * My strategy
 * There are 2 bound level around each player, call R1, R2
 * 1. I should remove block in R1, R2 of the enemy, but should not in R1, R2 of me, except some case: Kill enemy; I don't have direct way to move to this tile.
 * 2. Where i should move? Move to tile where i have much number of tile in R1
 *
 *
 * I will test my program by run and switch the player_id. When my Algorithm vs the default Bot, the number of move should be minimum to determine who win.
 */

#include <iostream>
#include <vector>
#include <assert.h>

using namespace std;
#define CLAMP(a,min,max) a<min? min:(a>max? max:a)

#define TEST_RUN_ON_LOCAL 0

#if !TEST_RUN_ON_LOCAL
#define printf(...)
#endif

// Size of grid
#define N 7
static int grid[N][N];
static int player_id;

void showMap()
{
#if TEST_RUN_ON_LOCAL
    for(int i = 0; i<N; ++i)
    {
        for(int j = 0; j<N; ++j)
        {
            printf(" %d", grid[i][j]);
        }
        printf("\n");
    }
#endif
}

class Position {
public:
    int x, y;
    
    Position(int ax, int ay) {
        x = ax;
        y = ay;
    }
};

bool is_neighbour(int px, int py, int i, int j) {
    if (px == i  &&  py == j)
        return false;
    else if (abs(px-i) <= 1 && abs(py-j) <= 1)
        return true;
    return false;
}

bool solveByDefault();
bool SolveByBangNT();

int main(int argc, const char * argv[]) {
    int i, j;
    
    // Scan previous state
    for(i=0; i<N; i++) {
        for(j=0; j<N; j++) {
            cin >> grid[i][j];
        }
    }
    
    // Scan previous player id
    cin >> player_id;
    
#if TEST_RUN_ON_LOCAL
    int initialPlayerId = player_id;
    int numMove = 0;
    while(true)
    {
        bool finished = false;
        if(player_id == initialPlayerId)
        {
            finished = SolveByBangNT();
        } else
        {
            finished = solveByDefault();
        }
        printf("Finished move: %d \n", ++numMove);
        if(finished)
            break;
        
        //Change player to test
        if(player_id == 2)
            player_id = 1;
        else
            player_id = 2;
    }
#else
    SolveByBangNT();
#endif
    return 0;
}

//Default bot, move randomly
bool solveByDefault()
{
    int i, j;
    int px = -1, py = -1;
    
    // Get the position of player
    for(i=0; i<N; i++) {
        for(j=0; j<N; j++) {
            if(grid[i][j] == player_id) {
                px = i;
                py = j;
            }
        }
    }
    printf("Player: %d at [%d %d]\n", player_id, px, py);
    
    // Get possible next positions of player
    vector<Position> moves;
    
    for(i=0; i<N; i++) {
        for(j=0; j<N; j++) {
            
            if(grid[i][j] != 0)
                continue;
            
            if(is_neighbour(px, py, i, j)) {
                moves.push_back(Position(i,j));
            }
        }
    }
    
    int totMoves = moves.size();
    if(totMoves == 0) {
        printf("Player: %d LOST\n", player_id);
        return true;
    }
    
    // Choose next position of player randomly
    srand(time(NULL));
    int id = rand() % totMoves;
    int next_px = moves[id].x, next_py = moves[id].y;
    
    // Output the next position of player
    cout << next_px << ' ' << next_py << endl;
    
    
    // Move player to next position
    grid[px][py] = 0;
    grid[next_px][next_py] = player_id;
    
    // Get posibles position of squares to remove
    moves.clear();
    
    for(i=0; i<N; i++) {
        for(j=0; j<N; j++) {
            if(grid[i][j] == 0) {
                moves.push_back(Position(i,j));
            }
        }
    }
    
    totMoves = moves.size();
    if(totMoves == 0) {
        assert(0); //Error, shouldn't come here
    }
    
    // Randomly choose a position of a sqaure to remove
    id = rand() % totMoves;
    
    //update removed square to map
    grid[moves[id].x][moves[id].y] = -1;
    
    // Output the position of square to remove
    cout << moves[id].x << ' ' << moves[id].y << endl;
    
    showMap();
    return false;
}

bool SolveByBangNT()
{
    int i, j;
    int px = -1, py = -1; //player pos
    int ex = -1, ey = -1; //enemy pos
    int row, col;
    
    // Get the position of player
    bool found = false;
    for(i=0; i<N; i++) {
        for(j=0; j<N; j++) {
            if(grid[i][j] == player_id) {
                px = i;
                py = j;
                found = true;
                break;
            }
        }
    }
    assert(found);
    printf("Player: %d at [%d %d]\n", player_id, px, py);
    
    //Get the enemy's position
    found = false;
    int enemyId = player_id == 1 ? 2:1;
    assert(enemyId != player_id);
    for(i=0; i<N; i++) {
        for(j=0; j<N; j++) {
            if(grid[i][j] == enemyId) {
                ex = i;
                ey = j;
                found = true;
                break;
            }
        }
    }
    assert(found);
    printf("Enemy at [%d %d]\n", ex, ey);
    
    vector<Position> playerRound1;
    for(i=-1; i<=1; ++i)
    {
        for(j=-1; j<=1; ++j)
        {
            if(i || j) //not at center position
            {
                row = CLAMP(px + i, 0, N-1);
                col = CLAMP(py + j, 0, N-1);
                if(grid[row][col] == 0)
                {
                    playerRound1.push_back(Position(row, col));
                }
            }
        }
    }
    
    //Pick a random position to move
    size_t n = playerRound1.size();
    if(n <= 0)
    {
        printf("LOST in %s\n", __func__);
        return true;
    }
    srand (time(NULL));
    int randomIdx = rand() % n;
    int next_px = playerRound1[randomIdx].x, next_py = playerRound1[randomIdx].y;
    // Output the next position of player
    cout << next_px << ' ' << next_py << endl;
    
    // Move player to next position
    grid[px][py] = 0;
    grid[next_px][next_py] = player_id;
    
    vector<Position> enemyRound1;
    for(i=-1; i<=1; ++i)
    {
        for(j=-1; j<=1; ++j)
        {
            if(i || j) //not at center position
            {
                row = CLAMP(ex + i, 0, N-1);
                col = CLAMP(ey + j, 0, N-1);
                if(grid[row][col] == 0)
                {
                    enemyRound1.push_back(Position(row, col));
                }
            }
        }
    }
    
    //Pick a random square to remove
    n = enemyRound1.size();
    srand (time(NULL));
    randomIdx = rand() % n;
    int removePositionRow = enemyRound1[randomIdx].x;
    int removePositionCol = enemyRound1[randomIdx].y;
    
    //update removed square to map
    grid[removePositionRow][removePositionCol] = -1;
    
    // Output the position of square to remove
    cout << removePositionRow << ' ' << removePositionCol << endl;
    showMap();
    return false;
}

