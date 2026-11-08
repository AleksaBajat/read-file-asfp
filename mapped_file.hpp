#pragma once

#include <cstddef>
#include <stdexcept>
#include <string_view>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

class MappedFile {
public:
    explicit MappedFile(const char* file_name) {
        fd = open(file_name, O_RDONLY);
        if (fd == -1) {
            throw std::runtime_error("unable to open the file");
        }

        struct stat sb;
        if (fstat(fd, &sb) == -1) {
            close(fd);
            throw std::runtime_error("unable to get file size");
        }

        size = sb.st_size;
        if (size == 0) {
            return;
        }

        data = mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0);
        if (data == MAP_FAILED) {
            close(fd);
            throw std::runtime_error("mmap failed");
        }
    }

    ~MappedFile() {
        if (data != MAP_FAILED) munmap(data, size);
        if (fd != -1) close(fd);
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

private:
    int fd = -1;
    void* data = MAP_FAILED;
    size_t size = 0;
};
