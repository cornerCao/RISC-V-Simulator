#ifndef CACHE_CACHE_H_
#define CACHE_CACHE_H_

#include <stdint.h>
#include "storage.h"
#include <math.h>
typedef struct CacheConfig_ {
  int size;
  int associativity;
  int set_num; // Number of cache sets
  int write_through; // 0|1 for back|through
  int write_allocate; // 0|1 for no-alc|alc
  int AccessTime; //
  int block_size;
} CacheConfig;

#define get_log(x) (unsigned int)(log(x)/log(2))
#define get_offset(x,bs) (unsigned long)x&(bs-1)
#define get_half(x,bs) ((unsigned long)x)>>(get_log(bs))
#define get_tag(x,ss) ((unsigned long)x)>>get_log(ss)
#define get_set(x,ss) ((unsigned long)x)&(ss-1)
#define get_initial(x,bs) x<<get_log(bs)
#define equal(x,y) x==y

struct block{
  //int block_size;
  long tag;//tag=tag+set
  char* content;//point to the first block addr
  bool valid;
  int flags;
  bool dirty;
};

struct Entity{
  block* content;//
  int associativity;
  int set_num;
  int block_num;
  int block_size;
};
class Cache: public Storage {
 public:
  Cache() {total_hit=0;total_miss=0;total=0;}
  ~Cache() {}

  // Sets & Gets
  //void SetLower(Storage * l){lower_=l;}
  void SetConfig(int size,int setnum, int associat,int writethrough,int writealloc,int access);
  void GetConfig();
  void SetLower(Storage *ll) { lower_ = ll; }
  void SetName(int n) { name=(char)(n+'A'); }
  // Main access process
  void HandleRequest(unsigned long addr, unsigned int bytes, int read,
                     char *content, int &hit, int &cycle);
  void Print(int index);
    int total_hit;
  int total_miss;
  int total;
 private:
  // Bypassing
  int BypassDecision();
  // Partitioning
  void PartitionAlgorithm();
  // Replacement
  int Check(unsigned long vpn);
  void Load(unsigned long vpn,int &hit,int &cycle);
  // Prefetching
  int PrefetchDecision();
  void PrefetchAlgorithm();


  CacheConfig config_;
  Entity entity_;
  Storage *lower_;
  DISALLOW_COPY_AND_ASSIGN(Cache);

  char name;
};

#endif //CACHE_CACHE_H_ 
