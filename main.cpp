#include <chrono>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>

struct Record {
    int16_t min = std::numeric_limits<int16_t>::max(), max = std::numeric_limits<int16_t>::lowest();
    int32_t count = 0;
    int64_t sum = 0;
};

void extract_temperature(std::string::iterator end, int32_t& num){
    num = 0;
    int32_t exp = 1;

    while (*--end != ';') {
        if(*end == '.'){
            continue;
        }

        if(*end == '-'){
            num *= -1;
            continue;
        }

        int32_t n = (*end - '0') * exp;
        exp *= 10;
        num += n;
    }
}

int main(){
    auto start = std::chrono::high_resolution_clock::now();
    std::string line;
    std::ifstream file("gas_stations.csv");

    Record result = Record();

    int32_t num;

    while(std::getline(file, line)){
        extract_temperature(line.end(), num);

        if(num > result.max){
            result.max = num;
        }

        if(num < result.min){
            result.min = num;
        }

        result.sum += num;
        result.count++;
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;

    std::cout << "elapsed: " << elapsed.count() << " ms\n";
    std::cout << "min: " << result.min / 10.0 << std::endl;
    std::cout << "max: " << result.max / 10.0 << std::endl;
    std::cout << "avg: " << result.sum / 10.0 / result.count << std::endl;

    return 0;
}
