#include <iostream>
#include <fstream>
#include <string>
#include "logic.h"

using std::cout, std::endl, std::ifstream, std::string;

/**
 * TODO: Student implement this function
 * Load representation of the dungeon level from file into the 2D map.
 * Calls createMap to allocate the 2D array.
 * @param   fileName    File name of dungeon level.
 * @param   maxRow      Number of rows in the dungeon table (aka height).
 * @param   maxCol      Number of columns in the dungeon table (aka width).
 * @param   player      Player object by reference to set starting position.
 * @return  pointer to 2D dynamic array representation of dungeon map with player's location., or nullptr if loading fails for any reason
 * @updates  maxRow, maxCol, player
 */
char** loadLevel(const string& fileName, int& maxRow, int& maxCol, Player& player) {
    ifstream fin;
    bool poss1 = false;
    bool poss2 = false;
    fin.open(fileName);
    if (!fin.is_open()) return nullptr;
    fin >> maxRow >> maxCol >> player.row >> player.col;
    if (fin.fail()) return nullptr;
    if (maxRow < 1 || maxCol < 1 || player.row < 0 || player.row >= maxRow || player.col < 0 || player.col >= maxCol) return nullptr;
    if (maxRow > INT32_MAX/maxCol || maxCol > INT32_MAX/maxRow) return nullptr;
    char** map = createMap(maxRow,maxCol);
    char x;
    for (int row=0; row<maxRow; ++row) {
        for (int col=0; col<maxCol; ++col) {
            fin >> x;
            if (fin.fail()) {
                deleteMap(map,maxRow);
                return nullptr;
            }
            if (!(x == TILE_AMULET || x == TILE_DOOR || x == TILE_EXIT || x == TILE_MONSTER || x == TILE_OPEN || x == TILE_PILLAR || x == TILE_TREASURE)) {
                deleteMap(map,maxRow);
                return nullptr;
            }
            if (x == TILE_DOOR) poss1 = true;
            if (x == TILE_EXIT) poss2 = true;
            map[row][col] = x;
        }
    }
    fin >> x;
    if (!fin.fail()) {
        deleteMap(map,maxRow);
        return nullptr;
    }
    fin.close();
    if (poss1!=true && poss2!=true) {
        deleteMap(map,maxRow);
        return nullptr;
    }
    map[player.row][player.col] = TILE_PLAYER;
    return map;
}

/**
 * TODO: Student implement this function
 * Translate the character direction input by the user into row or column change.
 * That is, updates the nextRow or nextCol according to the player's movement direction.
 * @param   input       Character input by the user which translates to a direction.
 * @param   nextRow     Player's next row on the dungeon map (up/down).
 * @param   nextCol     Player's next column on dungeon map (left/right).
 * @updates  nextRow, nextCol
 */
void getDirection(char input, int& nextRow, int& nextCol) {
    switch (input) {
        case MOVE_UP :
            nextRow--;
            break;
        case MOVE_DOWN :
            nextRow++;
            break;
        case MOVE_LEFT :
            nextCol--;
            break;
        case MOVE_RIGHT :
            nextCol++;
            break;
    }
}

/**
 * TODO: [suggested] Student implement this function
 * Allocate the 2D map array.
 * Initialize each cell to TILE_OPEN.
 * @param   maxRow      Number of rows in the dungeon table (aka height).
 * @param   maxCol      Number of columns in the dungeon table (aka width).
 * @return  2D map array for the dungeon level, holds char type.
 */
char** createMap(int maxRow, int maxCol) {
    if (maxRow <= 0 || maxCol <= 0) return nullptr;
    char** map = new char*[maxRow];
    for (int i=0; i<maxRow; ++i) {
        map[i] = new char[maxCol];
    }
    for (int row=0; row<maxRow; ++row) {
        for (int col=0; col<maxCol; ++col) {
            map[row][col] = TILE_OPEN;
        }
    }
    return map;
}

/**
 * TODO: Student implement this function
 * Deallocates the 2D map array.
 * @param   map         Dungeon map.
 * @param   maxRow      Number of rows in the dungeon table (aka height).
 * @return None
 * @update map, maxRow
 */
void deleteMap(char**& map, int& maxRow) {
    if (maxRow <= 0 || map == nullptr) {
        map = nullptr;
        maxRow = 0;
    }
    for (int i=0; i<maxRow; ++i) {
        delete [] map[i];
    }
    delete [] map;
    map = nullptr;
    maxRow = 0;
}

/**
 * TODO: Student implement this function
 * Resize the 2D map by doubling both dimensions.
 * Copy the current map contents to the right, diagonal down, and below.
 * Do not duplicate the player, and remember to avoid memory leaks!
 * You can use the STATUS constants defined in logic.h to help!
 * @param   map         Dungeon map.
 * @param   maxRow      Number of rows in the dungeon table (aka height), to be doubled.
 * @param   maxCol      Number of columns in the dungeon table (aka width), to be doubled.
 * @return  pointer to a dynamically-allocated 2D array (map) that has twice as many columns and rows in size.
 * @update maxRow, maxCol
 */
char** resizeMap(char** map, int& maxRow, int& maxCol) {
    if (map == nullptr || maxRow <= 0 || maxCol <= 0) return nullptr;
    char** newMap = createMap(maxRow*2,maxCol*2);
    for (int row=0; row<maxRow; row++) {
        for (int col=0; col<maxCol; col++) {
            char curr = map[row][col];
            if (curr == TILE_PLAYER) {
                newMap[row][col] = curr;
                curr = TILE_OPEN;
            } else {
                newMap[row][col] = curr;
            }
            newMap[row+maxRow][col+maxCol] = curr;
            newMap[row+maxRow][col] = curr;
            newMap[row][col+maxCol] = curr;
        }
    }
    int max = maxRow;
    deleteMap(map, maxRow);
    maxRow = 2 * max;
    maxCol = 2 * maxCol;
    return newMap;
}

/**
 * TODO: Student implement this function
 * Checks if the player can move in the specified direction and performs the move if so.
 * Cannot move out of bounds or onto TILE_PILLAR or TILE_MONSTER.
 * Cannot move onto TILE_EXIT without at least one treasure. 
 * If TILE_TREASURE, increment treasure by 1.
 * Remember to update the map tile that the player moves onto and return the appropriate status.
 * You can use the STATUS constants defined in logic.h to help!
 * @param   map         Dungeon map.
 * @param   maxRow      Number of rows in the dungeon table (aka height).
 * @param   maxCol      Number of columns in the dungeon table (aka width).
 * @param   player      Player object to by reference to see current location.
 * @param   nextRow     Player's next row on the dungeon map (up/down).
 * @param   nextCol     Player's next column on dungeon map (left/right).
 * @return  Player's movement status after updating player's position.
 * @update map contents, player
 */
int doPlayerMove(char** map, int maxRow, int maxCol, Player& player, int nextRow, int nextCol) {
    if (nextRow >= maxRow || nextCol >= maxCol || nextRow < 0 || nextCol < 0)  return STATUS_STAY;
    else if (map[nextRow][nextCol] == TILE_OPEN) {
        map[nextRow][nextCol] = TILE_PLAYER;
        map[player.row][player.col] = TILE_OPEN;
        player.row = nextRow;
        player.col = nextCol;
        return STATUS_MOVE;
    }
    else if (map[nextRow][nextCol] == TILE_MONSTER || map[nextRow][nextCol] ==  TILE_PILLAR) return STATUS_STAY;
    else if (map[nextRow][nextCol] == TILE_EXIT && player.treasure == 0) return STATUS_STAY;
    else if (map[nextRow][nextCol] == TILE_EXIT) {
        map[nextRow][nextCol] = TILE_PLAYER;
        map[player.row][player.col] = TILE_OPEN;
        player.row = nextRow;
        player.col = nextCol;
        return STATUS_ESCAPE;
    }
    else if (map[nextRow][nextCol] == TILE_DOOR) {
        map[nextRow][nextCol] = TILE_PLAYER;
        map[player.row][player.col] = TILE_OPEN;
        player.row = nextRow;
        player.col = nextCol;
        return STATUS_LEAVE;
    }
    else if (map[nextRow][nextCol] == TILE_TREASURE) {
        map[nextRow][nextCol] = TILE_PLAYER;
        map[player.row][player.col] = TILE_OPEN;
        player.row = nextRow;
        player.col = nextCol;
        player.treasure++;
        return STATUS_TREASURE;
    } else if (map[nextRow][nextCol] == TILE_AMULET) {
        map[nextRow][nextCol] = TILE_PLAYER;
        map[player.row][player.col] = TILE_OPEN;
        player.row = nextRow;
        player.col = nextCol;
        return STATUS_AMULET;
    } 
    return 0;
}

/**
 * TODO: Student implement this function
 * Update monster locations:
 * We check up, down, left, right from the current player position.
 * If we see an obstacle, there is no line of sight in that direction, and the monster does not move.
 * If we see a monster before an obstacle, the monster moves one tile toward the player.
 * We should update the map as the monster moves.
 * At the end, we check if a monster has moved onto the player's tile.
 * @param   map         Dungeon map.
 * @param   maxRow      Number of rows in the dungeon table (aka height).
 * @param   maxCol      Number of columns in the dungeon table (aka width).
 * @param   player      Player object by reference for current location.
 * @return  Boolean value indicating player status: true if monster reaches the player, false if not.
 * @update map contents
 */
bool doMonsterAttack(char** map, int maxRow, int maxCol, const Player& player) {
    bool poss = 1;
    bool att = 0;
    // check up
    for (int next = player.row; next>=0 && poss; --next) {
        if (map[next][player.col] == TILE_PILLAR) poss = 0;
        else if (map[next][player.col] == TILE_MONSTER) {
            map[next][player.col] = TILE_OPEN;
            map[next+1][player.col] = TILE_MONSTER;
            if (next+1 == player.row) att = 1;
            
        }
    }
    poss = 1;
    // check down
    for (int next = player.row; next<maxRow && poss; ++next) {
        if (map[next][player.col] == TILE_PILLAR) poss = 0;
        else if (map[next][player.col] == TILE_MONSTER) {
            map[next][player.col] = TILE_OPEN;
            map[next-1][player.col] = TILE_MONSTER;
            if (next-1 == player.row) att = 1;
            
        }
    }
    poss = 1;
    // check left
    for (int next = player.col; next>=0 && poss; --next) {
        if (map[player.row][next] == TILE_PILLAR) poss = 0;
        else if (map[player.row][next] == TILE_MONSTER) {
            map[player.row][next] = TILE_OPEN;
            map[player.row][next+1] = TILE_MONSTER;
            if (next+1 == player.col) att = 1;
            
        }
    }
    poss = 1;
    // check right
    for (int next = player.col; next<maxCol && poss; ++next) {
        if (map[player.row][next] == TILE_PILLAR) poss = 0;
        else if (map[player.row][next] == TILE_MONSTER) {
            map[player.row][next] = TILE_OPEN;
            map[player.row][next-1] = TILE_MONSTER;
            if (next-1 == player.col) att = 1;
            
        }
    }
    return att;
}
