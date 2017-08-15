#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define GRID_UNIT 3
#define GRID_SIZE (GRID_UNIT * GRID_UNIT)

int grid[GRID_SIZE][GRID_SIZE] = {

	{6,5,3,1,2,8,7,9,4},
	{1,7,4,3,5,9,6,8,2},
	{9,2,8,4,6,7,5,3,1},
	{2,8,6,5,1,4,3,7,9},
	{3,9,1,7,8,2,4,5,6},
	{5,4,7,6,9,3,2,1,8},
	{8,6,5,2,3,1,9,4,7},
	{4,1,2,9,7,5,8,6,3},
	{7,3,9,8,4,6,1,2,5}

};

// structure for passing data to threads 
typedef struct
{
	int row;
	int col;
}
parameters;

void *validate(void *param); // function declaration/header

// shared data (global variables), shared by all threads
int valid[3*GRID_SIZE]; // grid, row, col

int main()
{
	// allocate worker thread structures
	pthread_t row_tid[GRID_SIZE],
			  col_tid[GRID_SIZE],
			  blk_tid[GRID_SIZE];

	// save their parameter structure pointers to free when each worker exits
	parameters *row_param[GRID_SIZE],
			   *col_param[GRID_SIZE],
			   *blk_param[GRID_SIZE];

	// allocate shared pthread attribute structure
	pthread_attr_t attr;

	// initialize pthread attributes
	pthread_attr_init(&attr);

	printf("CS149 Sudoku from Yehia Qtaish\n");
	for(int y=0; y<GRID_SIZE; y++){ //prints out grid looping through index (y,z)
		for(int z=0; z<GRID_SIZE; z++){
			printf("%d|", grid[y][z]);
		}
		printf("\n");
	}
	fflush(stdout);
	int k;
	parameters *data;
	for (k=0; k<GRID_SIZE; k++)
	{
		// create worker thread to check row (with index) k
		data = (parameters *) malloc(sizeof(parameters));
		data->row = k+1;
		data->col = 0;
		// creates the worker thread passing it data as a parameter 
		pthread_create(&row_tid[k],&attr,validate,data);
		row_param[k] = data; // save pointer to allocated memory to free later

		// create worker thread to check column (with index) k
		data = (parameters *) malloc(sizeof(parameters));
		data->col = k+1;
		data->row = 0;
		// creates the worker thread passing it data as a parameter 
		pthread_create(&col_tid[k],&attr,validate,data);
		col_param[k] = data; // save pointer to allocated memory to free later

		// create worker thread to check block (with index) k
		data = (parameters *) malloc(sizeof(parameters));
		data->row = 1 + (k / GRID_UNIT); // integer division to get row
		data->col = 1 + (k % GRID_UNIT); // modular remainder to get col
		// create the worker thread passing it data as a parameter 
		pthread_create(&blk_tid[k],&attr,validate,data);
		blk_param[k] = data; // save pointer to allocated memory to free later
	}

	for (k=0; k<GRID_SIZE; k++)
	{
		// wait for row, column & block k threads to complete and free their param
		pthread_join(row_tid[k],NULL);
		free(row_param[k]);
		
		pthread_join(col_tid[k],NULL);
		free(col_param[k]);

		pthread_join(blk_tid[k],NULL);
		free(blk_param[k]);
	}

	// check thread results (checklist) to test validity
	// re-use int k...
	for (k=3*GRID_SIZE; k-- > 0 && valid[k]; );
	
	printf("Grid is in %s state.\n", k<0 ? "a valid" : "an invalid");
}

// function validate's implementation
// arg points to a parameters structure, *param.
void *validate(void *arg)
{
	parameters *param = (parameters *)arg;

	int row = param->row,
		col = param->col;

	//the distinct sum to compare and validate rows/blocks with 
	const int distinct_sum = GRID_SIZE * (GRID_SIZE + 1) / 2;

	int i, j, sum;

	if (row && col) // validate block (row,col)
	{
		int u = GRID_UNIT * (--row); // starting row index of this block
		int v = GRID_UNIT * (--col); // starting col index of this block
		for (sum=i=0; i<GRID_UNIT; i++) {//loops to get the block index
			for (j=0; j<GRID_UNIT; j++) {
				sum += grid[u+i][v+j];
			}
		}
		valid[row*GRID_UNIT+col] = (sum == distinct_sum)? 1: 0;
	}
	else if (row) // validate row (row)
	{
		row--; // convert row (ordinal) number to index
		for (sum=j=0; j<GRID_SIZE; j++) sum+=grid[row][j];
		valid[GRID_SIZE+row] = (sum == distinct_sum)? 1: 0;
	}
	else // validate col (col)
	{
		col--; // convert col (ordinal) number to index
		for (sum=i=0; i<GRID_SIZE; i++) sum+=grid[i][col];
		valid[2*GRID_SIZE+col] = (sum == distinct_sum)? 1: 0;
	}

	pthread_exit(0);
}

