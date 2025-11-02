#include <iostream>
#include <cmath>

struct Position {
    int row;
    int col;
    
    int manhattan(const Position& other) const {
        return abs(col - other.col) + abs(row - other.row);
    }
    
    bool operator==(const Position& other) const {
        return row == other.row && col == other.col;
    }
    
    bool operator!=(const Position& other) const {
        return !(*this == other);
    }
    
    Position operator+(const Position& other) const {
        return Position {
            row + other.row,
            col + other.col
        };
    }
    
    Position& clamp(int min_val, int max_val) {
        row = std::max(min_val, std::min(row, max_val));
        col = std::max(min_val, std::min(col, max_val));
        return *this;
    }
};

int main() {
    constexpr int ROWS = 5;
    constexpr int COLS = 5;

    char player = 'O';
    Position pp = { 0, 0 };
    
    char goal = 'X';
    Position gp = { ROWS - 1, COLS - 1 };

    
    char grid[ROWS][COLS] = {
        { '-', '-', '-', '-', '-' },
        { '-', '-', '-', '-', '-' },
        { '-', '-', '-', '-', '-' },
        { '-', '-', '-', '-', '-' },
        { '-', '-', '-', '-', '-' }
    };
    
    while (pp != gp) {
        Position gridP = {0,0};
        for (int i = 0; i < ROWS; i++) {
            gridP.row = i;
            for (int j = 0; j < COLS; j++) {
                gridP.col = j;
                
                char symbol = '-';
                if (gridP == pp) {
                    symbol = player;
                } else if (gridP == gp) {
                    symbol = goal;
                }
                
                grid[gridP.row][gridP.col] = symbol;
                std::cout << symbol << " ";
            }
            std::cout << std::endl;
        }
        
        std::cout << "==================" << std::endl;
        
        const Position offsets[4] = {{0,-1},{0,1},{-1,0},{1,0}};
        Position best_pos = { -1000, -1000 };
        int best_distance = 100000;
        
        for (const Position& offset : offsets) {
            const Position new_pos = (pp + offset).clamp(0, ROWS);
            const int distance = new_pos.manhattan(gp);
            if (distance < best_distance) {
                best_pos = new_pos;
                best_distance = distance;
            }
        }
        
        pp = best_pos;
    }

    return 0;
}