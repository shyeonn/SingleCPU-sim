/* **************************************
 * Module: top design of rv32i single-cycle processor
 *
 * Author:
 *
 * **************************************
 */

#include "rv32i.h"

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

	uint32_t cc = 2;	// clock count
	
	//initialize
	imem_in.addr = 0;

	while (cc < CLK_NUM) {
		// instruction fetch
		imem_out = imem(imem_in, imem_data);
		// instruction decode
		regfile_out = regfile(regfile_in, reg_data);
		// execution
		alu_out = alu(alu_in);
		// memory
		dmem_out = dmem(dmem_in, dmem_data);
		// write-back
		
		cc++;
	}

	free(reg_data);
	free(imem_data);
	free(dmem_data);

	return 1;
}

struct imem_output_t imem(struct imem_input_t imem_in, uint32_t *imem_data) {
	uint32_t curr_inst = imem_data[imem_in.addr];
	
	struct imem_output_t imem_out;

	//
	


	

}
