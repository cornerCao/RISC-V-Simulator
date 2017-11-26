#include "cache.h"
#include "def.h"
#include "utility.h"
#include "assert.h"
// Main access process
// [in]  addr: access address
// [in]  bytes: target number of bytes
// [in]  read: 0|1 for write|read
//             3|4 for write|read in prefetch
// [i|o] content: in|out data
// [out] hit: 0|1 for miss|hit
// [out] cycle: total access cycle
//#define DEBUG
int fff = 0;
void Cache::HandleRequest(unsigned long addr, unsigned int bytes, int read,
                          char *content,int &hit,int &cycle) {

  // Decide on whether a bypass should take place.
  if (!BypassDecision()) {

    // Generate some infomation for the data partition.
    PartitionAlgorithm();

    // Check whether the required data already exist.
    #ifdef DEBUG
      printf(" # Cache %c Handler # addr = 0x%llx, bytes = %d , read = %d \n",name,addr,bytes,read);
    #endif
    unsigned long first=get_half(addr,entity_.block_size);
    unsigned long last=get_half(addr+bytes-1,entity_.block_size);
    //printf(" first = %llx, last = %llx \n",first,last);
    //assert(FALSE);
    cycle+=config_.AccessTime;
    for(unsigned long i=first;i<=last;++i)
    {
      int index=Check(i);//check tag to find the match line
      
      int first_bytes=get_offset(addr,entity_.block_size);
   //   int last_bytes=entity_.block_size>bytes?bytes:entity_.block_size;
      int last_bytes = bytes+first_bytes>entity_.block_size?entity_.block_size:bytes+first_bytes;
      if (index!=-1) //HIT!
      {
          #ifdef DEBUG
          printf(" # Cache %c HIT # vpn = 0x%llx, read = %d, first byte = %d,last_byte = %d,index=%d\n",name,i,read,first_bytes,last_bytes,index);
          #endif
          if(read==1)
          {
            for(int j=first_bytes;j<last_bytes;++j)
            {
              *content=entity_.content[index].content[j];
              content++;
            }
          }
          else if(read==0)
          {
            for(int j=first_bytes;j<last_bytes;++j)
            {
              entity_.content[index].content[j]=*content;
              content++;
            }
            if(config_.write_through==1)//write through
            {
              lower_->HandleRequest(addr,last_bytes-first_bytes,read,
                &(entity_.content[index].content[first_bytes]),hit,cycle);
              entity_.content[index].dirty=FALSE;
            }
            else if(config_.write_through==0)//write back
            {
              entity_.content[index].dirty=TRUE;
            }
          }
          ++total_hit;
          bytes-=last_bytes-first_bytes;
          addr+=last_bytes-first_bytes;
          //flags的更新
            unsigned int set=get_set(i,config_.set_num);
            unsigned int k = set*config_.associativity;
            for(;k<(set+1)*config_.associativity;++k)//更新该set内的flag
            {
              entity_.content[k].flags++;
            }
            entity_.content[index].flags=0;
          continue;
      }
      #ifdef DEBUG
        printf(" # Cache %c MISS # vpn = 0x%llx, read = %d \n",name,i,read);
      #endif
      total_miss++;
      
      if(read==0&&config_.write_allocate==0)
      {
        lower_->HandleRequest(addr,last_bytes-first_bytes,read,content,hit,cycle);
        bytes-=last_bytes-first_bytes;
        addr+=last_bytes-first_bytes;
        content+=last_bytes-first_bytes;
        continue;
      }
      Load(i,hit,cycle);
      // Choose a victim for the current request.
      //i is vpn
      //只负责将条目更新，然后让这个查询再来一次。Load里面对响应的miss以及底层的hit和cycle做了更新。
      //cycle和hit的操作
      cycle-=config_.AccessTime;
      --i;
      --total_hit;//因为在上面的操作重新查询的时候之前的miss一定会成功，会让它hit多加一次。
      //这里可能还要涉及cycle的更改，这个具体的我也不清楚。看你怎么计算cycle的。
    }
    
    // Decide on whether a prefetch should take place.
    if (PrefetchDecision()) {
      // Fetch the other data via HandleRequest() recursively.
      // To distinguish from the normal requests,
      // the 2|3 is employed for prefetched write|read data
      // while the 0|1 for normal ones.
      PrefetchAlgorithm();
    }
  }
  // Fetch from the lower layer

  // If a victim is selected, replace it.
  // Something else should be done here
  // according to your replacement algorithm.
}

int Cache::BypassDecision() {
  return FALSE;
}

void Cache::PartitionAlgorithm() {
}

int Cache::Check(unsigned long vpn) {
  #ifdef DEBUG
    printf("# Cache %c Checking # \n",name);
  #endif

  unsigned int set=get_set(vpn,config_.set_num);
  unsigned int index=set*config_.associativity;
  for(;index<(set+1)*config_.associativity;++index)
  {
    if(entity_.content[index].valid&&equal(entity_.content[index].tag,vpn))
      {
        return index;
      }
  }
  Print(set);
  return -1;
}
//--------------------------------------------------------
// Load:
// Load便是根据所需要调的block：vpn进行该层cache的block的更新。
// 只有在miss的情况下，并且不是write_non_allocate的策略下才会进行cache的更新
// 更新的时候分三步：
//    1.从底层向上调取vpn的block
//    2.从该层选择一个block位置，作为更新位置
//          a）如果该位置原本的条目是有效的，并且dirty（在write_through中该条目无效，不过为了防止出错，也对dirty位进行了FALSE的更新，因为直写是不会有dirrty的block的）
//          b）如果该位置无效，则直接替换即可
//    3.将新的block放置，注意要其dirty位设为FALSE
//  可能相应的时间什么的你搞定了
//--------------------------------------------------------
void Cache::Load(unsigned long vpn,int &hit,int &cycle){
  #ifdef DEBUG
    printf("# Cache %c Loading # vpn = %llx \n",name,vpn);
  #endif
  //unsigned long tag=get_tag(vpn,config_.set_num);
  unsigned int set=get_set(vpn,config_.set_num);
  unsigned int index=set*config_.associativity;//set 号 * associaty 得到index
  char *tmp_block=new char[entity_.block_size];//先把底层的load上来
  lower_->HandleRequest(get_initial(vpn,entity_.block_size),entity_.block_size,1,tmp_block,hit,cycle);
  //get initial is get the init addr of vpn

  int max=-1;

  int replace=-1;
//LRU strategy
  for(;index<(set+1)*config_.associativity;++index)
  {
    if(!entity_.content[index].valid)// empty block
    {
      replace=index;
      break;
    }
    if(entity_.content[index].flags>max)//LRU replace
    {
      replace=index;max=entity_.content[index].flags;
    }
  }
  if(entity_.content[replace].valid&&entity_.content[replace].dirty)
  {
    int tmp_hit,tmp_cycle;
    lower_->HandleRequest(get_initial(entity_.content[replace].tag,entity_.block_size),entity_.block_size,0,entity_.content[replace].content,tmp_hit,tmp_cycle);
    //这里可能hit，cycle什么的要传一个新的进去，一个无用的进去，不然会改变总hit会cycle如果觉得这个替换不属于整体的消耗的话。
  }

  entity_.content[replace].valid=TRUE;
  entity_.content[replace].dirty=FALSE;
  entity_.content[replace].tag=vpn;
  #ifdef DEBUG
    printf("$$ Load Replace %c $$ set = %d, id = %d, tag = %llx\n",name,set, replace,entity_.content[replace].tag);
  #endif
    //printf("block content is ");
  for(int i=0;i<entity_.block_size;++i)
  {
    entity_.content[replace].content[i]=tmp_block[i];
  //  printf("%d ",tmp_block[i]);
  }
//  printf("\n");
  //entity_.content[replace].flags=$$$$$;看具体策略
  //hit和cycle的增加，这里看你的策略了。
}

int Cache::PrefetchDecision() {
  return FALSE;
}

void Cache::PrefetchAlgorithm() {
}

void Cache::SetConfig(int size,int blocksize,int associat,int writethrough,int writealloc,int access)
{
 entity_.block_size = blocksize;//初始化cache的每个line
 entity_.block_num = size / blocksize;
 entity_.set_num = entity_.block_num/associat;
 entity_.associativity = associat;
   entity_.content=new block[entity_.block_num];

   config_.size = size;
   config_.associativity = associat;
   config_.set_num = entity_.set_num;
   config_.write_through = writethrough;
   config_.write_allocate = writealloc;
   config_.AccessTime = access;
   config_.block_size = entity_.block_size;

  for(int i=0;i<entity_.block_num;++i)
  {
    //entity_.content[i].block_size=entity_.block_size;
    entity_.content[i].content=new char[entity_.block_size];
    entity_.content[i].valid=FALSE;
  }
  /*
  config_=cc;
  entity_.set_num=config_.set_num;
  entity_.associativity=config_.associativity;
  entity_.block_num=config_.set_num*config_.associativity;
  entity_.block_size=config_.size/(entity_.block_num);
  entity_.content=new block[entity_.block_num];
  for(int i=0;i<entity_.block_num;++i)
  {
    //entity_.content[i].block_size=entity_.block_size;
    entity_.content[i].content=new char[entity_.block_size];
    entity_.content[i].valid=FALSE;
  }*/
}
void Cache::GetConfig()
{
}
void Cache::Print(int s)
{
  #ifdef DEBUG
  printf("# Cache %c Content #\n",name);
  //for(int index=s*entity_.associativity;index<(s+1)*entity_.associativity;++index)
  for(int index=0;index<entity_.block_num;++index)
  {
    if(entity_.content[index].valid)
      {
        printf("# Cache[%d] # tag = %llx , flags = %d , dirty = %d\n",index,entity_.content[index].tag,entity_.content[index].flags,entity_.content[index].dirty);
      }
  }
  printf("# End Printing %c #\n",name);
  #endif
}
