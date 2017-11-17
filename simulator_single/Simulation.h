#include<string.h>
#include<iostream>
#include<stdio.h>
#include<math.h>
#include<time.h>
#include<stdlib.h>
#include"Reg_def.h"
#include "Read_Elf.h"

//#define DEBUG
#define DEBUG_REG
#define DEBUG_INST
#define DEBUG_MEM

#define ALU_ADD 100
#define ALU_SUB 1
#define ALU_MULL 2
#define ALU_DIV 3
#define ALU_SLL 4
#define ALU_SRL 5
#define ALU_AND 6
#define ALU_OR 7
#define ALU_XOR 8
#define ALU_SLT 9
#define ALU_SRA 10
#define ALU_REM 11
#define ALU_MULH 12
#define ALU_LB 13
#define ALU_LH 14
#define ALU_LW 15
#define ALU_LD 16
#define ALU_ADDI 17
#define ALU_SLLI 18
#define ALU_SLTI 19
#define ALU_XORI 20
#define ALU_SRLI 21
#define ALU_SRAI 22
#define ALU_ORI 23
#define ALU_ANDI 24
#define ALU_ADDIW 25
#define ALU_JALR 26
#define ALU_ECALL 27
#define ALU_SB 28
#define ALU_SH 29
#define ALU_SW 30
#define ALU_SD 31
#define ALU_BEQ 32
#define ALU_BNE 33
#define ALU_BLT 34
#define ALU_BGE 35
#define ALU_AUIPC 36
#define ALU_LUI 37
#define ALU_JAL 38
#define ALU_ADDW 39
#define ALU_MULW 40
#define ALU_DIVW 41
#define ALU_SLLIW 42
#define ALU_SRLIW 43
#define ALU_SRAIW 44


#define OP_JAL 111
#define OP_S 0x23
#define OP_R 51
#define OP_I1 3
#define OP_I2 0x13
#define OP_I3 0x1b
#define OP_I4 0x67
#define OP_I5 0x73
#define OP_SB 0x63
#define OP_U1 0x17
#define OP_U2 0x37
#define OP_UJ 0x6f
#define OP_E 0x3b

#define EXT_MEM 1 //extend the value of memory
#define EXT_IMM 2 // extend the imm num
#define EXT_ALUOUT 3 // extend alu out
#define EXT_ALUOUTIMM 4//extend alu out and imm num
/*
#define F3_ADD 0
#define F3_MULL 0
#define F3_SUB 0
#define F3_SLL 1
#define F3_MULH 1
#define F3_SLT 2
#define F3_XOR 4
#define F3_DIV 4
#define F3_SRL 5
#define F3_SRA 5
*/
#define F7_MSE 1
#define F7_ADD 0

#define F3_ADDI 0

#define F3_SB 0


#define F3_LB 0


#define F3_BEQ 0


#define F3_ADDIW 0


#define F3_ADDW 0
#define F7_ADDW 0



#define F3_SCALL 0
#define F7_SCALL 0

//100000000
#define MAX 400000000

//主存
unsigned char memory[MAX]={0};
//寄存器堆
REG reg[32]={0};
//PC
int PC=0;


//各个指令解析段
unsigned int OP=0;
unsigned int fuc3=0,fuc7=0;
int shamt=0;
int rs1=0,rs2=0,rd=0;
int imm=0;



//加载内存
void load_memory();

void simulate();

void IF();

void ID();

void EX();

void MEM();

void WB();


//符号扩展
long long ext_signed(int op,int imm,int len);

void printinst(int pc,int ALUop,int rs1,int rs2,int rd,long long imm);
void printreg();

//读内存
unsigned long long read_mem(int index,int len);
//获取指定位
unsigned int getbit(int s,int e);
//print reg content 
void printreg(){
	printf("reg content: \n");
	for(int i = 0;i<32;i++){
		printf("%llx ",reg[i]);
		if(i!=0&&i%8==0)
			printf("\n");
	}
	printf("\n");
}

unsigned int getbit(unsigned inst,int s,int e)
{
	unsigned int res = 0;
	unsigned int one = ((1 << (e+1))-1) - ((1<<s) - 1);
	res = (inst&one) >> s;
	return res;
}

unsigned long long read_mem(int index,int len){//len = 长度/8
	unsigned long long res = 0;
	for(int i = 0;i<len;i++){
		res += (memory[i+index] << (8*i));
	}
	return res;
}

long long ext_signed(int op,int imm,int len)
{
   if(op){
   		int high = (imm&(1<<(len-1)))>>(len-1);
   		if (high==0){
   			return (long long)imm;
   		}
   		else{
   			if(len<32){
   		//	printf("fushu initial imm is %d,len is %d\n",imm,len);
   			int  res = (imm-(1<<len));
   			return (long long)(res);}
   			else
   				return (long long)imm;
   		}
   }
   else return (long long)imm;
}

