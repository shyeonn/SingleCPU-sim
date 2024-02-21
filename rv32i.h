/* **************************************
 * Module: top design of rv32i single-cycle processor
 *
 * Author:
 *
 * **************************************
 */

// headers
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


// defines
#define REG_WIDTH 32
#define IMEM_DEPTH 1024
#define DMEM_DEPTH 1024

// configs
#define CLK_NUM 45

// structures
struct imem_input_t {
	uint32_t addr;
};

struct imem_output_t {
	uint32_t dout;
};

struct control_output_t {
	uint8_t branch;
	uint8_t mem_read;
	uint8_t memtoreg;
	uint8_t aluop;
	uint8_t mem_write;
	uint8_t alu_src;
	uint8_t reg_write;
};

struct regfile_input_t {
	uint8_t rs1;
	uint8_t rs2;
	uint8_t rd;
	uint32_t rd_din;
	uint8_t reg_write;
};

struct regfile_output_t {
	uint32_t rs1_dout;
	uint32_t rs2_dout;
};

struct alu_input_t {
	uint32_t in1;
	uint32_t in2;
	uint8_t alu_control;
};

struct alu_output_t {
	uint32_t result;
	uint8_t zero;
	uint8_t sign;
}
