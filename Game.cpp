#include "Game.h"

#include <limits.h>
#include <algorithm>
#include <iostream>

Board& Board::operator=(const Board& other) {
  if (gridWidth * gridHeight < other.gridWidth * other.gridHeight) {
    delete[] board;
    board = new uint8_t[other.gridWidth * other.gridHeight];
  }
  width = other.width;
  height = other.height;
  gridWidth = other.gridWidth;
  gridHeight = other.gridHeight;
  memcpy(board, other.board, gridWidth * gridHeight);
  return *this;
}

bool Board::isValid(int row, int col) const {
  if (row < 0 || row >= gridHeight || col < 0 || col >= gridWidth)
    return false;
  if (gm == SQUARE) return true;
  if (row <= height) return col < gridWidth - (height - row - 1);
  return col >= row - height - 1;
}

bool Board::isVacant(int row, int col) const {
  return isValid(row, col) && board[gridWidth * row + col] == 0;
}

int Board::get(int row, int col) const {
  if (row < 0 || row >= gridHeight || col < 0 || col >= gridWidth)
    return 0;
  return board[gridWidth * row + col];
}

void Board::put(int row, int col, int player) {
  board[gridWidth * row + col] = player;
}

int Board::newShapesFrom(int player, int row, int col) const {
  int score = 0;
  switch (gm) {
    case SQUARE: {
      int rlimit = std::max(row, gridHeight - row);
      int climit = std::max(col, gridWidth - col);
      int startI = rlimit > climit ? 1 : 0;
      int startJ = rlimit <= climit ? 1 : 0;
      // std::cout << rlimit << ' ' << climit << '\n';
      for (int i = startI; i <= rlimit; ++i) {
        // Avoid double-counting (i, 0) and (0, i)
        for (int j = startJ; j <= climit; ++j) {
          // std::cout << i << ' ' << j << '\n';
          // complement points to (i, j) are:
          // (-j, i) (-i, -j) (j, -i)
          bool s0 = get(row + i, col + j) == player;
          bool s1 = get(row - j, col + i) == player;
          bool s2 = get(row - i, col - j) == player;
          bool s3 = get(row + j, col - i) == player;
          int amt = i * i + j * j;
          if (s0 && s1 && get(row + i - j, col + j + i) == player) score += amt;
          if (s1 && s2 && get(row - j - i, col + i - j) == player) score += amt;
          if (s2 && s3 && get(row - i + j, col - j - i) == player) score += amt;
          if (s3 && s0 && get(row + j + i, col - i + j) == player) score += amt;
        }
      }
      break;
    }
    default:
      throw "NYI";
  }
  return score;
}

Game& Game::operator=(const Game& other) {
  if (numberOfPlayers < other.numberOfPlayers) {
    delete[] scores;
    scores = new int[other.numberOfPlayers];
  }
  numberOfPlayers = other.numberOfPlayers;
  memcpy(scores, other.scores, sizeof(int) * numberOfPlayers);
  board = other.board;
  currentPlayer = other.currentPlayer;
  return *this;
}

Eval evaluate(Eval& e, int origPlayer, int depth = 0, int alpha = INT_MIN, int beta = INT_MAX, int moveLimit = 20000000) {
  if (moveLimit <= 1)
    return e;
  Game& g = *(e.game);
  int player = g.getCurrentPlayer();
  Board& b = g.getBoard();
  int totalMoves = 0;
  for (int i = 0; i < b.getGridWidth(); ++i) {
    for (int j = 0; j < b.getGridHeight(); ++j) {
      if (b.isVacant(i, j)) ++totalMoves;
    }
  }
  if (totalMoves == 0) return e;
  int maxScore = INT_MIN;
  Eval bestForYou;
  for (int i = 0; i < b.getGridWidth(); ++i) {
    for (int j = 0; j < b.getGridHeight(); ++j) {
      if (!b.isVacant(i, j)) continue;
      Eval ep = e;
      ep.game = new Game(*(ep.game));
      ep.row = i;
      ep.col = j;
      ep.game->move(i, j);
      Eval next = evaluate(ep, origPlayer, depth + 1, alpha, beta, moveLimit / totalMoves);
      int newScore = next.game->netScore(player);
      /* if (depth == 0)
        std::cout << "By picking " << i << ' ' << j
          << ", we can get a score of " << newScore << ".\n"; */
      if (newScore > maxScore) {
        // if (depth == 0) std::cout << "(new best) \n";
        maxScore = newScore;
        delete bestForYou.game;
        bestForYou = ep;
      } else {
        delete ep.game;
      }
      if (next.game != ep.game)
        delete next.game;
      bool isMe = player == origPlayer;
      if (isMe && newScore > alpha) alpha = newScore;
      if (!isMe && -newScore < beta) beta = -newScore;
      if (beta <= alpha) break;
    }
  }
  return bestForYou;
}

int Game::netScore(int player) const {
  int res = scores[player - 1] * numberOfPlayers;
  for (int i = 0; i < numberOfPlayers; ++i) res -= scores[i];
  return res;
}

void Game::move(int row, int col, int score) {
  board.put(row, col, currentPlayer);
  scores[currentPlayer - 1] += score;
  currentPlayer = (currentPlayer % numberOfPlayers) + 1;
}

void Game::move(int row, int col) {
  move(row, col, board.newShapesFrom(currentPlayer, row, col));
}

int main() {
  Game g(2, 6, 6);
  int n;
  std::cout
    << "First (0) or second (1)? Or just let the AI play both sides (2). ";
  std::cin >> n;
  for (int i = 0; i < 36; ++i) {
    if (i % 2 == n) {
      std::cout << "Move? ";
      int row, col;
      while (true) {
        std::cin >> row >> col;
        if (g.getBoard().isVacant(row, col)) break;
        std::cout << "That space is occupied.\n";
      }
      g.move(row, col);
    } else {
      Eval e = { &g, -1, -1 };
      Eval bestMove = evaluate(e, g.getCurrentPlayer());
      g.move(bestMove.row, bestMove.col);
      std::cout << "AI chose " << bestMove.row << ' ' << bestMove.col << '\n';
      std::cout << "Board:\n";
      for (int i = 0; i < 6; ++i) {
        for (int j = 0; j < 6; ++j) {
          std::cout << g.getBoard().get(i , j) << ' ';
        }
        std::cout << '\n';
      }
      for (int i = 1; i <= 2; ++i) {
        std::cout << "Score of player " << i << " is " << g.getScore(i) << '\n';
      }
      delete bestMove.game;
    }
  }
  return 0;
}