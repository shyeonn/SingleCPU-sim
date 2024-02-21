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
struct control_output_t control_unit(uint32_t opcode);
struct regfile_output_t regfile(struct regfile_input_t regfile_in, uint32_t *reg_data);

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
	uint32_t pc_curr, pc_next;	// program counter
	struct imem_input_t imem_in;
	struct imem_output_t imem_out;

	struct regfile_input_t regfile_in;
	struct regfile_output_t regfile_out;
	uint8_t opcode;
	struct control_output_t control_out;

	uint32_t cc = 2;	// clock count
				
	//Initialize 
	pc_curr = 0;

	while (cc < CLK_NUM) {
		// instruction fetch
		imem_in.addr = pc_curr;
		pc_next = pc_curr + 4;

		imem_out = imem(imem_in, imem_data);
		P_DEBUG("dout : %08X\n", imem_out.dout);

		// instruction decode
		opcode = imem_out.dout & 0x3F;
		P_DEBUG("opcode : %x\n", opcode);

		//control_out = control_unit(opcode);

		regfile_in.rs1 = (imem_out.dout >> 15) & 0x3F;
		regfile_in.rd = (imem_out.dout >> 7) & 0x3F;

		if (opcode == SB-Type || opcode == R-Type)
		  regfile_in.rs2 = (imem_out.dout >> 20) & 0x3F;

		//Immediate generation
		if (opcode == U-Type)
		  alu_in.in2 = (imem_out.dout >> 12) & 0xFFFFF;
		else if (opcode == I_L-Type || opcode == I_R-Type)
		  alu_in.in2 = (imem_out.dout >> 20) & 0xFFF;
		
		regfile_out = regfile(regfile_in, reg_data, READ);

//		// execution
//		alu_out = alu(alu_in);
//		// memory
//		dmem_out = dmem(dmem_in, dmem_data);
//		// write-back
		
		pc_curr = pc_next;
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

struct control_output_t control_unit(uint32_t opcode){
	struct control_output_t control;

	switch (opcode) {
		//R-Type
		case 0b0110011:
			control.branch = 0;
			control.mem_read = 0;
			control.memtoreg = 0;
			control.aluop = 0b10;
			control.mem_write = 0;
			control.alu_src = 1;
			control.reg_write = 1; 
			break;
		//I-Type
		case 0b0010011:
		case 0b0000011:
			control.branch = 0;
			control.mem_read = 1;
			control.memtoreg = 1;
			control.aluop = 0b10;
			control.mem_write = 0;
			control.alu_src = 1;
			control.reg_write = 1; 
			break;
		//S-Type
		case 0b0100011:
			control.branch = 0;
			control.mem_read = 0;
			control.memtoreg = 1;
			control.aluop = 0b00;
			control.mem_write = 1;
			control.alu_src = 1;
			control.reg_write = 0; 
			break;
		//SB-Type
		case 0b1100011:
			control.branch = 1;
			control.mem_read = 0;
			control.memtoreg = 0;
			control.aluop = 0b10;
			control.mem_write = 0;
			control.alu_src = 0;
			control.reg_write = 0; 
			break;
		//U-Type
		case 0b0110111:
			control.branch = 0;
			control.mem_read = 0;
			control.memtoreg = 0;
			control.aluop = 0b11;
			control.mem_write = 0;
			control.alu_src = 1;
			control.reg_write = 1; 
			break;
		//UJ-Type
		case 0b1101111:
			control.branch = 1;
			control.mem_read = 1;
			control.memtoreg = 1;
			control.aluop = 0b11;
			control.mem_write = 0;
			control.alu_src = 1;
			control.reg_write = 1; 
			break;
	}

	return control;
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
