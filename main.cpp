#include <chrono>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>

struct Record {
    int16_t min, max;
    int32_t count = 0;
    int64_t sum = 0;
};

struct StringHash {
    using is_transparent = void;

    size_t operator()(std::string_view sv) const noexcept {
        return std::hash<std::string_view>{}(sv);
    }
};

void extract_temp(std::string::iterator end, int16_t& num){
    num = 0;
    int16_t mul = 1;

    while(*--end != ';'){
        if(*end == '.'){
            continue;
        }

        if(*end == '-'){
            num *= -1;
            continue;
        }

        num += (*end - '0') * mul;
        mul *= 10;
    }
}

void print_record(std::ostream& out, std::string_view name, const Record& r){
    out << name
        << " | min: "   << r.min / 10.0
        << " | max: "   << r.max / 10.0
        << " | avg: "  << static_cast<double>(r.sum) / r.count / 10.0
        << " | sum: "   << r.sum / 10.0
        << " | count: " << r.count
        << '\n';
}

int main(int argc, char* argv[]){
    const std::string input_path = argc > 1
            ? argv[1]
            : "gas_stations.csv";


    std::ifstream file(input_path);

    if(!file.is_open()){
        throw std::runtime_error("unable to open the input file");
    }

    std::unordered_map<std::string, Record, StringHash, std::equal_to<>> map = std::unordered_map<std::string, Record, StringHash, std::equal_to<>>();
    map.reserve(10'000);

    std::string line;
    int16_t num;

    auto start = std::chrono::high_resolution_clock::now();
    while(std::getline(file, line)){
        size_t semi_idx = line.find(';');

        std::string_view name(line.data(), semi_idx);
        extract_temp(line.end(), num);

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
            map.emplace(std::string(name), Record{num, num, 1, num});
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;

    std::cout << std::fixed << std::setprecision(1);
    for (const auto& [key, value] : map) {
        print_record(std::cout, key, value);
    }

    std::cerr << "elapsed: " << elapsed.count() << " ms\n";

    return 0;
}
