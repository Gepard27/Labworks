#include "../include/Game.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <random>
#include <algorithm>

Game::Game() 
    : mode(GameMode::SLAVE)
    , currentStrategy(Strategy::ORDERED)
    , width(0)
    , height(0)
    , shipCounts(4, 0)
    , gameStarted(false)
    , isMyTurn(false)
    , lastHitX(0)
    , lastHitY(0) {
}

bool Game::createGame(const std::string& mode) {
    if (mode == "master") {
        this->mode = GameMode::MASTER;
    } else if (mode == "slave") {
        this->mode = GameMode::SLAVE;
    } else {
        return false;
    }
    
    // Устанавливаем стандартные размеры поля
    width = 10;
    height = 10;

    shipCounts[0] = 4;  // 1-палубные
    shipCounts[1] = 3;  // 2-палубные
    shipCounts[2] = 2;  // 3-палубные
    shipCounts[3] = 1;  // 4-палубные

    initializeBoards();
    
    return true;
}

bool Game::setStrategy(const std::string& strategy) {
    if (strategy == "ordered") {
        currentStrategy = Strategy::ORDERED;
        return true;
    } else if (strategy == "custom") {
        currentStrategy = Strategy::CUSTOM;
        return true;
    }
    return false;
}

void Game::generateRandomShipPlacement() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint64_t> disW(0, width - 1);
    std::uniform_int_distribution<uint64_t> disH(0, height - 1);
    std::bernoulli_distribution disDir(0.5);

    myShips.clear();
    enemyShips.clear();
    initializeBoards();

    for (size_t size = shipCounts.size(); size > 0; --size) {
        uint8_t count = shipCounts[size - 1];
        if (count == 0) continue;
        
        int attempts = 0;
        const int MAX_ATTEMPTS = 1000;

        while (count > 0 && attempts < MAX_ATTEMPTS) {
            uint64_t x = disW(gen);
            uint64_t y = disH(gen);
            bool horizontal = disDir(gen);
            
            if (canPlaceShip(x, y, size, horizontal)) {
                if (placeShip(x, y, size, horizontal)) {
                    --count;
                    attempts = 0;
                }
            }
            ++attempts;
        }
    }

    for (size_t size = shipCounts.size(); size > 0; --size) {
        uint8_t count = shipCounts[size - 1];
        if (count == 0) continue;
        
        int attempts = 0;
        const int MAX_ATTEMPTS = 1000;

        while (count > 0 && attempts < MAX_ATTEMPTS) {
            uint64_t x = disW(gen);
            uint64_t y = disH(gen);
            bool horizontal = disDir(gen);
            
            if (canPlaceEnemyShip(x, y, size, horizontal)) {
                if (placeEnemyShip(x, y, size, horizontal)) {
                    --count;
                    attempts = 0;
                }
            }
            ++attempts;
        }
    }
}

bool Game::placeShip(uint64_t x, uint64_t y, int size, bool horizontal) {
    if (!canPlaceShip(x, y, size, horizontal)) {
        return false;
    }

    Ship newShip(x, y, size, horizontal);
    myShips.push_back(newShip);

    for (int i = 0; i < size; ++i) {
        if (horizontal) {
            myBoard[y][x + i] = CellState::SHIP;
        } else {
            myBoard[y + i][x] = CellState::SHIP;
        }
    }
    return true;
}

bool Game::canPlaceShip(uint64_t x, uint64_t y, int size, bool horizontal) const {
    if (horizontal) {
        if (x + size > width) return false;
    } else {
        if (y + size > height) return false;
    }

    for (int i = -1; i <= size; i++) {
        for (int j = -1; j <= 1; j++) {
            uint64_t checkX = horizontal ? x + i : x + j;
            uint64_t checkY = horizontal ? y + j : y + i;
            
            if (checkX >= width || checkY >= height) continue;
            if (checkX == (uint64_t)-1 || checkY == (uint64_t)-1) continue;

            if (myBoard[checkY][checkX] == CellState::SHIP) {
                return false;
            }
        }
    }

    return true;
}

bool Game::tryHitShip(uint64_t x, uint64_t y, Ship& ship) {
    return ship.tryHit(x, y);
}

ShootResult Game::processShot(uint64_t x, uint64_t y) {
    if (!isValidPosition(x, y)) {
        return ShootResult::INVALID;
    }

    if (enemyBoard[y][x] == CellState::HIT || enemyBoard[y][x] == CellState::MISS) {
        return ShootResult::INVALID;
    }

    if (enemyBoard[y][x] == CellState::SHIP) {
        enemyBoard[y][x] = CellState::HIT;
        
        for (Ship& ship : enemyShips) {
            if (ship.tryHit(x, y)) {
                return ship.isDestroyed() ? ShootResult::KILL : ShootResult::HIT;
            }
        }
    }
    
    if (enemyBoard[y][x] == CellState::EMPTY) {
        enemyBoard[y][x] = CellState::MISS;
    }
    
    return ShootResult::MISS;
}

ShootResult Game::processEnemyShot(uint64_t x, uint64_t y) {
    if (!isValidPosition(x, y)) {
        return ShootResult::INVALID;
    }

    if (myBoard[y][x] == CellState::HIT || myBoard[y][x] == CellState::MISS) {
        return ShootResult::INVALID;
    }

    if (myBoard[y][x] == CellState::SHIP) {
        myBoard[y][x] = CellState::HIT;
        
        for (Ship& ship : myShips) {
            if (ship.tryHit(x, y)) {
                return ship.isDestroyed() ? ShootResult::KILL : ShootResult::HIT;
            }
        }
    }
    
    if (myBoard[y][x] == CellState::EMPTY) {
        myBoard[y][x] = CellState::MISS;
    }
    
    return ShootResult::MISS;
}

std::pair<uint64_t, uint64_t> Game::getNextOrderedShot() {
    static uint64_t nextX = 0;
    static uint64_t nextY = 0;

    while (nextY < height) {
        while (nextX < width) {
            if (enemyBoard[nextY][nextX] == CellState::EMPTY) {
                uint64_t x = nextX++;
                return {x, nextY};
            }
            nextX++;
        }
        nextX = 0;
        nextY++;
    }
    return {0, 0};
}

std::pair<uint64_t, uint64_t> Game::getNextCustomShot() {
    static std::vector<std::pair<uint64_t, uint64_t>> lastHits;
    static std::vector<std::vector<bool>> visited(height, std::vector<bool>(width, false));

    if (!lastHits.empty()) {
        auto [lastX, lastY] = lastHits.back();
        
        const std::vector<std::pair<int, int>> directions = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
        for (const auto& [dx, dy] : directions) {
            uint64_t newX = lastX + dx;
            uint64_t newY = lastY + dy;
            
            if (isValidPosition(newX, newY) && !visited[newY][newX]) {
                visited[newY][newX] = true;
                return {newX, newY};
            }
        }
        
        lastHits.pop_back();
        return getNextCustomShot();
    }
    
    std::random_device rd;
    std::mt19937 gen(rd());
    
    while (true) {
        uint64_t x = std::uniform_int_distribution<uint64_t>(0, width - 1)(gen);
        uint64_t y = std::uniform_int_distribution<uint64_t>(0, height - 1)(gen);
        
        if ((x + y) % 2 == 0 && !visited[y][x]) {
            visited[y][x] = true;
            if (enemyBoard[y][x] == CellState::HIT) {
                lastHits.push_back({x, y});
            }
            
            return {x, y};
        }
    }
}

std::pair<uint64_t, uint64_t> Game::getNextShot() {
    return (currentStrategy == Strategy::ORDERED) ? 
           getNextOrderedShot() : getNextCustomShot();
}

bool Game::startGame() {
    if (!isValidGameSetup()) return false;
    
    generateRandomShipPlacement();
    gameStarted = true;
    isMyTurn = (mode == GameMode::SLAVE);

    std::cout << "\nGame started! Available commands:\n";
    std::cout << "- shot x y     : Make a shot at coordinates (x,y)\n";
    std::cout << "- save file    : Save the game to a file\n";
    std::cout << "- load file    : Load the game from a file\n";
    std::cout << "- display      : Show the game boards\n";
    std::cout << "- reveal       : Show enemy ships (debug)\n";
    std::cout << "- stop         : Stop the game\n\n";
    std::cout << "- exit        : Exit the program\n\n";

    return true;
}

bool Game::setWidth(uint64_t w) {
    if (gameStarted || w == 0) return false;
    width = w;
    initializeBoards();
    return true;
}

bool Game::setHeight(uint64_t h) {
    if (gameStarted || h == 0) return false;
    height = h;
    initializeBoards();
    return true;
}

bool Game::setShipCount(int shipSize, uint64_t count) {
    if (shipSize < 1 || shipSize > 4 || gameStarted) return false;
    shipCounts[shipSize - 1] = count;

    for (size_t i = 0; i < shipCounts.size(); ++i) {
        if (i != shipSize - 1) {
            shipCounts[i] = 0;
        }
    }
    
    return true;
}

uint64_t Game::getWidth() const {
    return width;
}

uint64_t Game::getHeight() const {
    return height;
}

uint64_t Game::getShipCount(int shipSize) const {
    if (shipSize < 1 || shipSize > 4) return 0;
    return shipCounts[shipSize - 1];
}

void Game::initializeBoards() {
    if (width > 0 && height > 0) {
        myBoard = std::vector<std::vector<CellState>>(height, std::vector<CellState>(width, CellState::EMPTY));
        enemyBoard = std::vector<std::vector<CellState>>(height, std::vector<CellState>(width, CellState::EMPTY));
    }
}

void Game::displayBoards() const {
    std::cout << "\nGame Mode: " << (mode == GameMode::MASTER ? "Master" : "Slave") << std::endl;
    if (gameStarted) {
        std::cout << "Current Turn: " << (isMyTurn ? "Your Turn" : "Enemy's Turn") << std::endl;
    }
    std::cout << std::endl;

    auto printColumnNumbers = [this]() {
        std::cout << "     ";
        for (uint64_t x = 0; x < width; ++x) {
            std::cout << x << "   ";
        }
        std::cout << std::endl;
    };

    std::cout << "\nMy Board:" << std::endl;
    printColumnNumbers();

    for (uint64_t y = 0; y < height; ++y) {
        std::cout << y << "   ";
        for (uint64_t x = 0; x < width; ++x) {
            char symbol = ' ';
            switch (myBoard[y][x]) {
                case CellState::EMPTY: symbol = '.'; break;
                case CellState::SHIP: {
                    for (const auto& ship : myShips) {
                        if (ship.containsPosition(x, y)) {
                            symbol = '0' + ship.getSize();
                            break;
                        }
                    }
                    break;
                }
                case CellState::HIT: symbol = 'X'; break;
                case CellState::MISS: symbol = 'O'; break;
            }
            std::cout << symbol << "   ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;

    std::cout << "Enemy Board:" << std::endl;
    printColumnNumbers();

    for (uint64_t y = 0; y < height; ++y) {
        std::cout << y << "   ";
        for (uint64_t x = 0; x < width; ++x) {
            char symbol = ' ';
            switch (enemyBoard[y][x]) {
                case CellState::EMPTY: symbol = '.'; break;
                case CellState::SHIP: symbol = '.'; break;
                case CellState::HIT: symbol = 'X'; break;
                case CellState::MISS: symbol = 'O'; break;
            }
            std::cout << symbol << "   ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void Game::displayEnemyShips() const {
    std::cout << "\nEnemy ships positions:\n";
    
    std::vector<std::vector<char>> tempBoard(height, std::vector<char>(width, '.'));

    for (const auto& ship : enemyShips) {
        int x = ship.getX();
        int y = ship.getY();
        bool isHorizontal = ship.isHorizontal();
        int size = ship.getSize();
        
        for (int i = 0; i < size; ++i) {
            if (isHorizontal) {
                tempBoard[y][x + i] = 'S';
            } else {
                tempBoard[y + i][x] = 'S';
            }
        }
    }
    
    std::cout << "  ";
    for (size_t i = 0; i < width; ++i) {
        std::cout << i << " ";
    }
    std::cout << "\n";
    
    for (size_t i = 0; i < height; ++i) {
        std::cout << i << " ";
        for (size_t j = 0; j < width; ++j) {
            std::cout << tempBoard[i][j] << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

bool Game::stopGame() {
    gameStarted = false;
    return true;
}

bool Game::isValidGameSetup() const {
    if (width == 0 || height == 0) return false;

    int totalShips = 0;
    int maxShipSize = 0;
    for (size_t i = 0; i < shipCounts.size(); ++i) {
        if (shipCounts[i] > 0) {
            totalShips += shipCounts[i];
            maxShipSize = std::max(maxShipSize, static_cast<int>(i + 1));
        }
    }
    if (totalShips == 0) return false;

    if (maxShipSize > width && maxShipSize > height) return false;

    int totalCells = width * height;
    int requiredCells = 0;
    for (size_t i = 0; i < shipCounts.size(); ++i) {
        requiredCells += shipCounts[i] * (i + 1);
    }
    if (requiredCells > totalCells) return false;

    return true;
}

bool Game::isValidPosition(uint64_t x, uint64_t y) const {
    return x < width && y < height;
}

bool Game::saveToFile(const std::string& path) const {
    std::ofstream file(path);
    if (!file) return false;

    file << width << " " << height << "\n";
    file << currentStrategy << "\n";
    file << gameStarted << "\n";
    file << isMyTurn << "\n";

    file << myShips.size() << "\n";
    for (const auto& ship : myShips) {
        file << ship.getSize() << " " << ship.getX() << " " << ship.getY() << " "
             << (ship.isHorizontal() ? "1" : "0") << "\n";
    }

    for (uint64_t i = 0; i < height; ++i) {
        for (uint64_t j = 0; j < width; ++j) {
            file << static_cast<int>(myBoard[i][j]) << " ";
        }
        file << "\n";
    }

    for (uint64_t i = 0; i < height; ++i) {
        for (uint64_t j = 0; j < width; ++j) {
            file << static_cast<int>(enemyBoard[i][j]) << " ";
        }
        file << "\n";
    }

    return true;
}

bool Game::loadFromFile(const std::string& path) {
    if (gameStarted) return false;

    std::ifstream file(path);
    if (!file) return false;

    uint64_t w, h;
    file >> w >> h;
    
    if (w == 0 || h == 0) return false;

    myShips.clear();
    setWidth(w);
    setHeight(h);

    int size;
    uint64_t x, y;
    std::string horizontal;
    
    while (file >> size >> x >> y >> horizontal) {
        if (size < 1 || size > 4) continue;
        bool isHorizontal = (horizontal == "1");
        placeShip(x, y, size, isHorizontal);
    }
    
    return true;
}

bool Game::isFinished() const {
    if (!gameStarted) return false;

    bool allMyShipsDestroyed = true;
    for (const Ship& ship : myShips) {
        if (!ship.isDestroyed()) {
            allMyShipsDestroyed = false;
            break;
        }
    }

    int enemyShipsLeft = 0;
    for (int size = 1; size <= 4; ++size) {
        enemyShipsLeft += shipCounts[size - 1];
    }

    int enemyShipsDestroyed = 0;
    for (uint64_t y = 0; y < height; ++y) {
        for (uint64_t x = 0; x < width; ++x) {
            if (enemyBoard[y][x] == CellState::HIT) {
                enemyShipsDestroyed++;
            }
        }
    }

    return allMyShipsDestroyed || (enemyShipsLeft > 0 && enemyShipsDestroyed >= enemyShipsLeft);
}

bool Game::isWinner() const {
    if (!isFinished()) return false;

    int enemyShipsLeft = 0;
    for (int size = 1; size <= 4; ++size) {
        enemyShipsLeft += shipCounts[size - 1];
    }

    int enemyShipsDestroyed = 0;
    for (uint64_t y = 0; y < height; ++y) {
        for (uint64_t x = 0; x < width; ++x) {
            if (enemyBoard[y][x] == CellState::HIT) {
                enemyShipsDestroyed++;
            }
        }
    }

    return enemyShipsDestroyed >= enemyShipsLeft;
}

bool Game::isLoser() const {
    for (const auto& ship : myShips) {
        if (!ship.isDestroyed()) {
            return false;
        }
    }
    return true;
}

bool Game::canPlaceEnemyShip(uint64_t x, uint64_t y, int size, bool horizontal) {
    if (horizontal) {
        if (x + size > width) return false;
    } else {
        if (y + size > height) return false;
    }

    for (int i = -1; i <= size; i++) {
        for (int j = -1; j <= 1; j++) {
            uint64_t checkX = horizontal ? x + i : x + j;
            uint64_t checkY = horizontal ? y + j : y + i;

            if (checkX >= width || checkY >= height) continue;
            if (checkX == (uint64_t)-1 || checkY == (uint64_t)-1) continue;

            if (enemyBoard[checkY][checkX] == CellState::SHIP) {
                return false;
            }
        }
    }
    return true;
}

bool Game::placeEnemyShip(uint64_t x, uint64_t y, int size, bool horizontal) {
    if (!canPlaceEnemyShip(x, y, size, horizontal)) {
        return false;
    }

    Ship newShip(x, y, size, horizontal);
    enemyShips.push_back(newShip);

    for (int i = 0; i < size; ++i) {
        if (horizontal) {
            enemyBoard[y][x + i] = CellState::SHIP;
        } else {
            enemyBoard[y + i][x] = CellState::SHIP;
        }
    }
    return true;
}
