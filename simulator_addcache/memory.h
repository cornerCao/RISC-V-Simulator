#ifndef CACHE_MEMORY_H_
#define CACHE_MEMORY_H_

#include <stdint.h>
#include "storage.h"
#include <string.h>

class Memory: public Storage {
 public:
  Memory(int At) {
    memset(mainMemory,0,sizeof(mainMemory));
    AccessTime=At;}
  //Memory(int At){AccessTime=At;name="Memory";}
  ~Memory() {}

  // Main access process
  void HandleRequest(unsigned long addr, unsigned int bytes, int read,
                     char *content, int &hit, int &cycle);

 private:
  // Memory implement
  int AccessTime=50;
  char mainMemory[400000000];
  char *name;
  DISALLOW_COPY_AND_ASSIGN(Memory);
};

#endif //CACHE_MEMORY_H_ 
