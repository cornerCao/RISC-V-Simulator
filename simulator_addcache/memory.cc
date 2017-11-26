#include "memory.h"

void Memory::HandleRequest(unsigned long addr, unsigned int bytes, int read,
                          char *content, int &hit, int &cycle) {
	//printf("# Memory Handler # addr = %llx #bytes = %d # read = %d\n",addr,bytes,read);
	cycle+=AccessTime;
	for(int i=addr;i<addr+bytes;++i,++content)
	{
		if(read)
			*content=mainMemory[i];
			//*content=0;
		else{
			mainMemory[i]=*content;
		//	printf("content is %c\n",mainMemory[i]);
		}
	}
}

