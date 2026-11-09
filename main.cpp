#include <chrono>
#include <cstdint>
#include <fcntl.h>
#include <functional>
#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include "custom_map.hpp"
#include "mapped_file.hpp"


struct StringHash {
  using is_transparent = void;

  size_t operator()(std::string_view sv) const noexcept {
    return std::hash<std::string_view>{}(sv);
  }
};

int16_t extract_temp(std::string_view temp) {
  int16_t num = 0;
  int16_t mul = 1;
  auto end = temp.end();

  while (end != temp.begin()) {
    --end;
    if (*end == '.') {
      continue;
    }

    if (*end == '-') {
      num *= -1;
      continue;
    }

    num += (*end - '0') * mul;
    mul *= 10;
  }

  return num;
}

void print_record(std::ostream &out, std::string_view name, const Record &r) {
  out << name << " | min: " << r.min / 10.0 << " | max: " << r.max / 10.0
      << " | avg: " << static_cast<double>(r.sum) / r.count / 10.0
      << " | sum: " << r.sum / 10.0 << " | count: " << r.count << '\n';
}

CustomMap aggregate(std::string_view data){
    CustomMap map;
    auto it_view = data.begin();

    while (it_view != data.end()) {
      std::string_view name;
      std::string_view temp;
      std::uint64_t hash = 0xcbf29ce484222325u;

      auto left = it_view;

      while (*it_view != ';') {
        hash ^= static_cast<unsigned char>(*it_view);
        hash *= 0x100000001b3u;
        it_view++;
      }

      name = std::string_view{left, it_view};
      it_view++;
      left = it_view;

      while (*it_view != '\n') {
        it_view++;
      }

      temp = std::string_view{left, it_view};
      it_view++;

      int16_t temp_val = extract_temp(temp);

      map.insert(name, hash, temp_val);
    }

    return map;
}

int main(int argc, char *argv[]) {
  const std::string input_path = argc > 1 ? argv[1] : "gas_stations.csv";


  MappedFile file(input_path.c_str());
  auto start = std::chrono::steady_clock::now();
  auto map = aggregate(file.view());
  auto end = std::chrono::steady_clock::now();
  std::chrono::duration<double, std::milli> elapsed = end - start;

  std::cout << std::fixed << std::setprecision(1);
  for (const auto &[key, value] : map) {
    print_record(std::cout, key, value);
  }

  std::cerr << "elapsed: " << elapsed.count() << " ms\n";

  return 0;
}
