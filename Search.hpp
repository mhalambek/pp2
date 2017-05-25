#pragma once

#include "Board.hpp"
#include "Move.hpp"
#include "timer.hpp"
#include <iostream>
#include <map>

using namespace std;

struct SearchTask {
  int depth;
  vector<Move> moves;

  SearchTask() {}

  SearchTask(int depth, const vector<Move> moves)
      : depth{ depth }
      , moves{ moves }
  {
  }
};

long winLoseDelta(Board b, int depth, long& stateCount)
{
  stateCount++;
  if (b.won() && b.turn == Player::HUMAN) {

    return 1;
  }

  if (b.lost() && b.turn == Player::COM) {

    return -1;
  }

  if (depth == 0) {

    return 0;
  }

  long ret = 0;
  auto validMoves = b.getValidMoves();

  map<int, long> moveQuality;

  for (int m : validMoves) {
    moveQuality[m] = 0;
    Board ba = b;
    ba.move(m, ba.turn);
    moveQuality[m] = winLoseDelta(ba, depth - 1, stateCount);
  }

  for (auto& a : moveQuality) {

    ret += a.second;
  }

  return ret;
}

struct Search {
  const int maxDepth, rank, poolSize;

  Search(int maxDepth, int rank, int poolSize)
      : maxDepth{ maxDepth }
      , rank{ rank }
      , poolSize{ poolSize }
  {
  }

  map<int, float> getMoveQuality(Board& b)
  {
    map<int, float> quality;
    auto valid = b.getValidMoves();

    for (int m : valid) {
      Board ba(b);
      ba.move(m, ba.turn);

      long stateCount = 0;
      long score = winLoseDelta(ba, maxDepth, stateCount);
      quality[m] = score / valid.size();
    }

    return quality;
  }

  int getBestMove(Board& b)
  {
    const auto moveQuality = getMoveQuality(b);
    int bestIndex = moveQuality.begin()->first;
    float bestValue = moveQuality.begin()->second;

    for (auto move : moveQuality) {

      // cout << "move: " << move.first << endl;
      // cout << "ratio: " << move.second << endl;

      if (move.second > bestValue) {
        bestValue = move.second;
        bestIndex = move.first;
      }
    }

    return bestIndex;
  }

  static int getBestMoveMPI(unsigned int poolSize, int depth, Board& b)
  {
    auto tasks = generateTasks(poolSize, depth, b);
    // cout << "task count: " << tasks.size() << endl;
    //send tasks, process some yourself

    map<int, vector<SearchTask> > order;

    for (unsigned int i = 0; i < tasks.size(); ++i) {
      order[i % poolSize].push_back(tasks[i]);
    }

    for (unsigned int i = 0; i < poolSize; ++i) {
      cout << "process " << i << ", tasks count: " << order[i].size() << endl;
    }

    //sendTasks, fetch results

    for (auto batch : order) {
      if (batch.first == 0) {
        continue;
      }
      int maxDepth = order.begin()->second.begin()->depth;
      //send depth
      MPI_Send(&maxDepth, 1, MPI_INT, batch.first, 0, MPI_COMM_WORLD);
      //send task count
      int tasksCount = batch.second.size();
      MPI_Send(&tasksCount, 1, MPI_INT, batch.first, 0, MPI_COMM_WORLD);
      for (auto& task : batch.second) {
        int movesCount = task.moves.size();
        MPI_Send(&movesCount, 1, MPI_INT, batch.first, 0, MPI_COMM_WORLD);
        MPI_Send(&task.moves[0], 2 * task.moves.size(), MPI_INT, batch.first, 0, MPI_COMM_WORLD);
      }
    }

    map<int, pair<int, int> > score;
    vector<pair<int, int> > result;

    for (auto task : order[0]) {
      long stateCount = 0;
      int res = Search::processTask(task, b, stateCount);
      result.push_back(pair<int, int>(res, stateCount));
      if (score.count(task.moves[0].position)) {
        auto& a = score[task.moves[0].position];
        a.first += res;
        a.second += stateCount;
      } else {
        score[task.moves[0].position].first = res;
        score[task.moves[0].position].second = stateCount;
      }
    }

    Timer t, fetchTimer;
    t.start();
    fetchTimer.start();
    for (auto batch : order) {
      if (batch.first == 0) {
        continue;
      }
      vector<pair<int, int> > results(batch.second.size());

      MPI_Recv(&results[0].first, 2 * batch.second.size(), MPI_INT, batch.first, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      for (unsigned int i = 0; i < batch.second.size(); ++i) {
        auto& task = batch.second[i];
        auto& res = results[i];
        if (score.count(task.moves[0].position)) {
          auto& a = score[task.moves[0].position];
          a.first += res.first;
          a.second += res.second;
        } else {
          score[task.moves[0].position].first = res.first;
          score[task.moves[0].position].second = res.second;
        }
      }
    }
    cout << "fetchTimer: " << fetchTimer.diffMilliseconds() << endl;

    map<int, float> ratio;

    int ret = score.begin()->first;

    float maxValue = -2;
    for (auto s : score) {
      ratio[s.first] = (float)s.second.first / s.second.second;

      if (ratio[s.first] > maxValue) {
        maxValue = ratio[s.first];
        ret = s.first;
      }
    }
    cout << "reduce all results: " << t.diffMilliseconds() << endl;

    return ret;
  }

  static vector<SearchTask> generateTasks(unsigned int poolSize, int depth, Board& b)
  {
    auto valid = b.getValidMoves();
    vector<SearchTask> ret;

    ret.push_back(SearchTask(depth, vector<Move>()));

    while (ret.size() < poolSize * 10 && --depth > 0) {
      vector<SearchTask> expandee;
      for (auto& task : ret) {

        //get board into state described in task
        Board ba(b);
        for (auto& m : task.moves) {
          ba.move(m.position, m.player);
        }

        //generate next level tasks
        for (auto& m : ba.getValidMoves()) {
          SearchTask t(task);
          t.depth = depth;
          t.moves.push_back(Move(ba.turn, m));
          expandee.push_back(t);
        }
      }

      ret = expandee;
    }

    return ret;

    if (false && valid.size() >= poolSize) {
      for (int m : valid) {
        ret.push_back(SearchTask(depth, vector<Move>({ Move(b.turn, m) })));
      }
    } else {
      for (int m : valid) {
        Board ba(b);
        ba.move(m, b.turn);
        for (int n : ba.getValidMoves()) {
          vector<Move> moves({ Move(b.turn, m), Move(ba.turn, n) });
          ret.push_back(SearchTask(depth - 1, moves));
        }
      }
    }

    return ret;
  }

  static int processTask(SearchTask& s, Board b, long& stateCount)
  {
    for (auto m : s.moves) {
      b.move(m.position, m.player);
    }

    return winLoseDelta(b, s.depth, stateCount);
  }
};
