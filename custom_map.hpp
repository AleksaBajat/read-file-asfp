#pragma once

#include <cstdint>
#include <algorithm>
#include <string_view>
#include <utility>
#include <vector>

struct Record {
  int16_t min, max;
  int32_t count = 0;
  int64_t sum = 0;

  Record() : min(INT16_MAX), max(INT16_MIN) {}
  explicit Record(int16_t v) : min(v), max(v), count(1), sum(v) {}

  void add(int16_t v) {
    min = std::min(min, v);
    max = std::max(max, v);
    sum += v;
    ++count;
  }
};


class CustomMap {
    private:
        struct Entry {
            std::string_view key;
            uint64_t hash = 0;
            Record record;
        };

        static constexpr uint64_t slots = uint64_t{1} << 15;
        std::vector<Entry> entries;
        uint64_t mask;

        const uint64_t slot(uint64_t hash){
                return hash & mask;
        }

    public:
        CustomMap():
        entries(slots), mask(slots - 1){}

        void insert(std::string_view key, uint64_t hash, int16_t val){
            uint64_t s = slot(hash);

            while(!entries[s].key.empty() && entries[s].hash != hash){
                s = (s + 1) & mask;
            }

            entries[s].key = key;
            entries[s].hash = hash;
            entries[s].record.add(val);
        }

        class const_iterator {
            public:
                using value_type = std::pair<std::string_view, const Record &>;

                const_iterator(const Entry *cur, const Entry *end) : cur(cur), end(end) {
                    skip_empty();
                }

                value_type operator*() const { return {cur->key, cur->record}; }

                const_iterator &operator++() {
                    ++cur;
                    skip_empty();
                    return *this;
                }

                bool operator!=(const const_iterator &other) const { return cur != other.cur; }
                bool operator==(const const_iterator &other) const { return cur == other.cur; }

            private:
                const Entry *cur;
                const Entry *end;

                void skip_empty() {
                    while (cur != end && cur->key.empty()) ++cur;
                }
        };

        const_iterator begin() const {
            return {entries.data(), entries.data() + entries.size()};
        }
        const_iterator end() const {
            const Entry *e = entries.data() + entries.size();
            return {e, e};
        }
};
