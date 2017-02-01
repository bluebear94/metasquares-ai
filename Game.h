#pragma once

#include <stdint.h>
#include <string.h>

enum GameMode {
  SQUARE = 1,
  TRIANGLE,
  RHOMBUS,
  HEXAGON
};

inline int actual(GameMode gm, int from) {
  return (gm == SQUARE) ? from : (2 * from - 1);
}

class Board {
public:
  Board() : board(nullptr) {}
  Board(int width, int height, GameMode gm = SQUARE) :
      gm(gm), width(width), height(height),
      gridWidth(actual(gm, width)),
      gridHeight(actual(gm, height)),
      board(new uint8_t[gridWidth * gridHeight]) {
    memset(board, 0, gridWidth * gridHeight);
  }
  Board(const Board& other) :
      gm(other.gm), width(other.width), height(other.height),
      gridWidth(other.gridWidth),
      gridHeight(other.gridHeight),
      board(new uint8_t[gridWidth * gridHeight]) {
    memcpy(board, other.board, gridWidth * gridHeight);
  }
  Board(const Board&& other) :
    gm(other.gm),width(other.width), height(other.height),
    gridWidth(other.gridWidth),
    gridHeight(other.gridHeight),
    board(other.board) {}
  Board& operator=(const Board& other);
  ~Board() { delete[] board; }
  bool isValid(int row, int col) const;
  bool isVacant(int row, int col) const;
  int get(int row, int col) const;
  void put(int row, int col, int player);
  int newShapesFrom(int player, int row, int col) const;
  int getGridWidth() const { return gridWidth; }
  int getGridHeight() const { return gridHeight; }
private:
  GameMode gm;
  int width;
  int height;
  int gridWidth;
  int gridHeight;
  uint8_t* board;
};

class Game {
public:
  Game() : numberOfPlayers(0), scores(nullptr) {}
  Game(int numberOfPlayers, int width, int height, GameMode gm = SQUARE) :
      numberOfPlayers(numberOfPlayers), scores(new int[numberOfPlayers]),
      board(width, height, gm) {
    memset(scores, 0, sizeof(int) * numberOfPlayers);
  }
  Game(const Game& other) :
      numberOfPlayers(other.numberOfPlayers),
      scores(new int[numberOfPlayers]),
      board(other.board),
      currentPlayer(other.currentPlayer) {
    memcpy(scores, other.scores, sizeof(int) * numberOfPlayers);
  }
  Game(const Game&& other) :
      numberOfPlayers(other.numberOfPlayers),
      scores(other.scores),
      board(other.board),
      currentPlayer(other.currentPlayer) {}
  ~Game() { delete[] scores; }
  Game& operator=(const Game& other);
  int getPlayerCount() const { return numberOfPlayers; }
  int getScore(int player) const { return scores[player - 1]; }
  int netScore(int player) const;
  Board& getBoard() { return board; }
  int getCurrentPlayer() const { return currentPlayer; }
  void move(int row, int col);
  void move(int row, int col, int score);
private:
  int numberOfPlayers;
  int* scores;
  Board board;
  int currentPlayer = 1;
};

struct Eval {
  Game* game = nullptr;
  int row, col;
};
