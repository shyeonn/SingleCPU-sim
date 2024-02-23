/* **************************************
 * Module: top design of rv32i single-cycle processor
 *
 * Author:
 *
 * **************************************
 */

#include "rv32i.h"

#define P_DEBUG printf

struct imem_output_t imem(struct imem_input_t imem_in, uint32_t *imem_data);
struct regfile_output_t regfile(struct regfile_input_t regfile_in, uint32_t *reg_data, enum REG regwrite);
struct alu_output_t alu(struct alu_input_t alu_in);
uint8_t alu_control_gen(uint8_t opcode, uint8_t func3, uint8_t func7);
struct dmem_output_t dmem(struct dmem_input_t dmem_in, uint32_t *dmem_data);

int main (int argc, char *argv[]) {

	// get input arguments
	FILE *f_imem, *f_dmem;
	if (argc < 3) {
		printf("usage: %s imem_data_file dmem_data_file\n", argv[0]);
		exit(1);
	}

	if ( (f_imem = fopen(argv[1], "r")) == NULL ) {
		printf("Cannot find %s\n", argv[1]);
		exit(1);
	}
	if ( (f_dmem = fopen(argv[2], "r")) == NULL ) {
		printf("Cannot find %s\n", argv[2]);
		exit(1);
	}

	// memory data (global)
	uint32_t *reg_data;
	uint32_t *imem_data;
	uint32_t *dmem_data;

	reg_data = (uint32_t*)malloc(32*sizeof(uint32_t));
	imem_data = (uint32_t*)malloc(IMEM_DEPTH*sizeof(uint32_t));
	dmem_data = (uint32_t*)malloc(DMEM_DEPTH*sizeof(uint32_t));

	// initialize memory data
	int i, j, k;
	for (i = 0; i < 32; i++) reg_data[i] = 0;
	for (i = 0; i < IMEM_DEPTH; i++) imem_data[i] = 0;
	for (i = 0; i < DMEM_DEPTH; i++) dmem_data[i] = 0;

	uint32_t d, buf;
	i = 0;
	printf("\n*** Reading %s ***\n", argv[1]);
	while (fscanf(f_imem, "%1d", &buf) != EOF) {
		d = buf << 31;
		for (k = 30; k >= 0; k--) {
			if (fscanf(f_imem, "%1d", &buf) != EOF) {
				d |= buf << k;
			} else {
				printf("Incorrect format!!\n");
				exit(1);
			}
		}
		imem_data[i] = d;
		printf("imem[%03d]: %08X\n", i, imem_data[i]);
		i++;
	}

	i = 0;
	printf("\n*** Reading %s ***\n", argv[2]);
	while (fscanf(f_dmem, "%8x", &buf) != EOF) {
		dmem_data[i] = buf;
		printf("dmem[%03d]: %08X\n", i, dmem_data[i]);
		i++;
	}

	fclose(f_imem);
	fclose(f_dmem);

	// processor model
	// IF
	uint32_t pc_curr, pc_next;	// program counter
	struct imem_input_t imem_in;
	struct imem_output_t imem_out;

	// ID
	struct regfile_input_t regfile_in;
	struct regfile_output_t regfile_out;
	uint8_t opcode;
	uint8_t func3;
	uint8_t func7;
	struct control_output_t control_out;

	// EX
	struct alu_input_t alu_in;
	struct alu_output_t alu_out;

	// M
	struct dmem_input_t dmem_in;
	struct dmem_output_t dmem_out;

	uint32_t cc = 2;	// clock count
				
	//Initialize 
	pc_curr = 0;

	while (cc < CLK_NUM) {
		// instruction fetch
		pc_curr = pc_next;
		imem_in.addr = pc_curr;

		imem_out = imem(imem_in, imem_data);

		// instruction decode
		opcode = imem_out.dout & 0x3F;

		if (!(opcode == U_TYPE && opcode == UJ_TYPE))
			func3 = (imem_out.dout >> 12) & 0x7;
		if (opcode == R_TYPE)
			func7 = (imem_out.dout >> 25) & 0x7F;

		regfile_in.rs1 = (imem_out.dout >> 15) & 0x3F;
		regfile_in.rd = (imem_out.dout >> 7) & 0x3F;

		if (opcode == SB_TYPE || opcode == R_TYPE)
			regfile_in.rs2 = (imem_out.dout >> 20) & 0x3F;

		regfile_out = regfile(regfile_in, reg_data, READ);

		alu_in.in1 = regfile_out.rs1_dout;

		//Immediate generation
		if (opcode == U_TYPE)
			alu_in.in2 = (imem_out.dout >> 12) & 0xFFFFF;
		else if (opcode == I_L_TYPE || opcode == I_R_TYPE)
			alu_in.in2 = (imem_out.dout >> 20) & 0xFFF;
		else
			alu_in.in2 = regfile_out.rs2_dout;

		// execution
		alu_in.alu_control = alu_control_gen(opcode, func3, func7);
		alu_out = alu(alu_in);

		// memory
		if(opcode == S_TYPE || opcode == I_L_TYPE){
			dmem_in.addr = alu_out.result;
			dmem_in.din = regfile_out.rs2_dout;

			if(opcode == S_TYPE)
				dmem_in.mem_write = 1;
			else
				dmem_in.mem_write = 0;

			dmem_out = dmem(dmem_in, dmem_data);
		}

		// write-back
		if(!(opcode == SB_TYPE || opcode == S_TYPE)){
			regfile_in.rd_din = regfile_out.rs2_dout;
			regfile_out = regfile(regfile_in, reg_data, WRITE);
		}

		// Program counter
		pc_next = pc_curr + 4;

		if(opcode == SB_TYPE){
			uint8_t pc_next_sel = 0;
			uint32_t r1 = regfile_out.rs1_dout;
			uint32_t r2 = regfile_out.rs2_dout;

			switch(func3){
				case F3_BEQ:
					pc_next_sel = r1 == r2;
					break;
				case F3_BNE:
					pc_next_sel = r1 != r2;
					break;
				case F3_BLT:
					pc_next_sel = (int32_t)r1 < (int32_t)r2;
					break;
				case F3_BGE:
					pc_next_sel = (int32_t)r1 >= (int32_t)r2;
					break;
				case F3_BLTU:
					pc_next_sel = r1 < r2;
					break;
				case F3_BGEU:
					pc_next_sel = r1 >= r2;
					break;
			}
			if(pc_next_sel){
				uint32_t imm = 0;

				imm = imm | ((imem_out.dout >> 31) & 0x1) << 12;
				imm = imm | ((imem_out.dout >> 25) & 0x3F) << 5;
				imm = imm | ((imem_out.dout >> 8) & 0xF) << 1;
				imm = imm | ((imem_out.dout >> 7) & 0x1) << 11;
				
				pc_next = pc_curr + imm;
			}
		}
		cc++;
	}

	free(reg_data);
	free(imem_data);
	free(dmem_data);

	return 1;
}

struct imem_output_t imem(struct imem_input_t imem_in, uint32_t *imem_data) {
	
	struct imem_output_t imem_out;

	imem_out.dout = imem_data[imem_in.addr/4];

	return imem_out;
}

struct regfile_output_t regfile(struct regfile_input_t regfile_in, uint32_t *reg_data, enum REG regwrite){

	struct regfile_output_t regfile_out;

	if(regwrite == READ){
		regfile_out.rs1_dout = reg_data[regfile_in.rs1];
		if(regfile_in.rs2 < REG_WIDTH)
			regfile_out.rs2_dout = reg_data[regfile_in.rs2];
	}
	else if(regwrite  = WRITE){
		reg_data[regfile_in.rd] = regfile_in.rd_din;
	}

	return regfile_out;
}

struct alu_output_t alu(struct alu_input_t alu_in){

	struct alu_output_t alu_out;

	alu_out.zero = 1;
	alu_out.sign = 1;

	switch(alu_in.alu_control){
		case C_AND:
			alu_out.result = alu_in.in1 & alu_in.in2;
		case C_OR:
			alu_out.result = alu_in.in1 | alu_in.in2;
		case C_XOR:
			alu_out.result = alu_in.in1 ^ alu_in.in2;
		case C_SL:
			alu_out.result = alu_in.in1 << (int32_t)alu_in.in2;
		case C_SR:
			alu_out.result = alu_in.in1 >> (int32_t)alu_in.in2;
		case C_SRA:
			alu_out.result = (int32_t)alu_in.in1 >> (int32_t)alu_in.in2;
		case C_SUB:
			alu_out.result = (int32_t)alu_in.in1 - (int32_t)alu_in.in2;
		default:
			alu_out.result = (int32_t)alu_in.in1 + (int32_t)alu_in.in2;
	}

	if(alu_out.result)
	  alu_out.zero = 0;

	if(alu_out.result > 0)
	  alu_out.sign = 0;
	//does not handle the case of sign = 1 when result is zero

	return alu_out;
}

uint8_t alu_control_gen(uint8_t opcode, uint8_t func3, uint8_t func7){

	if(opcode == I_L_TYPE && opcode == S_TYPE)
		return C_ADD;
	else if(opcode == SB_TYPE)
		return C_SUB;
	else {
		switch(func3){
		//Arithmetic & Shifts
			case F3_SL:
				return C_SL;
			case F3_SR:
				if(((func7 >> 6) & 1))
					return C_SUB;
				else
					return C_ADD;
			case F3_ADD_SUB:
				if(((func7 >> 6) & 1))
					return C_SRA;
			else
				return C_SR;
			case F3_XOR:
				return C_XOR;
			case F3_OR:
				return C_OR;
			case F3_AND:
				return C_AND;
			case F3_SLT:
			case F3_SLTU:
				return C_SUB;
		}
	}
}

struct dmem_output_t dmem(struct dmem_input_t dmem_in, uint32_t *dmem_data) {
	
	struct dmem_output_t dmem_out;

	if(dmem_in.mem_read)
		dmem_out.dout = dmem_data[dmem_in.addr/4];
	if(dmem_in.mem_write)
		dmem_data[dmem_in.addr/4] = dmem_in.din;

	return dmem_out;
}
