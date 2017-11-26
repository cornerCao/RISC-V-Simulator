#include "Simulation.h"
using namespace std;

extern void read_elf();
extern unsigned int cadr;
extern unsigned int csize;
extern unsigned int vadr;
extern unsigned long long gp;
extern unsigned int madr;
extern unsigned int endPC;
extern unsigned int entry;
extern FILE *file;
extern unsigned int sadr;
extern unsigned int ssize;
extern unsigned int snum;
extern unsigned int msize;
Cache L[6];
Memory * MainMem;
//指令运行数
long long inst_num=0;

//系统调用退出指示
int exit_flag=0;
int clk = 0;//时钟数
int instcnt = 0;//指令数

int bubble = 0;//在开头放的气泡数，即指令停止数
int halt = 0;//暂停次数

int control_haz_cnt = 0;
int data_haz_cnt = 0;

int EXErd = -1;
int MEMrd = -1;
int WBrd = -1;

int oldPC = 0;

void printinst(int pc,int ALUop,int rs1,int rs2,int rd,long long imm){
	printf("CLK=%d PC=%x ",clk,pc-4);
	switch(ALUop){
		case ALU_ADD:
			printf("add ");
			break;
		case ALU_SUB:
			printf("sub " );
			break;
		case ALU_MULL:
			printf("mull ");
			break;
		case ALU_DIV:
			printf("div ");
			break;
		case ALU_SLL:
			printf("sll ");
			break;
		case ALU_SRL:
			printf("srl ");
			break;
		case ALU_AND:
			printf("and ");
			break;
		case ALU_OR:
			printf("or ");
			break;
		case ALU_XOR:
			printf("xor ");
			break;
		case ALU_SLT:
			printf("slt ");
			break;
		case ALU_SRA:
			printf("sra ");
			break;
		case ALU_REM:
			printf("rem ");
			break;
		case ALU_MULH:
			printf("mulh ");
			break;
		case ALU_LB:
			printf("lb ");
			break;
		case ALU_LH:
			printf("lh ");
		  	break;
		case ALU_LW:
		  	printf("lw ");
		  	break;
		case ALU_LD:
			printf("ld ");
			break;
		case ALU_ADDI:
			printf("addi ");
			break;
		case ALU_SLLI:
			printf("slli ");
			break;
		case ALU_SLTI:
			printf("slti ");
			break;
		case ALU_XORI:
			printf("xori ");
			break;
		case ALU_SRLI:
			printf("srli ");
			break;
		case ALU_SRAI:
			printf("srai ");
			break;
		case ALU_ORI:
			printf("ori ");
			break;
		case ALU_ANDI:
			printf("andi ");
			break;
		case ALU_ADDIW:
			printf("addiw ");
			break;
		case ALU_JALR:
			printf("jalr ");
			break;
		case ALU_SB:
			printf("sb ");
			break;
		case ALU_SH:
			printf("sh ");
			break;
		case ALU_SW:
			printf("sw ");
			break;
		case ALU_SD:
			printf("sd ");
			break;
		case ALU_BEQ:
			printf("beq ");
			break;
		case ALU_BNE:
			printf("bne ");
			break;
		case ALU_BLT:
			printf("blt ");
			break;
		case ALU_BGE:
			printf("bge ");
			break;
		case ALU_AUIPC:
			printf("auipc ");
			break;
		case ALU_LUI:
			printf("lui ");
			break;
		case ALU_JAL:
			printf("jal ");
			break;
		case ALU_ADDW:
			printf("addw ");
			break;
		case ALU_MULW:
			printf("mulw ");
			break;
		case ALU_DIVW:
			printf("divw ");
			break;
		case ALU_SRAIW:
			printf("sraiw ");
			break;
		case ALU_SLLIW:
			printf("slliw ");
			break;
		case ALU_SRLIW:
			printf("srliw ");
			break;
		default:
			printf("error alu ");
	}
	printf("rs1=%d rs2=%d rd=%d imm=%lld\n",rs1,rs2,rd,imm);
}

//高位乘法
unsigned long long Multi_H(unsigned long long m,unsigned long long n){
	int a[100],b[100],c[200],len,la,lb,i,j;   
	la=0;  
	while(n>0)  {  
		la++;  
		a[la]=n%16;  
		n=n/16;  
	}  
	lb=0;  
	while(m>0)  {  
		lb++;  
		b[lb]=m%16;  
		m=m/16;  
	}  
	memset(c,0,sizeof(c));  
	for(i=1;i<=la;i++)  
		for(j=1;j<=lb;j++)  
		c[i+j-1]=c[i+j-1]+a[i]*b[j];  
	len=la+lb;  
	for(i=1;i<=len;i++)  {  
		c[i+1]=c[i+1]+c[i]/16;  
		c[i]=c[i]%16;  
	}  
	while(c[len]==0){len--;}  
	m=c[len];  
	while(m>0) {  
		c[len]=m%16;  
		m=m/16;  
		len++;  
	}  
	unsigned long long res = 0;
	for(int i = 32;i>=17;i--){
	res += ((unsigned long long)c[i]<<((i-17)*4));	
	}
	return res;
}
void visit_memo(int addr,void*target,int read,int bytes,int &cycle){//用于访存
	int hit = 0;
//	printf("visit memory addr = %llx,read = %d, bytes = %d",addr,read,bytes);
	if(read==1)
	{
	char buff[8];
	L[1].HandleRequest(addr,bytes,read,&buff[0],hit,clk);
		memcpy(target,&buff[0],bytes);
		//printf("read memory, content is %llx\n",*(int *)target);
	}
	else{
		L[1].HandleRequest(addr,bytes,read,(char*)target,hit,clk);
	}
}
//加载代码段
//初始化PC
void load_memory()
{//把所有section加载入内存
	/*
	fseek(file,cadr,SEEK_SET);
	//因为一行为32位,4 byte 
	fread(&memory[vadr>>2],1,csize,file);

	vadr=vadr>>2;
	csize=csize>>2;
	fclose(file);*/
	Elf64_Shdr *elf64_shdr= new Elf64_Shdr;

	for(int c=0;c<snum;c++)
	{
		fseek(file, sadr + c*ssize, SEEK_SET);

		//file should be relocated
		fread(elf64_shdr,1,sizeof(*elf64_shdr),file);
		int virtual_adr = elf64_shdr->sh_addr;
		fseek(file, elf64_shdr->sh_offset, SEEK_SET);
	//	fread(&memory[virtual_adr],1,elf64_shdr->sh_size,file);
		char tmpchar;
		int tmpclk = 0;
		int tmphit = 0;
		for(int i = 0;i<elf64_shdr->sh_size;i++){
	//	fread(&memory[virtual_adr],1,elf64_shdr->sh_size,file);
			fread(&tmpchar,1,1,file);
			MainMem->HandleRequest(virtual_adr+i,1,0,&tmpchar,tmphit,tmpclk);
	}
		//fread(&tmpchar,1,elf64_shdr->sh_size,file);
		//printf("visit virt addr %llx\n",virtual_adr);
		//visit_memo(virtual_adr,&tmpchar,0,1,tmpclk);
		//printf("%x\n", *(instruction*)p_seg_actual_addr);

	}

	//PC = madr;
}
void help()
{
	printf("This is a simulator to execute riscv ELF!\n\n");
	printf("Usage: ./exeute filename\n\n");
	//printf("Hope you will love it.\n");

}
/*void print_mem(){
	int i = 0;
	for(i  = 0;i<MAX;i+=4){
		unsigned int mem = 0;
		memcpy(&mem,&memory[i],4);
		if(mem != 0){
			printf("memory %8x : %8x\n",i,mem);
		}
	}
}*/
FILE*pcfile = NULL;
void init_cache(){//initial cache
	MainMem=new Memory(20);
	Storage* p=MainMem;
	L[3].SetLower(p);
	L[3].SetConfig(8192*1024,64,8,0,0,20);
	L[3].SetName(3);
	p=&L[3];

	L[2].SetLower(p);
	L[2].SetConfig(256*1024,64,8,0,0,8);
	L[2].SetName(2);
	p=&L[2];

	L[1].SetLower(p);
	L[1].SetConfig(32*1024,64,8,0,0,1);
	L[1].SetName(1);

}
int main(int argc, char *argv[])
{
	//参数不够则返回
	if(argc < 2 || strcmp(argv[1],"help") == 0)
	{
		help();
		return 0;
	}
//	pcfile = fopen("pc.txt","w");
	memset(reg,0,sizeof(reg));
//	memset(memory,0,sizeof(memory));
		char *file_name = argv[1];
		file = fopen(file_name, "rb");  // binary mode
		
		if(file == NULL)
		{
			printf("Can not open file : %s successfully.\n", file_name);
			exit(1);
		}

		printf("executing file : %s ...\n", file_name);

		//解析elf文件
		read_elf();
		
		init_cache();

	//加载内存
		load_memory();

	//设置入口地址
		PC=madr;
		endPC = madr + msize -3;//madr + msize -3
	//设置全局数据段地址寄存器
		reg[3]=gp;
	
		reg[2]=MAX/2;//栈基址 （sp寄存器）
		#ifdef DEBUG
		printf("entry=%d endPC=%d gp=%d madr=%d msize=%d\n",entry,endPC,gp,madr,msize);
		#endif


	simulate();
	#ifdef DEBUG_MEM
//	print_mem();
	#endif
	cout <<"simulate over!"<<endl;
	cout<<"clk="<<clk<<" instcnt="<<instcnt<<" CPI="<<float(clk)/instcnt<<endl;
	cout<<"control hazard clk cnt = "<<control_haz_cnt<<", data dazard clk cnt = "<<data_haz_cnt<<endl;
	long long wholecnt = L[1].total_miss + L[1].total_hit;
//	printf("%d,%llf,%f\n",whole,float(L[1].total_miss)/whole,100.1*9968/10000.0);
	printf("total miss = %d, total hit = %d, miss rate = %%%f, hit rate = %%%f\n",L[1].total_miss,L[1].total_hit,100*float(L[1].total_miss)/wholecnt,100*float(L[1].total_hit)/wholecnt);
	return 0;
}

void simulate()
{
	//结束PC的设置
	int end=endPC;
	clk = 0;
	instcnt = 0;

	IF_ID.Nop = 0;
	ID_EX.Nop = 0;//0表示是空指令
	EX_MEM.Nop = 0;
	MEM_WB.Nop = 0;

	bubble = 0;
	halt = 0;

	while(PC!=end)
	{
		//运行
		clk++;
		#ifdef DEBUG_PIPE
		printf("\nCLK=%d\n",clk);
		#endif
		IF();
		ID();
		if(bubble>0){//插入气泡
			ID_EX_old.Nop=0;
			bubble--;
			EXErd = -1;
		}
		EX();
		MEM();
		WB();
		//更新中间寄存器
		if(halt==0)
		{
			IF_ID=IF_ID_old;
		}
			ID_EX=ID_EX_old;
			EX_MEM=EX_MEM_old;
			MEM_WB=MEM_WB_old;

        if(exit_flag==1)
            break;

        reg[0]=0;//一直为零
	}
}


//取指令
void IF()
{
	//write IF_ID_old
	if(halt==0)
	{
		//memcpy(&IF_ID_old.inst,&memory[PC],4);L[1].HandleRequest(addr,1,1,&c,hit,cycle);
		visit_memo(PC,&IF_ID_old.inst,1,4,clk);
		oldPC = PC;
		PC=PC+4;
		IF_ID_old.PC=PC;
		IF_ID_old.Nop = 1;
		instcnt += 1;
		IF_ID_old.Nop = 1;
	}
	else{
	//	memcpy(&IF_ID_old.inst,&memory[oldPC],4);
		visit_memo(oldPC,&IF_ID_old.inst,1,4,clk);
		IF_ID_old.Nop = 1;
		halt--;
	}
	#ifdef DEBUG_PIPE
	printf("IF inst %d, %llx\n",instcnt,IF_ID_old.inst);
	#endif
	IF_ID_old.instnum = instcnt;
}

//译码
void ID()
{
	if(IF_ID.Nop==0){
		ID_EX_old.Nop = 0;
		return;
	}
	//Read IF_ID
	unsigned int inst=IF_ID.inst;
	int EXTop=0;
	unsigned int EXTsrc=0;
	#ifdef DEBUG_PIPE
	printf("ID %d\n",IF_ID.instnum);
	#endif
	int RegDst,ALUop,ALUSrc1,ALUSrc2;
	int Branch,MemRead,MemWrite;
	int RegWrite,MemtoReg;

	rd= -4;
	rs1 = -3;
	rs2 = -2;
	fuc3= 0;
	fuc7 = 0;
	OP = getbit(inst,0,6);
	imm = 0;

	Branch=0;
		MemRead=0;//是否需要读mem
		MemWrite=0;//是否需要写mem
		RegWrite=0;//是否要写寄存器
		MemtoReg=0;//是否要从内存写到寄存器
		EXTop=0;//是否要符号扩展
		EXTsrc = 0;//符号扩展src的长度
		RegDst=rd;
		ALUSrc1=rs1;
		ALUSrc2 = rs2;	

	if(OP==OP_R)
	{
		rd=getbit(inst,7,11);
	rs1 = getbit(inst,15,19);
	rs2 = getbit(inst,20,24);
	fuc3=getbit(inst,12,14);
	fuc7 = getbit(inst,25,31);
		RegWrite = 1;//要写寄存器
		if(fuc3==0&&fuc7==0)
		{
			ALUop=ALU_ADD;
		}
		else if(fuc3==0&&fuc7 == 1)
		{
		    ALUop = ALU_MULL;
		}
		else if(fuc3==0 && fuc7 == 0x20){
			ALUop = ALU_SUB;
		}
		else if (fuc3==1&&fuc7==0){
			ALUop = ALU_SLL;
		}
		else if (fuc3==1&&fuc7==1){
			ALUop = ALU_MULH;
		}
		else if (fuc3==2&&fuc7==0){
			ALUop = ALU_SLT;
		}
		else if (fuc3==4&&fuc7==0){
			ALUop = ALU_XOR;
		}
		else if (fuc3==4&&fuc7==1){
			ALUop = ALU_DIV;
		}
		else if (fuc3 == 5&&fuc7==0){
			ALUop = ALU_SRL;
		}
		else if (fuc3==5&&fuc7 == 0x20){
			ALUop = ALU_SRA;
		}
		else if (fuc3==6&&fuc7==0){
			ALUop = ALU_OR;
		}
		else if (fuc3==6&&fuc7==1){
			ALUop = ALU_REM;
		}
		else if (fuc3==7&&fuc7==0){
			ALUop = ALU_AND;
		}
	}
	else if(OP==OP_I1||OP==OP_I2||OP==OP_I3||OP==OP_I4||OP==OP_I5)
    {
		rs1 = getbit(inst,15,19);
		fuc3 = getbit(inst,12,14);
		rd = getbit(inst,7,11);
		imm = getbit(inst,20,31);
		if(OP==OP_I1){
			MemtoReg = 1;
			//RegWrite = 1;
			EXTop = 1;
            MemRead = 1;
            EXTsrc = 12;
    	//lb 8bit
      	  if(fuc3==0)
     	   {
   	 	        ALUop = ALU_LB;
    	    }
    	    else if (fuc3==1)
    	    {//lh 16bit
    	       ALUop = ALU_LH;
   		     }
  	 	     else if (fuc3 == 2){
        	//lw 32bit
 	          ALUop = ALU_LW;
  		      }
   		     else if (fuc3 == 3){
        	//ld 64bit
  	         ALUop = ALU_LD;
 	       }
    	}
    	else if (OP==OP_I2){
    		RegWrite = 1;
    		if(fuc3==0){
    			//addi
    			ALUop = ALU_ADDI;
    			EXTop = 1;
    			EXTsrc = 12;
    		}
    		else if(fuc3==1&&fuc7==0){
    			ALUop = ALU_SLLI;
    			EXTop = 1;
    			EXTsrc = 12;
    		}
    		else if(fuc3 == 2){
    			ALUop = ALU_SLTI;
    		}
    		else if (fuc3==4){
    			ALUop = ALU_XORI;
    		}
    		else if (fuc3 == 5 && fuc7 == 0){
    			ALUop = ALU_SRLI;
    		}
    		else if (fuc3 == 5&&fuc7 == 0x20){
    			ALUop = ALU_SRAI;
    		}
    		else if (fuc3 == 6 ){
    			ALUop = ALU_ORI;
    		}
    		else if (fuc3==7){
    			ALUop = ALU_ANDI;
    		}
    	}
    	else if (OP == OP_I3){
    		RegWrite = 1;
    		if(fuc3==0){
    			EXTop = 1;
    			EXTsrc = 12;
    			ALUop = ALU_ADDIW;
    		}
    		else if(fuc3 == 1){
    			ALUop = ALU_SLLIW;
    			EXTop = 1;
    			EXTsrc = 12;
    		}
    		else if(fuc3 == 5&&fuc7==0){
    			ALUop = ALU_SRLIW;
    			EXTop = 1;
    			EXTsrc = 12;
    		}
    		else if(fuc3 == 5 && fuc7==32){
    			ALUop = ALU_SRAIW;
    			EXTop = 1;
    			EXTsrc = 12;
    		}
    	}
    	else if (OP == OP_I4&&fuc3==0){
    		RegWrite = 1;
    		EXTop = 1;
    		EXTsrc = 12;
    		ALUop = ALU_JALR;
    		imm = ((imm>>1)<<1);
    	}
    	else if(OP==OP_I5 && fuc3 == 0 && fuc7 == 0){
    		//ecall
    	//	exit_flag = 1;
    	}
    }
    else if(OP==OP_S)
    {
    	rs1 = getbit(inst,15,19);
    	rs2 = getbit(inst,20,24);
    	fuc3 = getbit(inst,12,14);
    	imm = (getbit(inst,25,31)<<5)+(getbit(inst,7,11));
    	EXTop = 1;
    	EXTsrc  = 12;
    	MemWrite = 1;
        if(fuc3==0)
        {
           ALUop = ALU_SB;
        }
        else if (fuc3 == 1)
        {
           ALUop = ALU_SH;
        }
        else if(fuc3 == 2){
        	ALUop = ALU_SW;
        }
        else if(fuc3==3){
        	ALUop = ALU_SD;
        }
    }
    else if(OP==OP_SB)
    {
    	Branch = 1;
    	imm = (getbit(inst,8,11)<<1) + (getbit(inst,25,30)<<5) + (getbit(inst,7,7)<<11) + (getbit(inst,31,31)<<12);
    	rs2 = getbit(inst,20,24);
    	rs1 = getbit(inst,15,19);
    	fuc3 = getbit(inst,12,14);
    	EXTop = 1;
    	EXTsrc = 13;
        if(fuc3==0)
        {
			ALUop = ALU_BEQ;
        }
        else if(fuc3==1)
        {
        	ALUop = ALU_BNE;
        }
        else if(fuc3==4){
        	ALUop = ALU_BLT;
        }
        else if(fuc3==5){
        	ALUop = ALU_BGE;
        }
    }
    else if(OP == OP_U1){
    	rd = getbit(inst,7,11);
    	imm = (getbit(inst,12,31)<<12);
    	RegWrite = 1;
    	ALUop = ALU_AUIPC;
    }
    else if(OP == OP_U2){
    	rd = getbit(inst,7,11);
    	imm = (getbit(inst,12,31)<<12);
    	RegWrite = 1;
    	ALUop = ALU_LUI;
    }
    else if(OP==OP_UJ)
    {
    	imm = (getbit(inst,21,30)<<1)+(getbit(inst,20,20)<<11) + (getbit(inst,12,19)<<12) + (getbit(inst,31,31)<<20);
    	rd = getbit(inst,7,11);
    	EXTop = 1;
    	EXTsrc = 21;
    	RegWrite = 1;
    	ALUop = ALU_JAL;
     }
     else if(OP==OP_E){
     	rd=getbit(inst,7,11);
	rs1 = getbit(inst,15,19);
	rs2 = getbit(inst,20,24);
	fuc3=getbit(inst,12,14);
	fuc7 = getbit(inst,25,31);
	if(fuc7==0&&fuc3==0){
		RegWrite = 1;//要写寄存器
		ALUop = ALU_ADDW;
     	}
     else if(fuc7==1 &&fuc3==0){
     	RegWrite = 1;
     	ALUop = ALU_MULW;
     }
     else if(fuc7==1 &&fuc3==4){
     	RegWrite = 1;
     	ALUop = ALU_DIVW;
     }	
 	}

 	
	//write ID_EX_old
	ID_EX_old.Rd=rd;
	ID_EX_old.Rs1=rs1;
	ID_EX_old.Rs2 = rs2;
	ID_EX_old.Imm=ext_signed(EXTop,imm,EXTsrc);
	//ID_EX_old.Ctrl_EX_ALUSrc1 = reg[rs1];
//	ID_EX_old.Ctrl_EX_ALUSrc2 = reg[rs2];
	ID_EX_old.Ctrl_EX_ALUOp=ALUop;
	ID_EX_old.Ctrl_EX_RegDst = rd;
	ID_EX_old.Ctrl_M_Branch = Branch;
	ID_EX_old.Ctrl_M_MemWrite = MemWrite;
	ID_EX_old.Ctrl_M_MemRead = MemRead;
	ID_EX_old.Ctrl_WB_RegWrite = RegWrite;
	ID_EX_old.Ctrl_WB_MemtoReg = MemtoReg;
	ID_EX_old.Reg_Rs1 = rs1;
	ID_EX_old.Reg_Rs2 = rs2;
	ID_EX_old.Reg_Rd = reg[rd];
	ID_EX_old.PC = IF_ID.PC;
	ID_EX_old.Nop = 1;
	ID_EX_old.instnum = IF_ID.instnum;
	if((halt==0)&&((rs1==EXErd)||(rs2==EXErd))){//有冒险
		halt = 2;
		bubble = 2;
		data_haz_cnt += 2;
	}
	else if((halt==0)&&((rs1 == MEMrd)||(rs2 == MEMrd))){
		halt = 1;
		bubble = 1;
		data_haz_cnt += 1;
	}
	

	if(RegWrite||MemtoReg){
		EXErd = rd;
	}
	else{
		EXErd = -1;
	}

	#ifdef DEBUG
		printinst(PC,ALUop,rs1,rs2,rd,ID_EX_old.Imm);
		printf("MemWrite=%d MemRead=%d RegWrite=%d MemtoReg=%d\n",MemWrite,MemRead,RegWrite,MemtoReg);
	#endif
	#ifdef DEBUG_INST
		printf("ID ");
		printinst(IF_ID.PC,ALUop,rs1,rs2,rd,ID_EX_old.Imm);
	#endif
}

//执行
void EX()
{
	//read ID_EX
	if(ID_EX.Nop == 0){
		EX_MEM_old.Nop = 0;
		MEMrd = -1;
		return;
	}
	int temp_PC=ID_EX.PC;//此时的PC是加了1的PC
	int RegDst=ID_EX.Ctrl_EX_RegDst;
	int ALUOp=ID_EX.Ctrl_EX_ALUOp;
	int Branch = ID_EX.Ctrl_M_Branch;
	long long Imm = ID_EX.Imm;
	int Rs1 = ID_EX.Rs1;
	int Rs2 = ID_EX.Rs2;
	int Rd = ID_EX.Rd;
	#ifdef DEBUG_PIPE
	printf("EX inst %d\n",ID_EX.instnum);
	#endif
	//Branch PC calulate

	//choose ALU input number
	//alu calculate
	int Zero = reg[0];
	REG ALUout;
	switch(ALUOp){
	case ALU_ADD:
		ALUout = reg[Rs1] + reg[Rs2];
		break;
	case ALU_MULL:
		ALUout = ((reg[Rs1] * reg[Rs2]));
		break;
	case ALU_MULH:
		ALUout = Multi_H(reg[Rs1],reg[Rs2]);
		break;
	case ALU_SUB:
		ALUout = reg[Rs1] - reg[Rs2];
		break;
	case ALU_SLL:
		ALUout = reg[Rs1] << (unsigned int)reg[Rs2];
		break;
	case ALU_SLLIW:
		ALUout = reg[Rs1] << (unsigned int)Imm;
		break;
	case ALU_SLT:
		ALUout = ((long long)reg[Rs1]<(long long)reg[Rs2])?1:0;
		break;
	case ALU_XOR:
		ALUout = reg[Rs1] ^ reg[Rs2];
		break;
	case ALU_DIV:
		ALUout = (unsigned long long)(reg[Rs1] / reg[Rs2]);
		clk += 39;
		break;
	case ALU_SRL:
		ALUout = reg[Rs1] >> (unsigned int)reg[Rs2];
		break;
	case ALU_SRA:
		ALUout = (long long) reg[Rs1] >> (unsigned int)reg[Rs2];
		break;
	case ALU_OR:
		ALUout = reg[Rs1] | reg[Rs2];
		break;
	case ALU_REM:
		ALUout = reg[Rs1] % reg[Rs2];
		break;
	case ALU_AND:
		ALUout = reg[Rs1] & reg[Rs2];
		break;
	case ALU_LB:
	case ALU_LH:
	case ALU_LW:
	case ALU_LD:
		ALUout = reg[Rs1] + Imm;
		break;
	case ALU_ADDI:
		ALUout = Imm + reg[Rs1];
		break;
	case ALU_SLLI:
		ALUout = reg[Rs1] << (unsigned int)Imm;
		break;
	case ALU_SLTI:
		ALUout = ((long long)reg[Rs1]<Imm) ? 1:0;
		break;
	case ALU_XORI:
		ALUout = reg[Rs1] ^ Imm;
		break;
	case ALU_SRLI:
		ALUout = (unsigned long long)reg[Rs1] >> Imm;
		break;
	case ALU_SRLIW:
		ALUout = (unsigned long long)reg[Rs1] >> Imm;
		break;
	case ALU_SRAI:
		ALUout = (long long)reg[Rs1]>>Imm;
		break;
	case ALU_SRAIW:
		ALUout = (long long)reg[Rs1]>>Imm;
		break;
	case ALU_ORI:
		ALUout = reg[Rs1] | Imm;
		break;
	case ALU_ANDI:
		ALUout = reg[Rs1] & Imm;
		break;
	case ALU_ADDIW:
		ALUout = ext_signed(1,(int)(reg[Rs1] + Imm),32);
		break;
	case ALU_JALR:
		ALUout = temp_PC;
		temp_PC = reg[Rs1] + Imm;
		PC = temp_PC;//跳转 更新PC
		IF_ID_old.Nop = 0;
		ID_EX_old.Nop = 0;
		halt = 0;
		instcnt -= 2;
		control_haz_cnt += 2;
		break;
	case ALU_SB:
	case ALU_SH:
	case ALU_SW:
	case ALU_SD:
		ALUout = reg[Rs1] + Imm;
		break;
	case ALU_BEQ:
		if((long long)reg[Rs1]==(long long)reg[Rs2]){
			temp_PC = temp_PC - 4 + Imm;
			PC = temp_PC;
			IF_ID_old.Nop = 0;
			ID_EX_old.Nop = 0;
			halt = 0;
			instcnt -= 2;
			control_haz_cnt += 2;
		}
		break;
	case ALU_BNE:
		if((long long)reg[Rs1]!=(long long)reg[Rs2]){
			temp_PC = temp_PC - 4 + Imm;
			PC = temp_PC;
			IF_ID_old.Nop = 0;
			ID_EX_old.Nop = 0;
			halt = 0;
			instcnt -= 2;
			control_haz_cnt += 2;
		}
		break;
	case ALU_BLT:
		if((long long)reg[Rs1]<(long long)reg[Rs2]){
			temp_PC = temp_PC - 4 + Imm;
			PC = temp_PC;
			IF_ID_old.Nop = 0;
			ID_EX_old.Nop = 0;
			halt = 0;
			instcnt -= 2;
			control_haz_cnt += 2;
		}
		break;
	case ALU_BGE:
		if((long long)reg[Rs1]>=(long long)reg[Rs2]){
			temp_PC = temp_PC - 4 + Imm;
			PC = temp_PC;
			IF_ID_old.Nop = 0;
			ID_EX_old.Nop = 0;
			halt = 0;
			instcnt -= 2;
			control_haz_cnt += 2;
		}
		break;
	case ALU_AUIPC:
		ALUout = temp_PC - 4 + Imm;
		break;
	case ALU_LUI:
		ALUout = Imm;
		break;
	case ALU_JAL:
		ALUout = temp_PC;
		temp_PC = temp_PC - 4 + Imm;
		PC = temp_PC;
		halt = 0;
		IF_ID_old.Nop = 0;
		ID_EX_old.Nop = 0;
		instcnt -= 2;
		control_haz_cnt += 2;
		break;
	case ALU_ADDW:
		ALUout = ext_signed(1,(int)reg[Rs1] + reg[Rs2],32);
		break;
	case ALU_MULW:
		ALUout = ext_signed(1,(int)((int)reg[Rs1]*(int)reg[Rs2]),32);
		break;
	case ALU_DIVW:
		ALUout = ext_signed(1,int((int)reg[Rs1]/(int)reg[Rs2]),32);
		clk += 39;
		break;
	default:
		ALUout = 0;
	}

	//choose reg dst address
	/*int Reg_Dst;
	if(RegDst)
	{
		
	}
	else
	{

	}*/

	//write EX_MEM_old
	EX_MEM_old.ALU_out=ALUout;
	EX_MEM_old.PC=temp_PC;
	EX_MEM_old.Reg_dst = Rd;
	EX_MEM_old.Reg_Rt = Rs2;//用于save指令中的rs2
	EX_MEM_old.Zero = Zero;
	EX_MEM_old.ALU_op = ALUOp;
	EX_MEM_old.Ctrl_M_Branch = ID_EX.Ctrl_M_Branch;
	EX_MEM_old.Ctrl_M_MemWrite = ID_EX.Ctrl_M_MemWrite;
	EX_MEM_old.Ctrl_M_MemRead = ID_EX.Ctrl_M_MemRead;

	EX_MEM_old.Ctrl_WB_RegWrite = ID_EX.Ctrl_WB_RegWrite;
	EX_MEM_old.Ctrl_WB_MemtoReg = ID_EX.Ctrl_WB_MemtoReg;

	EX_MEM_old.Nop = 1;
	EX_MEM_old.instnum = ID_EX.instnum;
	
	if(ID_EX.Ctrl_WB_RegWrite||ID_EX.Ctrl_WB_MemtoReg){
		MEMrd = Rd;
	}
	else{
		MEMrd = -1;
	}

	#ifdef DEBUG
		printf("ALUout=%lld\n",ALUout);
	#endif
    //.....
}

//访问存储器
void MEM()
{

	if(EX_MEM.Nop==0){
		MEM_WB_old.Nop = 0;
		WBrd = -1;
		return;
	}
	

	//read EX_MEM
	int Branch = EX_MEM.Ctrl_M_Branch;
	int MemWrite = EX_MEM.Ctrl_M_MemWrite;
	int MemRead = EX_MEM.Ctrl_M_MemRead;
	REG ALUout = EX_MEM.ALU_out;
	int Reg_Dst = EX_MEM.Reg_dst;
	int ALU_op = EX_MEM.ALU_op;
	unsigned long long Mem_read = 0;
	char tmp1 = 0;
	short tmp2 = 0;
	int tmp3 = 0;
	long long tmp4 = 0;
	int Reg_Rt = EX_MEM.Reg_Rt;

	#ifdef DEBUG_PIPE
	printf("MEM inst %d\n",EX_MEM.instnum);
	#endif

	if(MemRead){
		switch(ALU_op){
			case ALU_LB:
			//	memcpy(&tmp1,&memory[ALUout],1);
				visit_memo(ALUout,&tmp1,1,1,clk);
				Mem_read = ext_signed(1,(int)tmp1,8);
				break;
			case ALU_LH:
				//memcpy(&tmp2,&memory[ALUout],2);
			visit_memo(ALUout,&tmp2,1,2,clk);
				Mem_read = ext_signed(1,(int)tmp2,16);
				break;
			case ALU_LW:
			//	memcpy(&tmp3,&memory[ALUout],4);
				visit_memo(ALUout,&tmp3,1,4,clk);
				Mem_read = ext_signed(1,(int)tmp3,32);
				break;
			case ALU_LD:
			//	memcpy(&tmp4,&memory[ALUout],8);
				visit_memo(ALUout,&tmp4,1,8,clk);
				Mem_read = tmp4;
				break;
			default:
				Mem_read = 0;
		}
	}
	if(MemWrite){
		unsigned char val1;
		short val2;
		unsigned int val3;
		unsigned long long val4;
		int val;
		switch(ALU_op){
			case ALU_SB:
				 val1 = (unsigned char)reg[Reg_Rt];//读高位?
			//	memcpy(&memory[ALUout],&val1,1);
				 visit_memo(ALUout,&val1,0,1,clk);
				break;
			case ALU_SH:
				val2 = (short)reg[Reg_Rt];
				visit_memo(ALUout,&val2,0,2,clk);
				//memcpy(&memory[ALUout],&val2,2);
				break;
			case ALU_SW:
				val3 = (unsigned int)reg[Reg_Rt];
				visit_memo(ALUout,&val3,0,4,clk);
			//	memcpy(&memory[ALUout],&val3,4);
				break;
			case ALU_SD:
				val4 = reg[Reg_Rt];
			//	memcpy(&memory[ALUout],&val4,8);
				visit_memo(ALUout,&val4,0,8,clk);
				break;
			default:
				val = 0;
		}
	}

	//complete Branch instruction PC change
	//read / write memory
	MEM_WB_old.Mem_read = Mem_read;
	MEM_WB_old.ALU_out = ALUout;
	MEM_WB_old.Reg_dst = Reg_Dst;
	MEM_WB_old.Ctrl_WB_MemtoReg = EX_MEM.Ctrl_WB_MemtoReg;
	MEM_WB_old.Ctrl_WB_RegWrite = EX_MEM.Ctrl_WB_RegWrite;

	MEM_WB_old.Nop = 1;
	MEM_WB_old.instnum = EX_MEM.instnum;

	if(EX_MEM.Ctrl_WB_RegWrite||EX_MEM.Ctrl_WB_MemtoReg){
	WBrd = Reg_Dst;}
	else{
		WBrd = -1;
	}

	#ifdef DEBUG
		if(MemRead){
			printf("read memory %d, content is %llx\n",ALUout,Mem_read);
		}
		if(MemWrite){
			printf("write memory %d, writing content is %llx\n",ALUout,reg[Reg_Rt]);
		}
		printf("new pc is %x\n",PC);
	#endif

	//write MEM_WB_old
}


//写回
void WB()
{
	//read MEM_WB
	if(MEM_WB.Nop==0){
		return;
	}
	#ifdef DEBUG_PIPE
	printf("WB inst %d\n",MEM_WB.instnum);
	#endif
	int Reg_Dst = MEM_WB.Reg_dst;
	int MemtoReg = MEM_WB.Ctrl_WB_MemtoReg;
	int RegWrite = MEM_WB.Ctrl_WB_RegWrite;
	REG ALUout = MEM_WB.ALU_out;
	unsigned long long Mem_read= MEM_WB.Mem_read;
	if(RegWrite){
		reg[Reg_Dst] = ALUout;
	}
	if(MemtoReg){
		reg[Reg_Dst] = Mem_read;
	}
	#ifdef DEBUG
	if(RegWrite){
		printf("write reg %d with ALUout, content is %llx\n",Reg_Dst,ALUout);
	}
	if(MemtoReg){
		printf("write reg %d with Memory, content is %llx\n",Reg_Dst,Mem_read);
	}
	printf("\n");
	#endif
	#ifdef DEBUG_REG
	printreg();
	printf("\n");
	#endif
	
	//write reg
}
