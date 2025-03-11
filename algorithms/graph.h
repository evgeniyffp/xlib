#pragma once

#include <string>
#include <unordered_map>
#include <deque>
#include <queue>
#include <utility>

namespace xlib {
  template <
        typename Key = std::string,
        class Collection = std::deque<Key>,
        class HashMap = std::unordered_map<Key, Collection>
  >
  class graph {
  private:
    HashMap values;

    static bool isContainsElement(std::queue<Key> q, const Key& key) {
      while (!q.empty()) {
        if (q.front() == key)
          return true;
        q.pop();
      }
      return false;
    }

  public:
    static inline size_t npos = -1;

    graph() = default;

    graph(const HashMap& values) : values(values) {}
    graph(HashMap&& values) : values(std::move(values)) {}

    Collection& operator[](const Key& key) {
      return values[key];
    }

    std::pair<bool, size_t> findShortestPath(const Key& from, const Key& to) {
      if (from == to)
        return { true, 0 };

      std::queue<Collection> search_queue;
      search_queue.push(values[from]);

      std::queue<Key> searched;

      size_t countValuesInLevel = 1;
      for (size_t i = 1; countValuesInLevel != 0; ++i) {
        size_t tempCountValuesInLevel = 0;
        for (size_t j = 0; j < countValuesInLevel; ++j) {
          for (auto& x : search_queue.front()) {
            if (!isContainsElement(searched, x)) {
              if (x == to) {
                return { true, i };
              }
              else if (values.contains(x)) {
                ++tempCountValuesInLevel;
                search_queue.push(values[x]);
                searched.push(x);
              }
            }
          }
          search_queue.pop();
        }
        countValuesInLevel = tempCountValuesInLevel;
      }

      return { false, npos };
    }
  };

}
