#include <iostream>
#include <vector>
#include <algorithm>  
#include "cache_size.h" 
#include "kaizen.h"
using CacheDetector::CHUNK_SIZE;

//TODO: 1: Implement chunk sort to not increment the size of subvectors that ...
//... need to be merged [do it with pairs]
//TODO: 2: Add run for git hub actions yaml file
//TODO: 3: Add iterations for accurate time measurement
int process_args(int argc, char* argv[]) {
    zen::print("The cache is " ,CHUNK_SIZE,"KB",'\n');
    zen::cmd_args args(argv, argc);

    auto size_options = args.get_options("--size");

    if (size_options.empty()) {
        zen::log("Error: --size argument is absent, using default 500!");
        return 500;
    }
    return std::stoi(size_options[0]);
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
        size *= 2; //incerement the size of the chunks to be merged 
    }
}
int main(int argc, char* argv[]) {
    int size = process_args(argc, argv);
    const int iterations = 10;  // Adjust as needed
    zen::timer timer;
    double chunk_total = 0.0, merge_total = 0.0;

    std::vector<int> data1(size), data2(size);
    for (int iter = 0; iter < iterations; iter++) {
        for (int i = 0; i < size; i++) {
            data1[i] = data2[i] = zen::random_int(0, size);
        }

        timer.start();
        chunk_sort(data1);
        timer.stop();
        chunk_total += timer.duration<zen::timer::nsec>().count();

        std::vector<int> temp(size);
        timer.start();
        merge_sort(data2, 0, data2.size() - 1, temp);
        timer.stop();
        merge_total += timer.duration<zen::timer::nsec>().count();
    }

    std::cout << "Average Chunk Sort Time: " << (chunk_total / iterations) << " nsec" << std::endl;
    std::cout << "Average Merge Sort Time: " << (merge_total / iterations) << " nsec" << std::endl;



    return 0;
}