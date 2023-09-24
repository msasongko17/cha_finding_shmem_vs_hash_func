#include "protocol.h"

#include <cstdlib>
#include <sys/mman.h>
#include <cinttypes>
#include <cstring>

#include <iostream>
#include <array>
#include <cassert>
#include <chrono>

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::milliseconds;
using std::chrono::microseconds;

std::array<int, NUM> findWithShm(const int *const data)
{
    std::array<int, NUM> res;

  for (int i = 0; i < NUM; ++i) {
    // std::cout << "data[" << i << "]: " << data[i] << ", CHA of &data[" << i << "]: " << data[i + NUM] << std::endl;
    res[i] = data[i + NUM];
  }

    return res;
}

std::array<int, NUM> findWithHashFunc(const int *const data)
{
    std::array<int, NUM> res;

  for (int i = 0; i < NUM; ++i) {
    // std::cout << "data[" << i << "]: " << data[i] << ", CHA of &data[" << i << "]: " << data[i + NUM] << std::endl;
    res[i] = findCHAByHashing(uintptr_t(&data[i]), base_sequence_28_skx);
  }

    return res;
}

template <typename T>
std::array<int, NUM> findChasOfArray(T *arr)
{
    std::array<int, NUM> res;

    auto addr = static_cast<T *>(&arr[0]);

    // #pragma omp parallel for
    for (int i = 0; i < NUM;)
    {
        addr = static_cast<T *>(&arr[i]);
        if (addr)
        {
            // SPDLOG_INFO("printing the int the addr points to in order to make it page allocated! => {}", *addr);
            // const void* addr2 = addr; /// important to do this, so that a page would be mapped for the address by OS. this is _important_ for finding cha by
            // *addr = 0;  /// important to write to this address so that a page would be mapped for the address by OS. this is _important_ for finding cha by
            // hashing method to not see the effects of lazy mapping. I DO NOT WANT TO WRITE IN THIS CASE!
        }
        else
        {
            //            SPDLOG_ERROR("addr is nullptr.");
            std::exit(EXIT_FAILURE);
        }

        const int cha = findCHAByHashing(reinterpret_cast<uintptr_t>(addr), base_sequence_28_skx);
        // const auto socket_cha = findCHAPerfCounter(reinterpret_cast<long long*>(addr));
        // std::cout << "i: " << i << std::endl;
        res[i] = cha;
        ++i;

        for (int j = 0; j < CACHE_LINE_SIZE / sizeof(T) - 1; ++j)
        {
            if (i < NUM)
            {
                // std::cout << "j: " << j << ", i: " << i << std::endl;
                // auto addr = static_cast<const int *>(&arr[i]);
                // const int cha_ = findCHAByHashing(reinterpret_cast<uintptr_t>(addr), base_sequence_18_skx);
                // assert(cha_ == cha);
                res[i] = cha;
                ++i;
            }
            else
            {
                break;
            }
        }

        //        SPDLOG_INFO("cha: {}", cha);
        // ++addr;
    }

    return res;
}

int main(int argc, char** argv) {
    if (geteuid() != 0) {
        std::cerr << "This program must be run as root!" << std::endl;
        return 1; // exit with an error code
    }

    std::cout << "Running with root privileges." << std::endl;
    std::cout << "NUM: " << NUM << std::endl;

    int destroy = 0;
    printf("argv[1]: %s\n", argv[1]);
    if(argc == 2 && strcmp("1", argv[1]) == 0) {
        destroy = 1;
    }

  const int fd = shm_open(NAME, O_RDONLY, 0666);
  if (fd < 0) {
    perror("shm_open()");
    return EXIT_FAILURE;
  }

  int *const data = static_cast<int *const>(mmap(nullptr, SIZE, PROT_READ, MAP_SHARED, fd, 0));
    if (data == MAP_FAILED) {
        perror("mmap failed");
        std::exit(EXIT_FAILURE);
    }

  printf("consumer mapped address: %p\n", data);

  for (int i = 0; i < NUM; ++i) {
    std::cout << "data[" << i << "]: " << data[i] << ", CHA of &data[" << i << "]: " << data[i + NUM] << std::endl; // read it here so that the address is mapped in so that physical address conversion does not mess up.
  }

  printf("physical address: %" PRIxPTR ", cha: %d\n", getPhysicalAddress(((uintptr_t)data)), findCHAByHashing(uintptr_t(data), base_sequence_28_skx));

    std::cout << "now benchmarking..." << std::endl;

    std::array<int, NUM> shm_arr;
    long long elapsed_shm = -1;
    {
        auto t1 = high_resolution_clock::now();
        shm_arr = findWithShm(data);
        auto t2 = high_resolution_clock::now();
        auto ms_int = duration_cast<microseconds>(t2 - t1);
        elapsed_shm = ms_int.count();
        std::cout << "shm arr: " << elapsed_shm << " us\n";
    }

    std::array<int, NUM> func_arr;
    long long elapsed_func = -1;
    {
        auto t1 = high_resolution_clock::now();
        func_arr = findWithHashFunc(data);
        auto t2 = high_resolution_clock::now();
        auto ms_int = duration_cast<microseconds>(t2 - t1);
        elapsed_func = ms_int.count();
        std::cout << "func arr: " << elapsed_func << " us\n";  
    }

    std::array<int, NUM> func_all_arr;
    long long elapsed_func_all = -1;
    {
        auto t1 = high_resolution_clock::now();
        func_all_arr = findChasOfArray<int>(data);
        auto t2 = high_resolution_clock::now();
        auto ms_int = duration_cast<microseconds>(t2 - t1);
        elapsed_func_all = ms_int.count();
        std::cout << "func all arr: " << elapsed_func_all << " us\n";  
    }    

    std::cout << "diff: " << elapsed_func / static_cast<double>(elapsed_shm) << std::endl;
    std::cout << "diff all: " << elapsed_func_all / static_cast<double>(elapsed_shm) << std::endl;

    for(int i = 0; i < NUM; ++i) {
        assert(shm_arr[i] == func_arr[i]);
        assert(shm_arr[i] == func_all_arr[i]);
    }
    
    std::cout << "cha assert success." << std::endl;


  munmap(data, SIZE);
  close(fd);

if(destroy) {
    printf("destroyed.\n");
    shm_unlink(NAME);
}
  

  return EXIT_SUCCESS;
}