#include <chrono>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

struct Record {
    int16_t min, max;
    int32_t count = 0;
    int64_t sum = 0;
};

class MappedFile {
    private:
        int fd = -1;
        void *data = MAP_FAILED;
        size_t size = 0;

    public:
    MappedFile(const char* file_name){
        fd = open(file_name, O_RDONLY);

        if (fd == -1){
            throw std::runtime_error("unable to open the file");
        }

        struct stat sb;
        if (fstat(fd, &sb) == -1){
            close(fd);
            throw std::runtime_error("unable to get file size");
        }

        size = sb.st_size;

        if(size == 0){
            return;
        }

        data = mmap(nullptr, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

        if(data == MAP_FAILED){
            close(fd);
            throw std::runtime_error("mmap failed");
        }
    }

    ~MappedFile(){
        if(data != MAP_FAILED){
            munmap(data, size);
        }

        if(fd != -1){
            close(fd);
        }
    }

    MappedFile(const MappedFile&) = delete;
    MappedFile& operator=(const MappedFile&) = delete;

    MappedFile(MappedFile&& other) noexcept
         : fd(other.fd), data(other.data), size(other.size) {
         other.fd = -1;
         other.data = MAP_FAILED;
         other.size = 0;
    }

     std::string_view view() const noexcept {
         if (data == MAP_FAILED || size == 0) return {};
         return {static_cast<const char*>(data), size};
     }
};

struct StringHash {
    using is_transparent = void;

    size_t operator()(std::string_view sv) const noexcept {
        return std::hash<std::string_view>{}(sv);
    }
};

void extract_temp(std::string_view temp, int16_t& num){
    num = 0;
    int16_t mul = 1;
    auto end = temp.end();

    while(end != temp.begin()){
        --end;
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

    std::unordered_map<std::string, Record, StringHash, std::equal_to<>> map = std::unordered_map<std::string, Record, StringHash, std::equal_to<>>();
    map.reserve(10'000);
    int16_t num;

    MappedFile file = MappedFile(input_path.c_str());
    auto view = file.view();
    auto it_view = view.begin();

    auto start = std::chrono::high_resolution_clock::now();
    while(it_view != view.end()){
        std::string_view name;
        std::string_view temp;

        auto left = it_view;

        while(*it_view != ';'){
            it_view++;
        }

        name = std::string_view{left, it_view};
        it_view++;
        left = it_view;

        while(*it_view != '\n'){
            it_view++;
        }

        temp = std::string_view{left, it_view};
        it_view++;

        extract_temp(temp, num);

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
