# Pipeline Performance Analysis
### Authors: John Dott (jkd28) and Jeremy Manin (jmmanin)

## Description
This project was designed to give us a better understanding of a pipelined processor. Given a program that simulated a single-cycle CPU, we modified it such that the program would simulate a pipelined processor.  

This simulation would identified data and structural hazards in execution traces and handle them accordingly.  We were also required to implement a Branch Prediction Table and simulate executions with branch prediction operations. Additionally, we simulated a superscalar pipelined processor to compare performance with a normal pipelined architecture.

## Performance Analysis
As part of the project, we were given several example execution traces. We compared these executions in terms of number of cycles required for execution.  This allowed us to examine the effects of branch prediction and multiple pipelines on different programs.  Below is a "prettier" version of the `analysis.txt` file, which we submitted for our assignment. The percentages are derived from comparisons to the unmodified `CPU.c` that we were provided at the beginning of the project.  

**Branch Predictor with Single Pipeline**  

|Trace File	      | Percent Decrease in Cycles with BPT |
|-----------------|-------------------------------------|
|sample.tr        | 3.43%                               |
|sample1.tr       | 14.55%                              |
|sample2.tr       | 2.92%                               |  
|sample3.tr       | 1.31%                               |  
|sample4.tr       | 6.25%                               |  
|sample_long1.tr  | 7.60%                               |  
|sample_long2.tr  | 5.42%                               |  

*Mean Decrease*: 5.93%  
*Standard Deviation*: 4.36%  

The addition of a branch predictor on the single pipeline CPU lead to a statistically significant decrease in the number of cycles taken. This difference differs greatly depending on the code running through the CPU (as shown by the large standard deviation above). This large deviation is most likely due to differences in the order of instructions across the different traces. In fact, it is entirely possible that a branch predictor could lead to an increase in cycles. This issue can be mitigated and the effect of the branch predictor magnified by the inclusion of a second "taken" bit, the addition of added intelligence to the compiler, or by moving the stage branches are resolved in from EX to an earlier stage.


**Two Pipelines (Superscalar)**  

|Trace File       | Percent Decrease in Cycles Over Single Pipeline |
|-----------------|-------------------------------------------------|
|sample.tr        | 17.08%                                          |
|sample1.tr       | 16.50%                                          |
|sample2.tr       | 18.91%                                          |
|sample3.tr       | 20.01%                                          |
|sample4.tr       | 18.49%                                          |
|sample_long1.tr  | 20.25%                                          |
|sample_long2.tr  | 17.73%                                          |

*Mean Decrease*: 18.42%  
*Standard Deviation*: 1.31%  

The addition of a second pipeline lead to very large decrease in cycles. This decrease is around 4 times that caused by the addition of a branch prediction table, and is caused in spite of a noticeable increase in the number of no-ops inserted. While, the dynamic scheduler adds many no-ops, owing to the constraints of the superscalar design, the fact that two instructions can be grouped together and progress through the pipeline concurrently greatly impacts cycles taken. Steps that can be take to further increase the effect of the second pipeline include using a more intelligent compiler and adding addition hardware to mitigate the constraints of the superscalar design.


**Two Pipelines (Superscalar) with Branch Predictor**

| Trace File     | Percent Decrease in Cycles with BPT |
|----------------|-----------------------------------|
|sample.tr       | 1.88%                             |
|sample1.tr      | 9.68%                             |
|sample2.tr      | 1.40%                             |
|sample3.tr      | 2.14%                             |
|sample4.tr      | 4.50%                             |
|sample_long1.tr | 4.20%                             |
|sample_long2.tr | 6.28%                             |

*Mean Decrease*: 4.30%  
*Standard Deviation*: 2.72%  

As expected, the addition of a branch prediction table in the superscalar CPU also lead to a statistically significant decrease in cycles. A slight decrease in the mean and a approximately 50% decrease in standard deviation can be explained by inefficiencies introduced by the dynamic scheduler. As stated before, the effect of the branch predictor can be made more significant by the inclusion of a second "taken" bit, the addition of added intelligence to the compiler, or by moving the stage branches are resolved in from EX to an earlier stage.
