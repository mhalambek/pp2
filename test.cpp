#include "Board.hpp"
#include <cassert>
#include <iostream>

using namespace std;

int main()
{
  Board b(7, 6, Player::COM);

  assert(!b.won());
  assert(!b.lost());
  auto valid = b.getValidMoves();
  assert(valid.size() == 7);

  for (int i = 0; i < 7; ++i) {
    assert(valid.count(i));
  }

  //vertical
  b.move(0, b.turn);
  b.move(0, b.turn);
  b.move(1, b.turn);
  b.move(0, b.turn);
  b.move(1, b.turn);
  b.move(0, b.turn);
  // cout << b << endl;
  assert(!b.won());
  b.move(1, b.turn);
  b.move(0, b.turn);
  assert(b.lost());
  // cout << b << endl;

  //horizontal
  Board ba(7, 6, Player::COM);
  ba.move(0, ba.turn);
  ba.move(0, ba.turn);
  ba.move(1, ba.turn);
  ba.move(1, ba.turn);
  ba.move(2, ba.turn);
  ba.move(2, ba.turn);
  ba.move(3, ba.turn);
  assert(ba.won());
  // cout << ba << endl;
  {
    Board bh(7, 6, Player::COM);
  }

  Board bslash(7, 6, Player::COM);
  bslash.move(0, bslash.turn);
  bslash.move(1, bslash.turn);
  bslash.move(1, bslash.turn);
  bslash.move(2, bslash.turn);
  bslash.move(3, bslash.turn);
  bslash.move(2, bslash.turn);
  bslash.move(2, bslash.turn);
  bslash.move(3, bslash.turn);
  bslash.move(3, bslash.turn);
  bslash.move(4, bslash.turn);
  bslash.move(3, bslash.turn);
  assert(bslash.won());

  Board bs(7, 6, Player::COM);

  bs.move(6, bs.turn);
  bs.move(5, bs.turn);
  bs.move(5, bs.turn);
  bs.move(4, bs.turn);
  bs.move(4, bs.turn);
  bs.move(3, bs.turn);
  bs.move(4, bs.turn);
  bs.move(3, bs.turn);
  bs.move(3, bs.turn);
  bs.move(2, bs.turn);
  bs.move(3, bs.turn);

  cout << bs << endl;
  assert(bs.won());

  return 0;
}
