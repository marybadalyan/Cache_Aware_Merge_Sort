
//TODO: 1: Implement chunk sort to not increment the size of subvectors that ...
//... need to be merged [do it with pairs] ✅
//TODO: 2: Add run for git hub actions yaml file ❌
//TODO: 3: Add iterations for accurate time measurement ✅
//TODO: 4: Maybe use insertSort for chunk sorting or find a better way to sort the chunks without copy() ✅
#include <iostream>
#include <vector>
#include <algorithm>  
#include "cache_size.h" 
#include "kaizen.h"

using CacheDetector::CHUNK_SIZE;

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

// Existing inPlaceMerge and merge_sort functions remain unchanged
int nextGap(int gap) {
    if (gap <= 1) return 0;
    return (gap + 1) / 2;
}

void inPlaceMerge(std::vector<int>& v, int left, int mid, int right) {
    int i = left, j = mid + 1;
    
    while (i <= mid && j <= right) {
        if (v[i] <= v[j]) {
            i++;
        } else {
            int value = v[j];
            int index = j;

            while (index > i) {
                v[index] = v[index - 1];
                index--;
            }
            v[i] = value;

            i++; mid++; j++;
        }
    }
}

void merge_sort(std::vector<int>& v, int left, int right) {
    if (left >= right) return;

    int mid = left + (right - left) / 2;
    merge_sort(v, left, mid);
    merge_sort(v, mid + 1, right);
    inPlaceMerge(v, left, mid, right);
}

void chunk_sort(std::vector<int>& v) {
    int n = v.size();
    int chunk_size = ((CHUNK_SIZE * 1024) / sizeof(int)) / 2;

    for (int i = 0; i < n; i += chunk_size) {
        int end = std::min(i + chunk_size - 1, n - 1);
        merge_sort(v, i, end);
    }

    std::vector<int> temp = v;
    bool use_temp = false;
    
    for (int size = chunk_size; size < n; size *= 2) {
        std::vector<int>& source = use_temp ? temp : v;
        std::vector<int>& target = use_temp ? v : temp;

        for (int left = 0; left < n; left += 2 * size) {
            int mid = std::min(left + size - 1, n - 1);
            int right = std::min(left + 2 * size - 1, n - 1);

            if (mid < right) {
                int i = left;
                int j = mid + 1;
                int k = left;

                while (i <= mid && j <= right) {
                    if (source[i] <= source[j]) {
                        target[k++] = source[i++];
                    } else {
                        target[k++] = source[j++];
                    }
                }

                while (i <= mid) {
                    target[k++] = source[i++];
                }
                while (j <= right) {
                    target[k++] = source[j++];
                }
            } else {
                for (int k = left; k <= mid; k++) {
                    target[k] = source[k];
                }
            }
        }
        
        int processed = ((n-1)/(2*size))*(2*size);
        for (int i = processed; i < n; i++) {
            target[i] = source[i];
        }
        
        use_temp = !use_temp;
    }

    if (use_temp) {
        for (int i = 0; i < n; i++) {
            v[i] = temp[i];
        }
    }
}

int main(int argc, char* argv[]) {
    int size = process_args(argc, argv);
    const int iterations = 10;  
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

        timer.start();
        merge_sort(data2, 0, data2.size() - 1);
        timer.stop();
        merge_total += timer.duration<zen::timer::nsec>().count();
    }

    std::cout << "Average Chunk Sort Time: " << (chunk_total / iterations) << " nsec" << std::endl;
    std::cout << "Average Merge Sort Time: " << (merge_total / iterations) << " nsec" << std::endl;

    return 0;
}