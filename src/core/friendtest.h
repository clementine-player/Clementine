/* This file is part of Clementine.
   Copyright 2026, John Maguire <john.maguire@gmail.com>

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

#ifndef CORE_FRIENDTEST_H_
#define CORE_FRIENDTEST_H_

// Lets a googletest test use a class's private members, as gtest_prod.h's
// FRIEND_TEST does, without Clementine itself needing googletest to build.
#define CLEMENTINE_FRIEND_TEST(test_suite_name, test_name) \
  friend class test_suite_name##_##test_name##_Test

#endif  // CORE_FRIENDTEST_H_
