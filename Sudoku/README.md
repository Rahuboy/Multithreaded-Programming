<!-- Overview: -->
The 2 C Files read input from a file (named 'input.txt' and present in the same directory), create the required number of threads, and 
generate a logfile called 'output.txt' containing information about the validity of the rows, columns and grids of an NxN sudoku. The time taken for
computation is also logged


Input File Format: \
`K N` 
(Sudoku) \
--end-- (not part of file) \
K is the number of threads, N is the dimension of the Sudoku (NxN)
The Sudoku is given as N space separated lists of N numbers. An example is: \
``` 1 2 3 4 
 3 4 1 2 
 2 3 4 1 
 4 1 2 3 
 ```

Execution: \ 
(On Linux) 
1) pthreads:
    - To compile: `gcc pthread.c -pthread -lm`
    - To execute: `./a.out`
2) OpenMP:
    - To compile: `gcc -fopenmp OpenMP.c -lm`
    - To execute: `./a.out`

Note:- No flag is required for linking the math library (-lm) or pthreads (-pthread) on MacOS. Running OpenMP might require the 
installation of gcc.


Output: \ 
The code will generate the file 'output.txt'. It contains:
1) The indexed columns, with their validity
2) The indexed rows, with their validity
3) The indexed grids, with their validity
4) A verdict on whether the sudoku itself is valid or not
5) Time taken for execution.


Early Termination: \ 
Uncomment all the lines with the comment '// UNCOMMENT FOR EARLY TERMINATION' \ 
Comment all the lines with the comment '// COMMENT FOR EARLY TERMINATION' \ 

- pthreads: If an anomaly is found, the global variable 'termcheck' is set to 1. When a thread runs the sudoku runner function, it checks for the value of termcheck, and exits if required.
- OpenMP: The mechanism of early termination in OpenMP is similar. 'termcheck' is incremented if an anomaly is found, and a positive value of termcheck leads to continuing in all iterations of the loop (note that you can't use a break in an OpenMP for loop)

Note: The code was developed on MacOS. There may be some variability in other operating systems.