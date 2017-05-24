#pragma once

#include <cassert>
#include <ostream>
#include <set>
#include <vector>

using namespace std;

enum Player {
  FREE,
  COM,
  HUMAN
};

class Board {
  const int width, heigth;
  vector<vector<short> > board;
  bool exists4InRow(short player) const
  {

    //horizontal
    for (int i = 0; i < heigth; ++i) {
      for (int j = 0; j < width - 3; ++j) {
        if (board[i][j] == player && board[i][j + 1] == player && board[i][j + 2] == player && board[i][j + 3] == player) {

          return true;
        }
      }
    }

    //vertical
    for (int i = 0; i < heigth - 3; ++i) {
      for (int j = 0; j < width; ++j) {
        if (board[i][j] == player && board[i + 1][j] == player && board[i + 2][j] == player && board[i + 3][j] == player) {

          return true;
        }
      }
    }
    // /

    for (int i = heigth - 1; i > 2; --i) {
      for (int j = 0; j < width - 2; ++j) {
        if (board[i][j] == player && board[i - 1][j + 1] == player && board[i - 2][j + 2] == player && board[i - 3][j + 3] == player) {

          return true;
        }
      }
    }

    for (int i = 0; i < heigth - 3; ++i) {
      for (int j = 0; j < width - 3; ++j) {
        if (board[i][j] == player && board[i + 1][j + 1] == player && board[i + 2][j + 2] == player && board[i + 3][j + 3] == player) {

          return true;
        }
      }
    }

    return false;
  }

  public:
  short turn;
  Board(const Board& b)
      : width{ b.width }
      , heigth{ b.heigth }
      , board{ b.board }
      , turn{ b.turn }
  {
  }

  Board(int width, int heigth, short turn)
      : width{ width }
      , heigth{ heigth }
      , turn{ turn }
  {
    board.resize(heigth);
    for (auto& row : board) {
      row = vector<short>(width, Player::FREE);
    }
  }

  set<int> getValidMoves()
  {
    set<int> ret;
    for (int j = 0; j < width; ++j) {
      for (int i = 0; i < heigth; ++i) {
        if (board[i][j] == Player::FREE) {
          ret.insert(j);
          break;
        }
      }
    }

    return ret;
  }

  void move(short m, short player)
  {
    assert(player == turn);

    for (int i = heigth - 1; i > -1; --i) {
      if (board[i][m] == Player::FREE) {
        board[i][m] = player;
        break;
      }
    }

    if (player == Player::COM) {
      turn = Player::HUMAN;
    } else if (player == Player::HUMAN) {
      turn = Player::COM;
    }
  }

  bool won() const
  {
    // cout << "won" << endl;

    return exists4InRow(Player::COM);
  }

  bool lost() const
  {
    // cout << "lost" << endl;

    return exists4InRow(Player::HUMAN);
  }

  friend ostream& operator<<(ostream& os, const Board& b)
  {
    for (auto& row : b.board) {
      for (auto e : row) {
        os << e << " ";
      }

      os << endl;
    }

    return os;
  }
};
