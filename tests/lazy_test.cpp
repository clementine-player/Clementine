/* This file is part of Clementine.

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "core/lazy.h"

#include "gtest/gtest.h"

namespace {

TEST(LazyTest, IsTrueOnceInitialised) {
  int made = 0;
  Lazy<int> lazy([&made]() {
    ++made;
    return new int(42);
  });

  // Asking whether it's initialised doesn't initialise it.
  EXPECT_FALSE(lazy);
  EXPECT_EQ(0, made);

  EXPECT_EQ(42, *lazy);
  EXPECT_TRUE(lazy);
  EXPECT_EQ(1, made);

  lazy.reset();
  EXPECT_FALSE(lazy);
}

}  // namespace
