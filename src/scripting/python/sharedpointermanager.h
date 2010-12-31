/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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

#ifndef SHAREDPOINTERMANAGER_H
#define SHAREDPOINTERMANAGER_H

#include <boost/shared_ptr.hpp>
#include <map>
#include <utility>

template<typename T>
struct SharedPointerManager {
  typedef T                                        cpp_type;
  typedef boost::shared_ptr<cpp_type>              smart_pointer_type;
  typedef std::pair<smart_pointer_type, int>       smart_pointer_count;
  typedef std::map<cpp_type*, smart_pointer_count> pointer_map_type;

  static pointer_map_type* _pointer_map;

  static void init() {
    if(!_pointer_map)
      _pointer_map = new pointer_map_type();
  }

  // Add a new reference or increment a previous reference
  static int AddRef(smart_pointer_type& ref) {
    init();

    typename pointer_map_type::iterator it = _pointer_map->find(ref.get());
    if (it != _pointer_map->end())
      return ++(*it).second.second;

    _pointer_map->insert(std::make_pair(ref.get(), std::make_pair(ref, 1)));
    return 1;
  }

  // Release a reference
  static int SubRef(cpp_type* ptr) {
    init();

    typename pointer_map_type::iterator it = _pointer_map->find(ptr);
    if (it == _pointer_map->end())
      return -1;

    int count = --(*it).second.second;
    if (0 == count)
      _pointer_map->erase(it);

    return count;
  }

  // Create a new smart pointer instance (from copy in map if available)
  static smart_pointer_type* CreatePointer(cpp_type* ptr) {
    init();

    typename pointer_map_type::iterator it = _pointer_map->find(ptr);
    if (it == _pointer_map->end())
      return new smart_pointer_type(ptr);
    else
      return new smart_pointer_type((*it).second.first);
  }
};

#endif // SHAREDPOINTERMANAGER_H
