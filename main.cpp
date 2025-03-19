#include <iostream>
#include <vector>
#include <algorithm>  
#include "cache_size.h" 
#include "kaizen.h"
using CacheDetector::CHUNK_SIZE;


int process_args(int argc, char* argv[]) {
    zen::print("The cache is " ,CHUNK_SIZE,"KB",'\n');
    if (argc < 2) {
        std::cout << "Error: --size argument is absent, using default 1000!" << std::endl;
        return 50000000; // Default size
    }
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--size") {
            if (i + 1 < argc) {
                return std::atoi(argv[i + 1]); // Convert KB to number of integers
            }
        }
    }
    return 50000000;
}

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
    int chunk_size = ((CHUNK_SIZE * 1024)/sizeof(int))/2; 
    std::vector<int> temp(n);

    // Sort each chunk independently
    for (int i = 0; i < n; i += chunk_size) {
        int end =std::min(static_cast<int>(i + chunk_size - 1), n - 1); 
        merge_sort(v, i, end, temp);
    }

    // Merge sorted chunks iteratively
    int size = chunk_size;
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
    int size = process_args(argc, argv); // Convert KB to number of integers
    zen::timer timer;


    std::vector<int> data(size);
    for(int i = 0; i < size; i++) {
        data[i] = zen::random_int(0,size);
    }

    
    timer.start();
    chunk_sort(data);
    timer.stop();
    std::cout << "Chunk Sort Time: " << timer.duration_string() << std::endl;


    data = std::vector<int>(size);
    for(int i = 0; i < size; i++) {
        data[i] = zen::random_int(0,size);
    }
    timer.start();
    std::vector<int> temp(data.size());
    merge_sort(data, 0, data.size() - 1, temp);
    timer.stop();
    std::cout << "Standard Merge Sort Time: " << timer.duration_string() << std::endl;

    std::cout << (std::is_sorted(data.begin(), data.end()) ? "Sorted!" : "Not Sorted!") << std::endl;


    return 0;
}