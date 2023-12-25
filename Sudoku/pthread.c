// Code by Rahul Ramachandran
// #define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/time.h> 
#include <math.h>  // Needs a flag in linux

typedef long long ll;

ll** sudoku;  //  Stores the sudoku
ll* indices;  //  Stores the thread indices
ll* work;  //  Stores the work to be assigned to the threads (column index / row index / grid index)
ll* threadwork;  //  Stores the ranges of work (from work) for each thread (2K entries, even entries are lower bounds, odd entries are upper bounds)
ll** longercheck;  //  To check which rows, columns and grids are valid. Has 3N lists, each containing (validity, threadnumber)
ll N, K, n;  //  Dimension of sudoku, number of threads, sqrt(N)
// pthread_t * threads; // UNCOMMENT FOR EARLY TERMINATION
// ll termcheck; //UNCOMMENT FOR EARLY TERMINATION



// Function to check if a column is valid
int checkCol(ll colno)
{
    ll arr[N]; // Bit array to check if a number is present in the column
    for(ll i = 0; i < N; i++) arr[i] = 0;
    for(ll i = 0; i < N; i++)
    {
        if(sudoku[i][colno] > N || sudoku[i][colno] <= 0) return 1; // If the number is not in the range 1 to N, the column is invalid
        arr[sudoku[i][colno] - 1]++;
        if(arr[sudoku[i][colno] - 1] != 1) return 1; // If the number is repeated, the column is invalid
    }
    return 0;
}

// Function to check if a row is valid
int checkRow(ll rowno)
{
    ll arr[N]; // Bit array to check if a number is present in the row
    for(ll i = 0; i < N; i++) arr[i] = 0;
    for(ll i = 0; i < N; i++)
    {
        if(sudoku[rowno][i] > N || sudoku[rowno][i] <= 0) return 1; // If the number is not in the range 1 to N, the row is invalid
        arr[sudoku[rowno][i] - 1]++;
        if(arr[sudoku[rowno][i] - 1] != 1) return 1; // If the number is repeated, the row is invalid
    }
    return 0;
}

// Function to check if a grid is valid
int checkGrid(ll gridno)
{
    ll coord1 = gridno / n; // The grid number is converted to a coordinate (grid values increase from left to right (by 1), top to bottom)
    ll coord2 = gridno % n;
    ll arr[N]; // Bit array to check if a number is present in the grid
    for(ll i = 0; i < N; i++) arr[i] = 0;
    for(ll i = 0; i < n; i++)
    {
        for(ll j = 0; j < n; j++)
        {
            if(sudoku[n*coord1 + i][n*coord2 + j] > N || sudoku[n*coord1 + i][n*coord2 + j] <= 0) return 1; // If the number is not in the range 1 to N, the grid is invalid
            arr[sudoku[n*coord1 + i][n*coord2 + j] - 1]++;
            if(arr[sudoku[n*coord1 + i][n*coord2 + j] - 1] != 1) return 1; // If the number is repeated, the grid is invalid
        }
    }
    return 0;
}

// Function run by each thread. Using checkCol, checkRow and checkGrid, it checks if the sudoku is valid
void* SudokuChecker(void* index)
{
    ll* ind = (ll*)index;
    ll lowerindex = threadwork[*ind*2];
    ll upperindex = threadwork[*ind*2+1];
    int tab = lowerindex;
    for(; tab < N && tab <= upperindex; tab++)
    {
        // if(termcheck) pthread_exit(0); // UNCOMMENT FOR EARLY TERMINATION
        if(!checkCol(work[tab]))
        {
            longercheck[tab][0] = 0;
            longercheck[tab][1] = *ind;
        }
        else
        {
            longercheck[tab][0] = 1;
            longercheck[tab][1] = *ind;
            // termcheck = 1; // UNCOMMENT FOR EARLY TERMINATION
            // pthread_exit(0); //UNCOMMENT FOR EARLY TERMINATION
        }
    }
    for(; tab < 2*N && tab <= upperindex; tab++)
    {
        if(!checkRow(work[tab]))
        {
            longercheck[tab][0] = 0;
            longercheck[tab][1] = *ind;
        }
        else
        {
            longercheck[tab][0] = 1;
            longercheck[tab][1] = *ind;
            // termcheck = 1; // UNCOMMENT FOR EARLY TERMINATION
            // pthread_exit(0); //UNCOMMENT FOR EARLY TERMINATION

        }
    }
    for(; tab < 3*N && tab <= upperindex; tab++)
    {
        if(!checkGrid(work[tab]))
        {
            longercheck[tab][0] = 0;
            longercheck[tab][1] = *ind;
        }
        else
        {
            longercheck[tab][0] = 1;
            longercheck[tab][1] = *ind;
            // termcheck = 1; // UNCOMMENT FOR EARLY TERMINATION
            // pthread_exit(0); //UNCOMMENT FOR EARLY TERMINATION
        }
    }
    pthread_exit(0);
}


int main()
{
    
    struct timeval tv1, tv2;
    gettimeofday(&tv1, NULL); //Function used to compute time
    
    FILE* fp_input;  //  File pointer to input.txt
    fp_input = fopen("input.txt", "r");
    fscanf(fp_input, "%lld %lld", &K, &N);  //  K = number of threads, N = dimension of sudoku (N x N)
    n = (ll)sqrt(N);  //  n = sqrt(N)

    //Allocating memory dynamically
    sudoku = (ll**)calloc(N,sizeof(ll*));
    work = (ll*)calloc(3*N,sizeof(ll));  // N indexes for colno, N for rowno, N for gridno
    threadwork = (ll*)calloc(2*K,sizeof(ll*));  // 2K entries, even entries are lower bounds, odd entries are upper bounds
    longercheck = (ll**)calloc(3*N,sizeof(ll*));  // To check which rows, columns and grids are valid
    indices = (ll*)calloc(K,sizeof(ll));  // Stores the thread indices (0 to K-1)
    // threads = (pthread_t*)calloc(K,sizeof(pthread_t)); //UNCOMMENT FOR EARLY TERMINATION
    // termcheck = 0; //UNCOMMENT FOR EARLY TERMINATION
    for(ll i = 0; i < N; i++) sudoku[i] = (ll*)calloc(N,sizeof(ll));
    for(ll i = 0; i < 3*N; i++) longercheck[i] = (ll*)calloc(2,sizeof(ll));  // Each list has 2 entries, validity and thread number

    
    for(ll i = 0; i < K; i++) indices[i] = i;  // Assigning indices to threads
    for(ll i = 0; i < N; i++)
    {
       for(ll j = 0; j < N; j++) fscanf(fp_input, "%lld", &sudoku[i][j]);  // Reading sudoku from input.txt
    }

    //closes file pointer
    fclose(fp_input);
    
    ll nwork[K];  // Stores the number of work each thread has to do (1 unit = 1 row, column or grid)
    ll remainder = 3*N - ((3*N)/K)*(K);  // The variable remainder is used to divide the no. of random numbers as equitably as possible.

    for(ll i = 0; i < K; i++)
    {
         // Divide the work between the threads
        if(remainder > 0)
        {
            nwork[i] = (3*N/K) + 1;
            remainder--;
        }
        else  nwork[i] = (3*N/K);
    }
    

     // Assigning lower and upper bounds for each thread using nwork
    for(ll i = 0; i < 2*K; i+=2)
    {
        if(i == 0) threadwork[i] = 0;
        else threadwork[i] = threadwork[i-1] + 1;
        threadwork[i+1] = threadwork[i]+nwork[i/2]-1;   
    }
    
    

     //  Column indices, row indices, grid indices
    for(ll i = 0; i < N; i++) work[i] = i;
    for(ll i = N; i < 2*N; i++) work[i] = i - N;
    for(ll i = 2*N; i < 3*N; i++) work[i] = i - 2*N;
    

    pthread_t threads[K]; // COMMENT FOR EARLY TERMINATION
     // Create the threads
    for(ll i = 0; i < K; i++) pthread_create(&threads[i], NULL, &SudokuChecker, (void *)&indices[i]);
     // Wait for the threads to stop executing
    for(ll i = 0; i < K; i++) pthread_join(threads[i], NULL);


    // if(termcheck == 1) printf("Terminated early. Invalid Sudoku\n"); // UNCOMMENT FOR EARLY TERMINATION
    // else printf("Valid sudoku\n"); // UNCOMMENT FOR EARLY TERMINATION
    // return 0; // UNCOMMENT FOR EARLY TERMINATION


    //Stop the clock after computation
    gettimeofday(&tv2, NULL);
    float time_elapsed = (double)(tv2.tv_usec - tv1.tv_usec)/1000000 + (double)(tv2.tv_sec - tv1.tv_sec); 
    printf("Time taken: %f\n", time_elapsed);

    ll tab = 0;
    FILE* fp_output;  //  File pointer to output.txt
    fp_output = fopen("output.txt", "w");
    
    for(ll i = 0; i < 3*N; i++) // Prints log of validity of rows, columns and grids
    {
        if(i < N) // Column
        {
            if(longercheck[i][0] == 0) fprintf(fp_output, "Thread %lld checks column %lld and it is valid\n", longercheck[i][1], work[i]);
            else
            {
                fprintf(fp_output, "Thread %lld checks column %lld and it is invalid\n", longercheck[i][1], work[i]);
                tab = 1;
            }
        }
        else if(i < 2*N) // Row
        {
            if(longercheck[i][0] == 0) fprintf(fp_output, "Thread %lld checks row %lld and it is valid\n", longercheck[i][1], work[i]);
            else
            {
                fprintf(fp_output, "Thread %lld checks row %lld and it is invalid\n", longercheck[i][1], work[i]);
                tab = 1;
            } 
        }
        else // Grid
        {
            if(longercheck[i][0] == 0) fprintf(fp_output, "Thread %lld checks grid %lld and it is valid\n", longercheck[i][1], work[i]);
            else
            {
                fprintf(fp_output, "Thread %lld checks grid %lld and it is invalid\n", longercheck[i][1], work[i]);
                tab = 1;
            } 
        }
    }
    if(tab == 0) fprintf(fp_output, "Sudoku is valid\n");
    else fprintf(fp_output, "Sudoku is invalid\n");
    fprintf(fp_output, "Time taken: %f ms\n", time_elapsed*1000);  // Time in milliseconds (ms)

    fclose(fp_output);

    //Freeing memory
    for(ll i = 0; i < N; i++) free(sudoku[i]);
    free(sudoku);
    free(work);
    free(threadwork);
    free(indices);
    return 0;
}