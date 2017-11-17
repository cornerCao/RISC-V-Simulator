typedef unsigned long long REG;

struct IFID{
	int instnum;
	int Nop;
	unsigned int inst;
	int PC;
}IF_ID,IF_ID_old;


struct IDEX{
	int instnum;
	int Nop;
	int Rd,Rs1,Rs2;
	int PC;
	long long Imm;
	REG Reg_Rs1,Reg_Rs2,Reg_Rd;

	int Ctrl_EX_ALUSrc1;
	int Ctrl_EX_ALUSrc2;
	int Ctrl_EX_ALUOp;
	int Ctrl_EX_RegDst;

	int Ctrl_M_Branch;
	int Ctrl_M_MemWrite;
	int Ctrl_M_MemRead;

	int Ctrl_WB_RegWrite;
	int Ctrl_WB_MemtoReg;

}ID_EX,ID_EX_old;

struct EXMEM{
	int instnum;
	int Nop;
	int PC;
	int Reg_dst;
	REG ALU_out;
	int Zero;
	REG Reg_Rt;
	int ALU_op;

	int Ctrl_M_Branch;
	int Ctrl_M_MemWrite;
	int Ctrl_M_MemRead;

	int Ctrl_WB_RegWrite;
	int Ctrl_WB_MemtoReg;

}EX_MEM,EX_MEM_old;

struct MEMWB{
	int instnum;
	int Nop;
	unsigned long long Mem_read;
	REG ALU_out;
	int Reg_dst;
		
	int Ctrl_WB_RegWrite;
	int Ctrl_WB_MemtoReg;

}MEM_WB,MEM_WB_old;