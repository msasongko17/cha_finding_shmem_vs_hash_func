#include "protocol.h"

#include <cstdio>
#include <sys/mman.h>
#include <cinttypes>
#include <cstring>

#include <iostream>

// http://logan.tw/posts/2018/01/07/posix-shared-memory/
int main() {
    if (geteuid() != 0) {
        std::cerr << "This program must be run as root!" << std::endl;
        return 1; // exit with an error code
    }

    std::cout << "Running with root privileges." << std::endl;
    std::cout << "NUM: " << NUM << std::endl;

  const int fd = shm_open(NAME, O_CREAT | O_EXCL | O_RDWR, 0600);
  if (fd < 0) {
    perror("shm_open()");
    return EXIT_FAILURE;
  }

  ftruncate(fd, SIZE);

  int *const data = static_cast<int *const>(mmap(nullptr, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    if (data == MAP_FAILED) {
        perror("mmap failed");
        std::exit(EXIT_FAILURE);
    }      

  for (int i = 0; i < NUM; ++i) {
    data[i] = i * 13;
    data[i + NUM] = findCHAByHashing((uintptr_t(&data[i])), base_sequence_28_skx);
    // std::cout << "data[" << i << "]: " << data[i] << ", CHA of &data[" << i << "]: " << data[i + NUM] << std::endl;
  }

  // print here so that physical address does not get printed as 0 as a result of the address not being mapped to the actual DRAM yet. by writing into the addresses
  // above, we make sure that mapping is done.
  printf("producer mapped virtual address: %p, physical address: %" PRIxPTR ", cha: %d\n", data, 
    getPhysicalAddress(((uintptr_t)data)), findCHAByHashing((uintptr_t(data)), base_sequence_28_skx));

    // printf("waiting for char input...\n");
    // getchar();

  munmap(data, SIZE);
  printf("unmapped\n");
  close(fd);
    printf("closed fd\n");


  return EXIT_SUCCESS;
}