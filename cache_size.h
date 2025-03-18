#ifndef CACHE_UTILS_H
#define CACHE_UTILS_H

#include <iostream>
#ifdef _WIN32
#include <windows.h>
#undef min // Prevent Windows macro from interfering with std::min
#else
#include <cstdlib>
#endif

namespace cache_utils {

inline int detectL1DataCacheSize() {
    int l1DataSizeKB = 32; // Default fallback

#ifdef _WIN32
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = nullptr;
    DWORD bufferSize = 0;

    if (GetLogicalProcessorInformation(buffer, &bufferSize) == FALSE && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
        buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(bufferSize);
        if (buffer && GetLogicalProcessorInformation(buffer, &bufferSize)) {
            PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = buffer;
            DWORD byteOffset = 0;
            while (byteOffset < bufferSize) {
                if (ptr->Relationship == RelationCache && ptr->Cache.Type == CacheData && ptr->Cache.Level == 1) {
                    std::cout << "L1 Data Cache Level: " << ptr->Cache.Level
                              << ", Size: " << (ptr->Cache.Size / 1024) << " KB, Type: Data" << std::endl;
                    l1DataSizeKB = ptr->Cache.Size / 1024;
                    break;
                }
                byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
                ptr++;
            }
        }
        if (buffer) free(buffer);
    }
    if (bufferSize == 0) {
        std::cerr << "Failed to retrieve L1 data cache information. Using default 32 KB." << std::endl;
    }
#else
    std::cout << "L1 data cache information retrieval not implemented for this platform. Using default 32 KB." << std::endl;
#endif

    return l1DataSizeKB;
}

inline int calculateChunkSize() {
    int l1DataSizeKB = detectL1DataCacheSize();
    int chunkSize = (l1DataSizeKB * 1024 / 4) / 8; // 1/8 of L1 data cache in integers (4-byte ints)
    std::cout << "Calculated CHUNK_SIZE: " << chunkSize << " integers (based on " << l1DataSizeKB << " KB L1 data cache)" << std::endl;
    return chunkSize > 0 ? chunkSize : 1024; // Ensure positive value, fallback to 1024
}

const int CHUNK_SIZE = calculateChunkSize();

} // namespace cache_utils

#endif // CACHE_UTILS_H