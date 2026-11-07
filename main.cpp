#include <charconv>
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>

struct Record {
    double min = std::numeric_limits<double>::max(), max = std::numeric_limits<double>::lowest();
    double sum = 0;
    int count = 0;
};

struct StringHash {
    using is_transparent = void;

    size_t operator()(std::string_view sv) const noexcept {
        return std::hash<std::string_view>{}(sv);
    }
};

int main(int argc, char* argv[]){
    const std::string input_path = argc > 1
            ? argv[1]
            : "gas_stations.csv";


    std::ifstream file("gas_stations.csv");

    if(!file.is_open()){
        throw std::runtime_error("unable to open the input file");
    }

    std::unordered_map<std::string, Record, StringHash, std::equal_to<>> map = std::unordered_map<std::string, Record, StringHash, std::equal_to<>>();
    map.reserve(10'000);

    std::string line;
    double num;

    auto start = std::chrono::high_resolution_clock::now();
    while(std::getline(file, line)){
        size_t semi_idx = line.find(';');

        std::string_view name(line.data(), semi_idx);
        std::from_chars(std::to_address(line.begin() + semi_idx + 1), std::to_address(line.end()), num);

        if(auto entry = map.find(name); entry != map.end()){
            if(num > entry->second.max){
                entry->second.max = num;
            }

            if(num < entry->second.min){
                entry->second.min = num;
            }

            entry->second.sum += num;
            entry->second.count++;
        }else{
            map.emplace(std::string(name), Record{num, num, num, 1});
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;

    std::cout << "elapsed: " << elapsed.count() << " ms\n";

    return 0;
}
