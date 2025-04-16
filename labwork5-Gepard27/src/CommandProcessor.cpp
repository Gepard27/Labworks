#include "../include/CommandProcessor.hpp"
#include <iostream>
#include <sstream>

CommandProcessor::CommandProcessor(Game& game) : game(game) {}

void CommandProcessor::run() {
    std::cout << "\nAvailable commands for game creation:\n";
    std::cout << "create master/slave - create game in master/slave mode\n";
    std::cout << "exit - exit the game\n\n";

    std::string line;
    while (std::getline(std::cin, line)) {
        std::string response = processCommand(line);
        std::cout << response << std::endl;
        
        if (line == "exit") {
            break;
        }
    }
}

std::pair<std::string, std::string> CommandProcessor::parseCommand(const std::string& command) {
    std::istringstream iss(command);
    std::string cmd, args;
    iss >> cmd;
    std::getline(iss >> std::ws, args);
    return {cmd, args};
}

std::string CommandProcessor::processCommand(const std::string& command) {
    std::istringstream iss(command);
    std::string cmd;
    iss >> cmd;
    std::string args;
    std::getline(iss >> std::ws, args);

    if (cmd == "create") {
        if (game.createGame(args)) {
            std::cout << "\nGame configuration started! Available commands:\n";
            std::cout << "- set size <width> <height>  : Set board size (example: set size 10 10)\n";
            std::cout << "- set ships <size> <count>   : Set number of ships (example: set ships 4 1)\n";
            std::cout << "- set strategy type      : Set strategy (ordered/random/custom)\n";
            std::cout << "- start                  : Start the game\n\n";
            return "Game mode set to " + args;
        }
        return "Failed to set game mode";
    }
    else if (cmd == "start") {
        if (game.startGame()) {
            game.displayBoards();
            if (game.isCurrentTurn()) {
                return "Your turn! Make a shot (shot x y)";
            } else {
                return makeEnemyShot(), "Enemy's turn!";

            }
        }
        return "Failed to start game";
    }
    else if (cmd == "shot") {
        if (!game.isCurrentTurn()) {
            return "Not your turn!";
        }
        
        std::istringstream iss(args);
        uint64_t x, y;
        if (!(iss >> x >> y)) {
            return "Invalid shot format. Use: shot x y";
        }
        
        if (!game.isValidPosition(x, y)) {
            return "Invalid coordinates";
        }
        
        ShootResult result = game.processShot(x, y);
        game.displayBoards();
        
        switch (result) {
            case ShootResult::MISS:
                game.switchTurn();
                return "Miss! " + makeEnemyShot();
            case ShootResult::HIT:
                return "Hit! Your turn again.";
            case ShootResult::KILL:
                if (game.isFinished()) {
                    return "Ship destroyed! Game Over - You won!";
                }
                return "Ship destroyed! Your turn again.";
            case ShootResult::INVALID:
                return "Invalid shot";
            default:
                return "Unknown result";
        }
    }
    else if (cmd == "stop") {
        if (game.stopGame()) {
            return "Game stopped";
        }
        return "Failed to stop game";
    }
    else if (cmd == "display") {
        game.displayBoards();
        return "";
    }
    else if (cmd == "reveal") {
        game.displayEnemyShips();
        return "";
    }
    else if (cmd == "set") {
        std::istringstream iss(args);
        std::string param;
        iss >> param;

        if (param == "strategy") {
            std::string strategy;
            iss >> strategy;
            return game.setStrategy(strategy) ? "ok" : "failed";
        }
        else if (param == "size") {
            uint64_t width, height;
            iss >> width >> height;
            return (game.setWidth(width) && game.setHeight(height)) ? "ok" : "failed";
        }
        else if (param == "ships") {
            int size;
            uint64_t count;
            if (iss >> size >> count) {
                return game.setShipCount(size, count) ? "ok" : "failed";
            }
        }
        return "Invalid set command";
    }
    else if (cmd == "save") {
        return game.saveToFile(args) ? "Game saved" : "Failed to save game";
    }
    else if (cmd == "load") {
        return game.loadFromFile(args) ? "Game loaded" : "Failed to load game";
    }
    else if (cmd == "exit") {
        return "Goodbye!";
    }
    
    return "Unknown command";
}

std::string CommandProcessor::makeEnemyShot() {
    auto [x, y] = game.getNextShot();
    ShootResult result = game.processEnemyShot(x, y);
    game.displayBoards();
    
    std::string resultStr = "Enemy shot at (" + std::to_string(x) + "," + std::to_string(y) + "): ";
    switch (result) {
        case ShootResult::MISS:
            game.switchTurn();
            return resultStr + "Miss! Your turn!";
        case ShootResult::HIT:
            return resultStr + "Hit! Enemy's turn again." + makeEnemyShot();
        case ShootResult::KILL:
            if (game.isFinished()) {
                return resultStr + "Ship destroyed! Game Over - Enemy won!";
            }
            return resultStr + "Ship destroyed! Enemy's turn again." + makeEnemyShot();
        default:
            return resultStr + "Invalid shot";
    }
}
