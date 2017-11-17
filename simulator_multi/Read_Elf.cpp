#include"Read_Elf.h"

FILE *elf=NULL;
Elf64_Ehdr *elf64_hdr=NULL;

unsigned int cadr=0;

//代码段的长度
unsigned int csize=0;

//代码段在内存中的虚拟地址
unsigned int vadr=0;

//全局数据段在内存的地址
unsigned long long gp=0;

//main函数在内存中地址
unsigned int madr=0;
unsigned int msize = 0;

//程序结束时的PC
unsigned int endPC=0;

//程序的入口地址
unsigned int entry=0;

FILE *file=NULL;

unsigned int padr=0;
unsigned int psize=0;
unsigned int pnum=0;

//Section Headers
unsigned int sadr=0;
unsigned int ssize=0;
unsigned int snum=0;

//Symbol table
unsigned int symnum=0;
unsigned int symadr=0;
unsigned int symsize=0;

//ÓÃÓÚÖ¸Ê¾ °üº¬½ÚÃû³ÆµÄ×Ö·û´®ÊÇµÚ¼¸¸ö½Ú£¨´ÓÁã¿ªÊ¼¼ÆÊý£©
//unsigned int index=0;

//×Ö·û´®±íÔÚÎÄ¼þÖÐµØÖ·£¬ÆäÄÚÈÝ°üÀ¨.symtabºÍ.debug½ÚÖÐµÄ·ûºÅ±í
unsigned int stradr=0;
unsigned int shstrndx = 0;
unsigned int symstradr = 0;

//Program headers

bool open_file()
{	
	elf = fopen("elfinfo.txt","w");
	if(!elf)
		return false;
	return true;
}

void read_elf()
{
	if(file==NULL||!open_file())
		return ;

	fprintf(elf,"ELF Header:\n");
	read_Elf_header();

	fprintf(elf,"\n\nSection Headers:\n");
	read_elf_sections();

	fprintf(elf,"\n\nProgram Headers:\n");
	read_Phdr();

	fprintf(elf,"\n\nSymbol table:\n");
	read_symtable();

	fclose(elf);
}

void read_Elf_header(){
	elf64_hdr = new Elf64_Ehdr;
	//file should be relocated
	fread(elf64_hdr,1,sizeof(*elf64_hdr),file);

	pnum = elf64_hdr->e_phnum;
	psize = elf64_hdr->e_phentsize;
	padr = elf64_hdr->e_phoff;

	snum = elf64_hdr->e_shnum;
	ssize = elf64_hdr->e_shentsize;
	sadr = elf64_hdr->e_shoff;

	entry = (unsigned int)elf64_hdr->e_entry;
	shstrndx = elf64_hdr->e_shstrndx;

	fprintf(elf," magic number:  ");
	for(int i = 0;i<16;i++){
		fprintf(elf, "%2x ", elf64_hdr->e_ident[i]);
	}
	fprintf(elf,"\n");

	switch(elf64_hdr->e_ident[4]){
		case 0:
			fprintf(elf," Class:  ILLEGAL ELFCLASS\n");
			break;
		case 1:
			fprintf(elf," Class:  ELFCLASS32\n");
			break;
		case 2:
			fprintf(elf," Class:  ELFCLASS64\n");
			break;
		default: break;
	}
	
	switch(elf64_hdr->e_ident[5]){
		case 0:
			fprintf(elf," Data:  ILLEGAL\n");
			break;
		case 1:
			fprintf(elf," Data:  little-endian\n");
			break;
		case 2:
			fprintf(elf," Data:  big-endian\n");
			break;
		default: break;
	}
		
	fprintf(elf," Version:  %d current\n",elf64_hdr->e_ident[EI_VERSION]);

	fprintf(elf," OS/ABI:	 System V ABI\n");
	
	fprintf(elf," ABI Version:  %d \n",elf64_hdr->e_ident[EI_OSABI]);
	
	fprintf(elf," Type:  %x",elf64_hdr->e_type);

	fprintf(elf," Machine:  %x \n",elf64_hdr->e_machine);

	fprintf(elf," Version:  %x\n",elf64_hdr->e_version);
	
	fprintf(elf," Entry point address:  0x%x\n",elf64_hdr->e_entry);

	fprintf(elf," Start of program headers:  %d  bytes into  file\n",elf64_hdr->e_phoff);

	fprintf(elf," Start of section headers:  %d  bytes into  file\n",elf64_hdr->e_shoff);

	fprintf(elf," Flags:  0x%x\n",elf64_hdr->e_flags);

	fprintf(elf," Size of this header:  %d Bytes\n",elf64_hdr->e_ehsize);

	fprintf(elf," Size of program headers:  %d Bytes\n",elf64_hdr->e_phentsize);

	fprintf(elf," Number of program headers:  %d \n",elf64_hdr->e_phnum);

	fprintf(elf," Size of section headers:  %d  Bytes\n",elf64_hdr->e_shentsize);

	fprintf(elf," Number of section headers: %d \n",elf64_hdr->e_shnum);

	fprintf(elf," Section header string table index:  %d \n",elf64_hdr->e_shstrndx);
}
	
void read_elf_sections()
{

	Elf64_Shdr *elf64_shdr = new Elf64_Shdr;
	stradr = sadr + shstrndx*ssize;//利用str index 定位出str的地址

	fseek(file,stradr,SEEK_SET);
	fread(elf64_shdr,1,sizeof(*elf64_shdr),file);
	stradr = elf64_shdr->sh_offset;

	for(int c=0;c<snum;c++)
	{
		fprintf(elf," [%3d]\n",c);

		fseek(file, sadr + c*ssize, SEEK_SET);
		//file should be relocated
		fread(elf64_shdr,1,sizeof(*elf64_shdr),file);

		//read section name
		fseek(file, elf64_shdr->sh_name + stradr, SEEK_SET);
		char section_name[20];
		char tmp;
		int i=0;
		while(1)
		{
			fread(&tmp,1,1,file);
			if(tmp!='\0') section_name[i++]=tmp;
			else break;
		}
		section_name[i]='\0';

		fprintf(elf," Name: %s",section_name);
		//代码段
		if(strcmp(section_name,".text")==0)
		{
			cadr = elf64_shdr->sh_offset;
			csize = elf64_shdr->sh_size;
			vadr = elf64_shdr->sh_addr;
		}
		else if(strcmp(section_name,".symtab")==0)
		{
			symadr = elf64_shdr->sh_offset;
			symsize = elf64_shdr->sh_size;
			int symensize = elf64_shdr->sh_entsize;
			symnum=symsize/symensize;
			symsize = symensize;
			//printf("Symbol table found:\nsymadr=%x,symsize=%ld,symnum=%d\n",symadr,symsize,symnum);
		}
		else if(strcmp(section_name,".strtab")==0)
		{
			symstradr = elf64_shdr->sh_offset;

		}


		fprintf(elf," Type: %x",elf64_shdr->sh_type);

		fprintf(elf," Address:  %x",elf64_shdr->sh_addr);

		fprintf(elf," Offest:  %d\n",elf64_shdr->sh_offset);

		fprintf(elf," Size:  %d",elf64_shdr->sh_size);

		fprintf(elf," Entsize:  %d",elf64_shdr->sh_entsize);

		fprintf(elf," Flags:  %d ",elf64_shdr->sh_flags);
		
		fprintf(elf," Link: %d ",elf64_shdr->sh_link);

		fprintf(elf," Info: %d ",elf64_shdr->sh_info);

		fprintf(elf," Align: %d\n",elf64_shdr->sh_addralign);

 	}
 	delete elf64_shdr;
}

void read_Phdr()
{
	Elf64_Phdr *elf64_phdr=new Elf64_Phdr;

	for(int c=0;c<pnum;c++)
	{
		fprintf(elf," [%3d]\n",c);
		fseek(file, padr + c*psize, SEEK_SET);
			
		//file should be relocated
		fread(elf64_phdr,1,sizeof(*elf64_phdr),file);

		fprintf(elf," Type:  %d ",elf64_phdr->p_type);
		
		fprintf(elf," Flags:  %d ",elf64_phdr->p_flags);
		
		fprintf(elf," Offset:  %d ",elf64_phdr->p_offset);
		
		fprintf(elf," VirtAddr: %x ",elf64_phdr->p_vaddr);
		
		fprintf(elf," PhysAddr:  %x ",elf64_phdr->p_paddr);

		fprintf(elf," FileSiz:  %d ",elf64_phdr->p_filesz);

		fprintf(elf," MemSiz:  %d ",elf64_phdr->p_memsz);
		
		fprintf(elf," Align:  %d \n",elf64_phdr->p_align);
	}
	delete elf64_phdr;
}


void read_symtable()
{
	Elf64_Sym* elf64_sym = new Elf64_Sym;
	for(int c=0;c<symnum;c++)
	{
		fprintf(elf," [%3d]   ",c);
		fseek(file, symadr + c*symsize, SEEK_SET);
			
		//file should be relocated
		
		//file should be relocated
		fread(elf64_sym,1,sizeof(*elf64_sym),file);

		//read section name
		fseek(file, elf64_sym->st_name + symstradr, SEEK_SET);
		
		char symbol_name[40];
		char tmp;
		int i=0;
		while(1)
		{
			fread(&tmp,1,1,file);
			if(tmp!='\0') symbol_name[i++]=tmp;
			else break;
		}
		symbol_name[i]='\0';

		if(strcmp(symbol_name,"main")==0)
			{madr = elf64_sym->st_value;
			msize = elf64_sym->st_size;}
		else if(strcmp(symbol_name,"__global_pointer$")==0)
			gp = elf64_sym->st_value;


		fprintf(elf," Name:  %40s   ",symbol_name);

		fprintf(elf," Bind:  %d ",elf64_sym->st_info);
		
		fprintf(elf," Type:  %d ",elf64_sym->st_other);
		
		fprintf(elf," NDX:   %d",elf64_sym->st_shndx);
		
		fprintf(elf," Size:  %d ",elf64_sym->st_size);

		fprintf(elf," Value:  %d \n",elf64_sym->st_value);

	}
	delete elf64_sym;
}


