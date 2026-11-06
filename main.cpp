#include <charconv>
#include <chrono>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <string>

struct Record {
    double min = std::numeric_limits<double>::max(), max = std::numeric_limits<double>::min();
    double sum = 0;
    int count = 0;
};

int main(){
    auto start = std::chrono::high_resolution_clock::now();
    std::string line;
    std::ifstream file("gas_stations.csv");

    Record result = Record();

    double num;

    while(std::getline(file, line)){
        size_t idx = line.find(';');
        std::from_chars(std::to_address(line.begin() + idx + 1), std::to_address(line.end()), num);

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
    std::cout << "min: " << result.min << std::endl;
    std::cout << "max: " << result.max << std::endl;
    std::cout << "avg: " << result.sum / result.count << std::endl;

    return 0;
}
