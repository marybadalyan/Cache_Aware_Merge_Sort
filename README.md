# Cache-Oblivious Merge Sort Implementation

This project implements a cache-oblivious merge sort algorithm with chunk-based optimization, demonstrating how it naturally takes advantage of cache locality while remaining cache-oblivious. The implementation is written in C++ and uses the `kaizen.h` library for printing and argument parsing.

## Overview

Merge sort is inherently cache-oblivious because it divides the problem into smaller subproblems recursively, which naturally aligns with cache hierarchies without explicitly knowing the cache size. However, this implementation introduces a chunk-based approach that leverages cache locality more effectively by processing data in chunks that fit into a cache line (default: 64 bytes).

### Key Features
- **Cache-Oblivious Nature**: The algorithm doesn't require explicit knowledge of cache parameters, yet it benefits from spatial and temporal locality due to its divide-and-conquer strategy.
- **Chunk-Based Optimization**: Data is processed in chunks of size `CHUNK_SIZE` (default: 64 bytes, matching a typical cache line size). This improves performance by aligning memory access patterns with hardware prefetching and cache line utilization.
- **In-Place Merging**: Uses an optimized gap-based in-place merge to minimize temporary memory usage, which performs better than traditional merging in practice due to reduced memory allocation overhead.
- **Performance Measurement**: Compares the chunk-optimized version with a standard merge sort, averaging results over multiple iterations for accuracy.

## Dependencies
- [**`kaizen.h`**](https://github.com/heinsaar/kaizen): A custom library used for:
  - Pretty printing (`zen::print`)
  - Command-line argument parsing (`zen::cmd_args`)
  - Timing measurements (`zen::timer`)
  - Random number generation (`zen::random_int`)
 - **`cache_info.h`**: Header file for detecting L1 cache size using CPUID (optional).
 - C++20 compatible compiler [support for `<format>` alternative use ```zen::print()```.
 - Standard C++ libraries: `<iostream>`, `<vector>`, `<algorithm>`, `<iomanip>`, `<format>`.
# Cache-Oblivious Merge Sort Implementation

This project implements a cache-oblivious merge sort algorithm with chunk-based optimization, demonstrating how it naturally leverages cache locality while remaining oblivious to specific cache sizes. Optionally, it can use a cache-aware approach via the `cache_info.h` header to detect the L1 cache size dynamically. The implementation is written in C++ and uses the `kaizen.h` library for printing and argument parsing.

### Chunk Size Considerations
- **Default (64 bytes)**: Matches a typical cache line size, leveraging hardware prefetching for sequential access. This works better than using the full L1 cache size in practice, likely due to prefetcher efficiency.
- **Cache-Aware Option (`CacheDetector::CHUNK_SIZE`)**: Detects the L1 cache size (e.g., 32KB) using CPUID. While this makes the algorithm cache-aware, experiments show it yields less than a 0.1% performance improvement over the cache-oblivious 64-byte default. This suggests the cache-oblivious design is robust and prefetching compensates for larger chunk sizes.
-**Notes on Integration**
    - Conversion: CacheDetector::CHUNK_SIZE returns the L1 cache size in KB (e.g., 32KB). Multiply by 1024 to convert to bytes, then divide by sizeof(int) (typically 4) to get the number of integers. For a 32KB L1 cache, this yields 32 * 1024 / 4 = 8192 integers.
    - Trade-Off: Using the full L1 cache size (e.g., 8192 integers) increases the working set significantly compared to 16 integers (64 bytes), which may reduce prefetching efficiency and increase cache misses. This explains why the 64-byte default often performs better.

### Why Chunk Size = Cache Line Size?
- The chunk size is set to 64 bytes (one cache line) rather than the full L1 cache size (e.g., 32KB). This works better in practice, likely due to hardware prefetching, which anticipates sequential memory access within cache lines.
- Using the full cache size as the chunk size could lead to more cache misses as the working set exceeds the cache line prefetching window.
- Experiments show that making the algorithm explicitly cache-aware (e.g., tuning for L1/L2 cache sizes) yields less than a 0.1% performance difference, validating the cache-oblivious design.
  
## Build Instructions

1. **Clone the repository**:
    ```bash
    git clone https://github.com/marybadalyan/Cache_Aware_Oblivious_Merge_Sort
    ```

2. **Go into the repository**:
    ```bash
    cd Cache_Aware_Oblivious_Merge_Sort
    ```

3. **Generate the build files**:
    ```bash
    cmake -DCMAKE_BUILD_TYPE=Release -S . -B build
    ```

4. **Build the project**:
    ```bash
    cmake --build build --config Release
    ```

5. **Run the executable** generated in the build directory:
    ```bash
    ./build/Cache_Aware_Oblivious_Merge_Sort
    ```

## Usage
Once compiled, run the program to start the memory stress test:

```bash
./Cache_Aware_Oblivious_Merge_Sort --size [num] --iter [num] // num as in int 
```
