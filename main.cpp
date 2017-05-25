#include <cassert>
#include <chrono>
#include <iostream>
#include <map>
#include <mpi.h>
#include <sstream>
#include <utility>
#include <vector>

#include "Board.hpp"
#include "Move.hpp"
#include "Search.hpp"
#include "timer.hpp"

#define BOARD_WIDTH 7
#define BOARD_HEIGTH 6

using namespace std;

int getValidUserMove(Board& b)
{
  int ret = -1;
  auto validMoves = b.getValidMoves();

  while (!validMoves.count(ret)) {
    cout << "Your move: " << endl;
    cin >> ret;
  }

  assert(ret >= 0 && ret < BOARD_WIDTH);

  return ret;
}

int main(int argc, char** argv)
{
  MPI_Init(&argc, &argv);

  Board b(BOARD_WIDTH, BOARD_HEIGTH, Player::COM);

  int maxDepth = 4;

  if (argc > 1) {
    stringstream ss;
    ss << argv[1];
    ss >> maxDepth;
  }

  int rank;
  int poolSize;

  MPI_Comm_size(MPI_COMM_WORLD, &poolSize);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  map<int, float> moveQuality;
  if (rank == 0) {

    while (!b.won() && !b.lost() && b.getValidMoves().size() > 0) {
      auto validMoves = b.getValidMoves();
      // Search s(maxDepth, rank, poolSize);
      Timer t;
      t.start();
      int bestIndex = Search::getBestMoveMPI(poolSize, maxDepth, b);
      // cout << "MPI best: " << bestIndex << endl;
      cout << t.diffMilliseconds() << endl;

      int maxIndex = bestIndex;
      // cout << "maxINdex: " << maxIndex << endl;

      Move::broadcastMove(b.turn, maxIndex);
      b.move(maxIndex, b.turn);

      if (b.getValidMoves().size() == 0) {
        break;
      }

      cout << b << endl;

      if (b.won()) {
        break;
      }

      int gameOver = 0;
      MPI_Bcast(&gameOver, 1, MPI_INT, 0, MPI_COMM_WORLD);

      maxIndex = getValidUserMove(b);

      Move::broadcastMove(b.turn, maxIndex);
      b.move(maxIndex, b.turn);

      if (!b.lost()) {
        MPI_Bcast(&gameOver, 1, MPI_INT, 0, MPI_COMM_WORLD);
      } else {
        gameOver = 1;
        MPI_Bcast(&gameOver, 1, MPI_INT, 0, MPI_COMM_WORLD);
      }

      cout << b << endl;
    }

    int gameOver = 1;
    MPI_Bcast(&gameOver, 1, MPI_INT, 0, MPI_COMM_WORLD);

    cout << b << endl;

    if (b.won()) {
      cout << "you lose" << endl;
    } else if (b.lost()) {
      cout << "you won" << endl;
    } else {
      if (b.getValidMoves().size() == 0) {

        cout << "it's a tie" << endl;
      } else {

        cout << "this is embarassing" << endl;
      }
    }

  } else {
    while (true) {
      //TODO process tasks from master
      int depth;
      MPI_Recv(&depth, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      int tasksCount;
      MPI_Recv(&tasksCount, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      vector<SearchTask> tasks;
      while (tasksCount--) {
        int movesCount = 0;
        MPI_Recv(&movesCount, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        vector<Move> moves(movesCount);
        MPI_Recv(&moves[0], 2 * movesCount, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        tasks.push_back(SearchTask(depth, moves));
      }

      vector<pair<int, int> > result;
      for (auto task : tasks) {
        long stateCount = 0;
        int res = Search::processTask(task, b, stateCount);
        result.push_back(pair<int, int>(res, stateCount));
      }

      MPI_Send(&result[0].first, 2 * result.size(), MPI_INT, 0, 0, MPI_COMM_WORLD);
      // for (auto r : result) {
      //   MPI_Send(&r.first, 2, MPI_INT, 0, 0, MPI_COMM_WORLD);
      // }

      auto move = Move::getMove();

      // cout << "slave wait COM" << endl;
      // cout << "move: " << move << endl;
      b.move(move.position, move.player);

      int gameOver = 0;
      MPI_Bcast(&gameOver, 1, MPI_INT, rank, MPI_COMM_WORLD);

      if (gameOver) {
        break;
      }

      // cout << "slave wait HUMAN" << endl;
      move = Move::getMove();
      // cout << "move: " << move << endl;
      b.move(move.position, move.player);

      MPI_Bcast(&gameOver, 1, MPI_INT, rank, MPI_COMM_WORLD);

      if (gameOver) {
        break;
      }
    }
  }

  MPI_Finalize();

  return 0;
}
