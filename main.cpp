
//TODO: 1: Implement chunk sort to not increment the size of subvectors that ...
//... need to be merged [do it with pairs] ✅
//TODO: 2: Add run for git hub actions yaml file ✅
//TODO: 3: Add iterations for accurate time measurement ✅
//TODO: 4: Maybe use insertSort for chunk sorting or find a better way to sort the chunks without copy() ✅
#include <iostream>
#include <vector>
#include <algorithm>
#include "cache_size.h" 
#include "kaizen.h"

using CacheDetector::CHUNK_SIZE;

int process_args(int argc, char* argv[]) {
    zen::print("The cache is ", CHUNK_SIZE, "KB", '\n');
    zen::cmd_args args(argv, argc);
    auto size_options = args.get_options("--size");

    if (size_options.empty()) {
        zen::log("Error: --size argument is absent, using default 500!");
        return 500;
    }
    try {
        int size = std::stoi(size_options[0]);
        if (size <= 0) throw std::out_of_range("Size must be positive");
        return size;
    } catch (const std::exception& e) {
        zen::log("Error: Invalid size argument, using default 500!");
        return 500;
    }
}

// Optimized gap-based in-place merge
int nextGap(int gap) {
    if (gap <= 1) return 0;
    return (gap + 1) / 2;
}

void inPlaceMerge(std::vector<int>& v, int left, int mid, int right) {
    int gap = nextGap(right - left + 1);
    while (gap > 0) {
        for (int i = left; i + gap <= right; i++) {
            if (v[i] > v[i + gap]) {
                std::swap(v[i], v[i + gap]);
            }
        }
        gap = nextGap(gap);
    }
}

// Optimized merge sort with minimal temporary space
void merge_sort(std::vector<int>& v, int left, int right, std::vector<int>& temp) {
    if (left >= right) return;
    
    int mid = left + (right - left) / 2;
    merge_sort(v, left, mid, temp);
    merge_sort(v, mid + 1, right, temp);
    inPlaceMerge(v, left, mid, right);
}

// Optimized chunk sort using merge sort throughout
void chunk_sort(std::vector<int>& v, std::vector<int>& temp) {
    int n = v.size();
    int chunk_size = std::max(1, static_cast<int>((CHUNK_SIZE * 1024) / sizeof(int)) / 2);

    // Sort each chunk using merge sort
    for (int i = 0; i < n; i += chunk_size) {
        int end = std::min(i + chunk_size - 1, n - 1);
        merge_sort(v, i, end, temp);
    }

    // Merge sorted chunks with increasing sizes
    for (int size = chunk_size; size < n; size *= 2) {
        for (int i = 0; i < n; i += 2 * size) {
            int mid = std::min(i + size - 1, n - 1);
            int right_end = std::min(i + 2 * size - 1, n - 1);
            if (mid < right_end) {
                inPlaceMerge(v, i, mid, right_end);
            }
        }
    }
}

int main(int argc, char* argv[]) {
    int size = process_args(argc, argv);
    const int iterations = 10;
    zen::timer timer;
    
    // Single allocation for temporary space
    std::vector<int> data(size), original(size), temp(size);

    // Warm-up run to prime caches
    for (int i = 0; i < size; i++) {
        original[i] = zen::random_int(0, size);
    }
    data = original;
    chunk_sort(data, temp);

    // Performance measurement
    double chunk_total = 0.0, merge_total = 0.0;
    
    for (int iter = 0; iter < iterations; iter++) {
        // Test chunk sort
        data = original;
        timer.start();
        chunk_sort(data, temp);
        timer.stop();
        chunk_total += timer.duration<zen::timer::nsec>().count();

        // Test standard merge sort
        data = original;
        timer.start();
        merge_sort(data, 0, size - 1, temp);
        timer.stop();
        merge_total += timer.duration<zen::timer::nsec>().count();
    }

    std::cout << "Average Chunk Sort Time: " << (chunk_total / iterations) << " nsec\n";
    std::cout << "Average Merge Sort Time: " << (merge_total / iterations) << " nsec\n";
    
    return 0;
}