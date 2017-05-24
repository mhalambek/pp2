#pragma once

#include <cassert>
#include <mpi.h>
#include <ostream>
#include <vector>

using namespace std;

struct Move {

  int player;
  int position;

  Move() {}

  Move(int player, int position)
      : player{ player }
      , position{ position }
  {
  }

  static void broadcastMoves(vector<Move> moves)
  {
    int moveCount = moves.size();

    int poolSize;
    MPI_Comm_size(MPI_COMM_WORLD, &poolSize);

    for (int i = 1; i < poolSize; ++i) {
      MPI_Send(&moveCount, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
      MPI_Send(&moves[0].player, 2 * moveCount, MPI_INT, i, 0, MPI_COMM_WORLD);
    }
  }

  static void broadcastMove(int player, int position)
  {
    Move move(player, position);
    broadcastMoves(vector<Move>({ move }));
  }

  static void sendMoves(vector<Move> moves, int dest)
  {
    int movesCount = moves.size();
    MPI_Send(&movesCount, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
    MPI_Send(&moves[0].player, 2 * movesCount, MPI_INT, dest, 0, MPI_COMM_WORLD);
  }

  static vector<Move> getMoves()
  {
    vector<Move> ret;

    return ret;
  }

  static Move getMove()
  {
    Move move;
    int moveCount;

    MPI_Recv(&moveCount, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    assert(moveCount == 1);

    MPI_Recv(&move.player, 2, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    return move;
  }

  friend ostream& operator<<(ostream& os, const Move& m)
  {
    os << "player: " << m.player << ", move: " << m.position << endl;

    return os;
  }
};
