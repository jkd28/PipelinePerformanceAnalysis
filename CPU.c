/**************************************************************/
/* CS/COE 1541
   just compile with gcc -o pipeline pipeline.c
   and execute using
   ./pipeline  /afs/cs.pitt.edu/courses/1541/short_traces/sample.tr	0
***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include "CPU.h"

int main(int argc, char **argv)
{
  struct trace_item *tr_entry;
  size_t size;
  char *trace_file_name;
  int trace_view_on = 0;
  int branch_prediction_on = 0;

  unsigned char t_type = 0;
  unsigned char t_sReg_a= 0;
  unsigned char t_sReg_b= 0;
  unsigned char t_dReg= 0;
  unsigned int t_PC = 0;
  unsigned int t_Addr = 0;

  unsigned int cycle_number = 0;

  struct bpt_entry bp_table[64]; //creates branch prediction table

  // Check for improper usage
  if (argc == 1 || argc > 4) {
    fprintf(stdout, "\nUSAGE: ./CPU <trace file> <trace switch> <branch prediction switch>");
    fprintf(stdout, "\n(trace switch) to turn on or off individual item view.");
    fprintf(stdout, "\n(brach prediction switch) to turn on or off branch prediction table.\n");
    exit(0);
  }

  // Handle input values and set necessary values
  trace_file_name = argv[1];
  if (argc >= 3) trace_view_on = atoi(argv[2]) ;
  if (argc == 4) branch_prediction_on = atoi(argv[3]);

  fprintf(stdout, "\n ** opening file %s\n", trace_file_name);

  // Open the trace file
  trace_fd = fopen(trace_file_name, "rb");

  // Handle trace file error
  if (!trace_fd) {
    fprintf(stdout, "\ntrace file %s not opened.\n\n", trace_file_name);
    exit(0);
  }

  // Initialize the trace buffer
  trace_init();

  // Begin simulation
  while(1) {
    // Check that there are remaining instructions in the buffer
    size = trace_get_item(&tr_entry);

    if (!size) {
      // No more instructions (trace_items) to simulate
      printf("+ Simulation terminates at cycle : %u\n", cycle_number);
      break;
    }
    else{
      // Parse the next instruction to simulate
      cycle_number++;
      t_type = tr_entry->type;
      t_sReg_a = tr_entry->sReg_a;
      t_sReg_b = tr_entry->sReg_b;
      t_dReg = tr_entry->dReg;
      t_PC = tr_entry->PC;
      t_Addr = tr_entry->Addr;
    }

// SIMULATION OF A SINGLE CYCLE cpu IS TRIVIAL - EACH INSTRUCTION IS EXECUTED
// IN ONE CYCLE

    if (trace_view_on) {/* print the executed instruction if trace_view_on=1 */
      switch(tr_entry->type) {
        case ti_NOP:
          printf("[cycle %d] NOP:",cycle_number) ;
          break;
        case ti_RTYPE:
          printf("[cycle %d] RTYPE:",cycle_number) ;
          printf(" (PC: %x)(sReg_a: %d)(sReg_b: %d)(dReg: %d) \n", tr_entry->PC, tr_entry->sReg_a, tr_entry->sReg_b, tr_entry->dReg);
          break;
        case ti_ITYPE:
          printf("[cycle %d] ITYPE:",cycle_number) ;
          printf(" (PC: %x)(sReg_a: %d)(dReg: %d)(addr: %x)\n", tr_entry->PC, tr_entry->sReg_a, tr_entry->dReg, tr_entry->Addr);
          break;
        case ti_LOAD:
          printf("[cycle %d] LOAD:",cycle_number) ;
          printf(" (PC: %x)(sReg_a: %d)(dReg: %d)(addr: %x)\n", tr_entry->PC, tr_entry->sReg_a, tr_entry->dReg, tr_entry->Addr);
          break;
        case ti_STORE:
          printf("[cycle %d] STORE:",cycle_number) ;
          printf(" (PC: %x)(sReg_a: %d)(sReg_b: %d)(addr: %x)\n", tr_entry->PC, tr_entry->sReg_a, tr_entry->sReg_b, tr_entry->Addr);
          break;
        case ti_BRANCH:
          printf("[cycle %d] BRANCH:",cycle_number) ;
          printf(" (PC: %x)(sReg_a: %d)(sReg_b: %d)(addr: %x)\n", tr_entry->PC, tr_entry->sReg_a, tr_entry->sReg_b, tr_entry->Addr);
          break;
        case ti_JTYPE:
          printf("[cycle %d] JTYPE:",cycle_number) ;
          printf(" (PC: %x)(addr: %x)\n", tr_entry->PC,tr_entry->Addr);
          break;
        case ti_SPECIAL:
          printf("[cycle %d] SPECIAL:",cycle_number) ;
          break;
        case ti_JRTYPE:
          printf("[cycle %d] JRTYPE:",cycle_number) ;
          printf(" (PC: %x) (sReg_a: %d)(addr: %x)\n", tr_entry->PC, tr_entry->dReg, tr_entry->Addr);
          break;
      }
    }
  }

  // Uninitialize the trace buffer
  trace_uninit();

  exit(0);
}
