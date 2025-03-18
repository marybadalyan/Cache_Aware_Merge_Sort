#include <iostream>
#include <vector>
#include <algorithm>  // for std::merge and std::min
#include <chrono>     // for high_resolution_clock
#include <malloc.h>   // for _aligned_malloc and _aligned_free
#include "cache_size.h"

using cache_utils::CHUNK_SIZE;

int process_args(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Error: --size argument is absent, using default 1000!" << std::endl;
        return 1000;
    }
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--size") {
            if (i + 1 < argc) {
                return std::atoi(argv[i + 1]);
            }
        }
    }
    return 1000;
}

// Custom timer class using high_resolution_clock
class Timer {
public:
    void start() {
        start_time = std::chrono::high_resolution_clock::now();
    }
    void stop() {
        end_time = std::chrono::high_resolution_clock::now();
    }
    long long duration_ms() const {
        return std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time, end_time;
};

void merge_sort(std::vector<int>& v, int left, int right, std::vector<int>& temp) {
    if (left >= right) return;

    int mid = left + (right - left) / 2;
    merge_sort(v, left, mid, temp);
    merge_sort(v, mid + 1, right, temp);

    std::merge(v.begin() + left, v.begin() + mid + 1,
               v.begin() + mid + 1, v.begin() + right + 1,
               temp.begin() + left);
    
    std::copy(temp.begin() + left, temp.begin() + right + 1, v.begin() + left);
}

void chunk_sort(std::vector<int>& v) {
    int n = v.size();
    std::vector<int> temp(n);

    // Sort each chunk independently
    for (int i = 0; i < n; i += CHUNK_SIZE) {
        int end = std::min(i + CHUNK_SIZE - 1, n - 1);
        merge_sort(v, i, end, temp);
    }

    // Merge sorted chunks iteratively
    int size = CHUNK_SIZE;
    while (size < n) {
        for (int i = 0; i < n; i += 2 * size) {
            int mid = std::min(i + size - 1, n - 1);
            int right_end = std::min(i + 2 * size - 1, n - 1);
            if (mid < right_end) {
                std::merge(v.begin() + i, v.begin() + mid + 1,
                           v.begin() + mid + 1, v.begin() + right_end + 1,
                           temp.begin() + i);
                std::copy(temp.begin() + i, temp.begin() + right_end + 1, v.begin() + i);
            }
        }
        size *= 2;
    }
}

int main(int argc, char* argv[]) {
    int size = process_args(argc, argv);
    Timer t;

    size_t data_size = size * sizeof(int);
    int* aligned_data = static_cast<int*>(_aligned_malloc(data_size, 64));

    if (!aligned_data) {
        std::cerr << "Memory allocation failed" << std::endl;
        return 1;
    }

    // Initialize with random values
    srand(static_cast<unsigned>(time(0))); // Seed for random numbers
    for (int i = 0; i < size; ++i) {
        aligned_data[i] = rand() % size; // Simple random number generation
    }

    std::vector<int> data(aligned_data, aligned_data + size);

    t.start();
    chunk_sort(data);
    t.stop();
    std::cout << "Chunk Sort Time: " << t.duration_ms() << " ms" << std::endl;

    t.start();
    merge_sort(data, 0, data.size() - 1, std::vector<int>(data.size()));
    t.stop();
    std::cout << "Standard Merge Sort Time: " << t.duration_ms() << " ms" << std::endl;

    std::cout << (std::is_sorted(data.begin(), data.end()) ? "Sorted!" : "Not Sorted!") << std::endl;

    _aligned_free(aligned_data);

    return 0;
}