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

struct trace_item * no_op_initializer();

struct bpt_entry {
   unsigned char taken;
   unsigned int address;
};

int main(int argc, char **argv)
{
   // define constants
   const int pipeline_size = 5;

   // variable declarations
   struct trace_item *tr_entry;
   struct trace_item *temp1, *temp2;
   size_t size;
   char *trace_file_name;
   int trace_view_on = 0;
   int branch_prediction_on = 0;
   int lw_hazard_detected = 0;
   int squashing = 0;
   int buffer_skip = 0;
   unsigned int cycle_number = 0;

   struct trace_item *pipeline[pipeline_size];
   struct bpt_entry *bp_table[64];
   const unsigned int bpt_hash = 1008;
   unsigned int bpt_index;

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

   // Initialize pipeline with each step at NULL
   for(int i= 0;i<(sizeof(pipeline)/sizeof(struct trace_item *));i++)
      pipeline[i]= NULL;

   if(branch_prediction_on)
   {
      for(int i= 0;i<(sizeof(bp_table)/sizeof(struct bpt_entry *));i++)
         bp_table[i]= NULL;
   }

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
      if (!buffer_skip){
         // Check that there are remaining instructions in the buffer
         size = trace_get_item(&tr_entry);
      }
      // reset the buffer skip signal after 1 pass
      buffer_skip = 0;

      if(!size){
         tr_entry= NULL;

         if (pipeline[4]==NULL) {
            // No more instructions (trace_items) to simulate
            printf("+ Simulation terminates at cycle : %u\n", cycle_number);
            break;
         }
      }

      // Hazard Detection
      if ((pipeline[1] != NULL) && (pipeline[0] != NULL)){
         if (pipeline[1]->type == ti_LOAD) {
            // Load Word Detected
            if ((pipeline[0]->type == ti_RTYPE) || (pipeline[0]->type == ti_STORE) || (pipeline[0]->type == ti_BRANCH)){
               if ((pipeline[1]->dReg == pipeline[0]->sReg_a) || (pipeline[1]->dReg == pipeline[0]->sReg_b)){
                  // Load-Use Hazard Detected
                  lw_hazard_detected = 1;
               }
            }
            else if ((pipeline[0]->type == ti_ITYPE) || (pipeline[0]->type == ti_LOAD) || (pipeline[0]->type == ti_JRTYPE)){
               if(pipeline[1]->dReg == pipeline[0]->sReg_a){
                  // Load-Use Hazard Detected
                  lw_hazard_detected = 1;
               }
            }
         }
      }

      //Advance instructions through pipeline
      //   pipeline[0] => IF/ID
      //   pipeline[1] => ID/EX
      //   pipeline[2] => EX/MEM
      //   pipeline[3] => MEM/WB
      //   pipeline[4] => Output

      // Handle LOAD-USE Hazard
      if (lw_hazard_detected){
         // Advance the pipeline from the ID/EX Buffer on
         pipeline[4] = pipeline[3];
         pipeline[3] = pipeline[2];
         pipeline[2] = pipeline[1];

         // Insert a bubble at ID/EX
         pipeline[1] = no_op_initializer();

         // Unset the flag
         lw_hazard_detected = 0;
         buffer_skip = 1;
      }
      else if(squashing) { //handle instruction
         if(squashing==2)
         {
            tr_entry = temp2;

            buffer_skip = 1;
         }
         else
            tr_entry = temp1;

         pipeline[4] = pipeline[3];
         pipeline[3] = pipeline[2];
         pipeline[2] = pipeline[1];
         pipeline[1] = pipeline[0];
         pipeline[0] = tr_entry;

         squashing--;
      }
      else {
         // Advance pipeline normally
         pipeline[4] = pipeline[3]; // MEM/WB to OUTPUT
         pipeline[3] = pipeline[2]; // EX/MEM to MEM/WB
         pipeline[2] = pipeline[1]; // ID/EX to EX/MEM
         pipeline[1] = pipeline[0]; // IF/ID to ID/EX
         pipeline[0] = tr_entry;    // PC to IF/ID
      }

      if(pipeline[2]!=NULL && (pipeline[2]->type == ti_JTYPE || pipeline[2]->type == ti_JRTYPE)) //jump handler
      {
         temp1 = pipeline[0]; //removes instructions from pipeline
         temp2 = pipeline[1];

         pipeline[1] = no_op_initializer(); //inserts bubble (squashes instructions)
         pipeline[0] = no_op_initializer();

         squashing = 2; //sets flags
         buffer_skip = 1;
      }

      if(pipeline[1]!=NULL && pipeline[2]!=NULL && pipeline[2]->type == ti_BRANCH) //branch handler
      {
         if(!branch_prediction_on) //no branch prediction table (assume "not taken")
         {
            if(pipeline[2]->Addr == pipeline[1]->PC) //branch taken
            {
               temp1 = pipeline[0]; //removes instructions from pipeline
               temp2 = pipeline[1];

               pipeline[1] = no_op_initializer(); //inserts bubble (squashes instructions)
               pipeline[0] = no_op_initializer();

               squashing = 2; //sets flags
               buffer_skip = 1;
            }
         }
         else //single bit branch prediction table
         {
            bpt_index = (pipeline[2]->PC & bpt_hash) >> 4;

            if(bp_table[bpt_index]==NULL) //if entry has not been initialized yet (assume "not taken")
            {
               bp_table[bpt_index]= (struct bpt_entry *)malloc(sizeof(struct bpt_entry));
               bp_table[bpt_index]->address= pipeline[2]->PC;

               if(pipeline[2]->Addr == pipeline[1]->PC) //branch taken
               {
                  bp_table[bpt_index]->taken= 1;

                  temp1 = pipeline[0]; //removes instructions from pipeline
                  temp2 = pipeline[1];

                  pipeline[1] = no_op_initializer(); //inserts bubble (squashes instructions)
                  pipeline[0] = no_op_initializer();

                  squashing = 2; //sets flags
                  buffer_skip = 1;
               }
               else //branch not taken
                  bp_table[bpt_index]->taken= 0;
            }
            else //entry already initialized
            {
               if(bp_table[bpt_index]->address == pipeline[2]->PC) //entry refers to correct instruction (no collision has occured)
               {
                  //bad prediction: table says taken but branch is not taken, or table says not taken but branch is taken
                  if(((pipeline[2]->Addr != pipeline[1]->PC) && bp_table[bpt_index]->taken) || ((pipeline[2]->Addr == pipeline[1]->PC) && !bp_table[bpt_index]->taken))
                  {
                     bp_table[bpt_index]->taken = !bp_table[bpt_index]->taken; //flips entry's taken flag

                     temp1 = pipeline[0]; //removes instructions from pipeline
                     temp2 = pipeline[1];

                     pipeline[1] = no_op_initializer(); //inserts bubble (squashes instructions)
                     pipeline[0] = no_op_initializer();

                     squashing = 2; //sets flags
                     buffer_skip = 1;
                  }
               }
               else //collision has occured (assume not taken)
               {
                  bp_table[bpt_index]->address= pipeline[2]->PC; //replaces old entry (hash collision)

                  if(pipeline[2]->Addr == pipeline[1]->PC) //branch taken
                  {
                     bp_table[bpt_index]->taken= 1;

                     temp1 = pipeline[0]; //removes instructions from pipeline
                     temp2 = pipeline[1];

                     pipeline[1] = no_op_initializer(); //inserts bubble (squashes instructions)
                     pipeline[0] = no_op_initializer();

                     squashing = 2; //sets flags
                     buffer_skip = 1;
                  }
                  else //branch not taken
                     bp_table[bpt_index]->taken= 0;
               }
            }
         }
      }

      cycle_number++;
      if (trace_view_on) {/* print the executed instruction if trace_view_on=1 */
         if(cycle_number<pipeline_size) {
            printf("[cycle %d] No output, pipeline filling.\n", cycle_number);
         }
         else if(pipeline[4]!=NULL) {
            switch(pipeline[4]->type) {
               case ti_NOP:
                  free(pipeline[4]);
                  printf("[cycle %d] NOP:\n",cycle_number) ;
                  break;
               case ti_RTYPE:
                  printf("[cycle %d] RTYPE:",cycle_number) ;
                  printf(" (PC: %x)(sReg_a: %d)(sReg_b: %d)(dReg: %d)\n", pipeline[4]->PC, pipeline[4]->sReg_a, pipeline[4]->sReg_b, pipeline[4]->dReg);
                  break;
               case ti_ITYPE:
                  printf("[cycle %d] ITYPE:",cycle_number) ;
                  printf(" (PC: %x)(sReg_a: %d)(dReg: %d)(addr: %x)\n", pipeline[4]->PC, pipeline[4]->sReg_a, pipeline[4]->dReg, pipeline[4]->Addr);
                  break;
               case ti_LOAD:
                  printf("[cycle %d] LOAD:",cycle_number) ;
                  printf(" (PC: %x)(sReg_a: %d)(dReg: %d)(addr: %x)\n", pipeline[4]->PC, pipeline[4]->sReg_a, pipeline[4]->dReg, pipeline[4]->Addr);
                  break;
               case ti_STORE:
                  printf("[cycle %d] STORE:",cycle_number) ;
                  printf(" (PC: %x)(sReg_a: %d)(sReg_b: %d)(addr: %x)\n", pipeline[4]->PC, pipeline[4]->sReg_a, pipeline[4]->sReg_b, pipeline[4]->Addr);
                  break;
               case ti_BRANCH:
                  printf("[cycle %d] BRANCH:",cycle_number) ;
                  printf(" (PC: %x)(sReg_a: %d)(sReg_b: %d)(addr: %x)\n", pipeline[4]->PC, pipeline[4]->sReg_a, pipeline[4]->sReg_b, pipeline[4]->Addr);
                  break;
               case ti_JTYPE:
                  printf("[cycle %d] JTYPE:",cycle_number) ;
                  printf(" (PC: %x)(addr: %x)\n", pipeline[4]->PC,pipeline[4]->Addr);
                  break;
               case ti_SPECIAL:
                  printf("[cycle %d] SPECIAL:\n",cycle_number) ;
                  break;
               case ti_JRTYPE:
                  printf("[cycle %d] JRTYPE:",cycle_number) ;
                  printf(" (PC: %x) (sReg_a: %d)(addr: %x)\n", pipeline[4]->PC, pipeline[4]->dReg, pipeline[4]->Addr);
                  break;
            }
         }
      }
   }

   // Uninitialize the trace buffer
   trace_uninit();

   exit(0);
}

struct trace_item * no_op_initializer()
{
   struct trace_item *no_op;

   //allocates memory for no-op
   no_op = (struct trace_item *)malloc(sizeof(struct trace_item));

   //sets dummy values for no-op
   no_op->type   = 0;
   no_op->sReg_a = 255;
   no_op->sReg_b = 255;
   no_op->dReg   = 255;
   no_op->PC     = 0;
   no_op->Addr   = 0;

   return(no_op);
}
