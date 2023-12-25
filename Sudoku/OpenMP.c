// For linux: gcc -fopenmp OpenMP.c -lm
// For mac: gcc-12 -fopenmp OpenMP.c
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h> 
#include <math.h> //Might need a flag in linux

typedef long long ll;

ll** sudoku;  //  Stores the sudoku
ll* work;  //  Stores the work to be assigned to the threads (column index / row index / grid index)
ll* check;  //  For main thread to check
ll** longercheck;  //  To check which rows, columns and grids are valid. Has 3N lists, each containing (validity, threadnumber)
ll N, K, n;  //  Dimension of sudoku, number of threads, sqrt(N)
// ll termcheck = 0; //UNCOMMENT FOR EARLY TERMINATION

// Function to check if a column is valid
int checkCol(ll colno, ll thread_id)
{
    ll arr[N];
    for(ll i = 0; i < N; i++) arr[i] = 0;
    for(ll i = 0; i < N; i++)
    {
        if(sudoku[i][colno] > N || sudoku[i][colno] <= 0)
        {
            longercheck[colno][0] = 1;
            longercheck[colno][1] = thread_id;
            check[thread_id] = 1;
            return 1;
        }
        arr[sudoku[i][colno] - 1]++;
        if(arr[sudoku[i][colno] - 1] != 1)
        {
            longercheck[colno][0] = 1;
            longercheck[colno][1] = thread_id;
            check[thread_id] = 1;
            return 1;
        }
    }
    longercheck[colno][1] = thread_id;
    return 0;
}

// Function to check if a row is valid
int checkRow(ll rowno, ll thread_id)
{
    ll arr[N];
    for(ll i = 0; i < N; i++) arr[i] = 0;
    for(ll i = 0; i < N; i++)
    {
        if(sudoku[rowno][i] > N || sudoku[rowno][i] <= 0)
        {
            longercheck[N + rowno][0] = 1;
            longercheck[N + rowno][1] = thread_id;
            check[thread_id] = 1;
            return 1;
        }
        arr[sudoku[rowno][i] - 1]++;
        if(arr[sudoku[rowno][i] - 1] != 1)
        {
            longercheck[N + rowno][0] = 1;
            longercheck[N + rowno][1] = thread_id;
            check[thread_id] = 1;
            return 1;
        }
    }
    longercheck[N + rowno][1] = thread_id;
    return 0;
}

// Function to check if a grid is valid
int checkGrid(ll gridno, ll thread_id)
{
    ll coord1 = gridno / n;
    ll coord2 = gridno % n;
    ll arr[N];
    for(ll i = 0; i < N; i++) arr[i] = 0;
    for(ll i = 0; i < n; i++)
    {
        for(ll j = 0; j < n; j++)
        {
            if(sudoku[n*coord1 + i][n*coord2 + j] > N || sudoku[n*coord1 + i][n*coord2 + j] <= 0)
            {
                longercheck[2*N + gridno][0] = 1;
                longercheck[2*N + gridno][1] = thread_id;
                check[thread_id] = 1;
                return 1;
            }
            arr[sudoku[n*coord1 + i][n*coord2 + j] - 1]++;
            if(arr[sudoku[n*coord1 + i][n*coord2 + j] - 1] != 1)
            {
                longercheck[2*N + gridno][0] = 1;
                longercheck[2*N + gridno][1] = thread_id;
                check[thread_id] = 1;
                return 1;
            }
        }
    }
    longercheck[2*N + gridno][1] = thread_id;
    return 0;
}

int main()
{

    struct timeval tv1, tv2;
    gettimeofday(&tv1, NULL); //Function used to compute time

    // Input
    FILE* fp_input;  //  File pointer to input.txt
    fp_input = fopen("input.txt", "r");
    fscanf(fp_input, "%lld %lld", &K, &N);  //  K = number of threads, N = dimension of sudoku (N x N)
    n = (ll)sqrt(N);  //  n = sqrt(N)

    //Allocating memory dynamically
    sudoku = (ll**)calloc(N,sizeof(ll*));
    work = (ll*)calloc(3*N,sizeof(ll));  // N indexes for colno, N for rowno, N for gridno
    check = (ll*)calloc(K,sizeof(ll));  // For main thread to check each thread's validity
    longercheck = (ll**)calloc(3*N,sizeof(ll*));  // To check which rows, columns and grids are valid
    for(ll i = 0; i < N; i++) sudoku[i] = (ll*)calloc(N,sizeof(ll));
    for(ll i = 0; i < 3*N; i++) longercheck[i] = (ll*)calloc(2,sizeof(ll));

    // Input sudoku
    for(ll i = 0; i < N; i++)
    {
        for(ll j = 0; j < N; j++) fscanf(fp_input, "%lld", &sudoku[i][j]);
    }

    //closes file pointer
    fclose(fp_input);

    // Column indices, row indices, grid indices
    for(ll i = 0; i < N; i++) work[i] = i;
    for(ll i = N; i < 2*N; i++) work[i] = i - N;
    for(ll i = 2*N; i < 3*N; i++) work[i] = i - 2*N;

    omp_set_dynamic(0);  //  Disable dynamic teams
    omp_set_num_threads(K);  //  Use K threads for all consecutive parallel regions


    ll thread_id;  //  Stores the thread id
    #pragma omp parallel private(thread_id) shared(sudoku, work, check, longercheck)
    {
        thread_id = omp_get_thread_num();  //  Get the thread id
        #pragma omp for
        for(int i = 0; i < 3*N; i++)
        {
            // if(termcheck) continue; // UNCOMMENT FOR EARLY TERMINATION   
            if(i < N) checkCol(work[i], thread_id); // COMMENT FOR EARLY TERMINATION
            else if(i < 2*N) checkRow(work[i], thread_id); // COMMENT FOR EARLY TERMINATION
            else checkGrid(work[i], thread_id); // COMMENT FOR EARLY TERMINATION
            // int tab = 0; // UNCOMMENT FOR EARLY TERMINATION
            // if(i < N) tab = checkCol(work[i], thread_id); // UNCOMMENT FOR EARLY TERMINATION
            // else if(i < 2*N) tab = checkRow(work[i], thread_id); // UNCOMMENT FOR EARLY TERMINATION
            // else tab = checkGrid(work[i], thread_id); // UNCOMMENT FOR EARLY TERMINATION
            // if(tab == 1) termcheck++; // UNCOMMENT FOR EARLY TERMINATION   
        }
    }

    // if(termcheck) printf("Early termination. Invalid sudoku.\n"); // UNCOMMENT FOR EARLY TERMINATION   
    // else printf("Valid sudoku.\n"); // UNCOMMENT FOR EARLY TERMINATION   
    // return 0; // UNCOMMENT FOR EARLY TERMINATION   

    //Stop the clock after computation
    gettimeofday(&tv2, NULL);
    float time_elapsed = (double)(tv2.tv_usec - tv1.tv_usec)/1000000 + (double)(tv2.tv_sec - tv1.tv_sec);
    printf("Time taken: %f\n", time_elapsed);

    ll tab = 0;
    FILE* fp_output;  //  File pointer to output.txt
    fp_output = fopen("output.txt", "w");
    for(ll i = 0; i < 3*N; i++)
    {
        if(i < N)
        {
            if(longercheck[i][0] == 0) fprintf(fp_output, "Thread %lld checks column %lld and it is valid\n", longercheck[i][1], work[i]);
            else
            {
                fprintf(fp_output, "Thread %lld checks column %lld and it is invalid\n", longercheck[i][1], work[i]);
                tab = 1;
            }
        }
        else if(i < 2*N)
        {
            if(longercheck[i][0] == 0) fprintf(fp_output, "Thread %lld checks row %lld and it is valid\n", longercheck[i][1], work[i]);
            else
            {
                fprintf(fp_output, "Thread %lld checks row %lld and it is invalid\n", longercheck[i][1], work[i]);
                tab = 1;
            } 
        }
        else
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
    fprintf(fp_output, "Time taken: %f ms\n", time_elapsed*1000);  // Time in milliseconds (ms

    fclose(fp_output);

    // Free all memory
    for(ll i = 0; i < N; i++) free(sudoku[i]);
    free(sudoku);
    free(work);
    free(check);
    free(longercheck);

    return 0;
}