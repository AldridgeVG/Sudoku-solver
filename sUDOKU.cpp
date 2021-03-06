// sUDOKU.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "string.h"
#include "malloc.h"
#include "stdLib.h"
#include "io.h"
#include "time.h"
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE
#pragma warning(disable:4996)
#define SATISFIABLE 1
#define UNSATIFIABLE -1
#define UNCERTAIN 0

#define DEBUG 0

int Sudoku[10][10];								//sudoku structure as 3 dimension array
int matrix[10][10];								//Matrix array to output sudoku
int KNOWN[10][10];								//known sudoku terminal file to generate a game
char ORIGINALFINAL[100] = "SudokuRemain0.txt";

int vnum = 0;									//in sudoku, 729 variables
int cnum = 0;
int debug = 0;
int *current;
int *valuation;									//valuation of a variable, all 729
#define MAXOP 6
#define MAXVNUM 10000							//max vnum to include special situation
												//e.g. in Sudoku, vnum is not consecutive

//Sudoku func
struct lit										//lit def, for creating sudoku sat
{
	int literal;
	int status;
	struct lit *nextlit;
};
struct Clause									//clause and literal as cross linked list
{
	lit *Lit;
	struct Clause *nextclause;
};

//SAT DPLL func
struct Literal									//Literal def, for DPLL
{
	int index;
	struct Literal *next;
};
struct Clause1									//clause and literal as cross linked list
{
	Literal *Lit;
	struct Clause1 *nextclause;
};

//sudoku to SATinCNF section
int SudokuInput(int p[10][10]);
int SudokuFileInput(char *filename, int p[10][10]);
int SudokuOutput(char *filename);
int SudokuFileOutput(int p[10][10]);
int SudokuFileOutput1(int p[10][10], char*internal);
int SudokuInitialize(int p[10][10]);
int SudokuSAT(Clause* p1, int q[10][10]);
int CreateCNF(Clause* p1);
int CreateCNF1(Clause *p1, char *internal2);
int WriteSudokuSolution(char * filename);

//Sudoku Game section
int GenerateRandomSudoku(int e, char *filename);

//DPLL solver section
int DPLL(Clause1 *C);
//basic f
Clause1* ReadCNF(char *filename);
int PrintCNF(Clause1 *C);
int WriteSolution(char * filename);
void PrintClause(Clause1 *C);
Clause1* AddClause(Clause1 *C);
Clause1* CloneClause(Clause1 *C);
void RemoveLiteral(Literal *L);
void RemoveClause(Clause1 *C);
//function f
int Find_NEXT_UnitClause(Clause1 *C);
int Find_Pure_Literal(Clause1 *C);
int UnitPropagation(Clause1 *C);
int PL_Elimination(Clause1 *C);
int ChooseLiteral(Clause1 *C);
int ChooseLiteral2(Clause1 *C);
Clause1* Branch(Clause1 *C, int e);
//checking f
int IfContainEmptyClause(Clause1 *C);
int IfAllUnitClause(Clause1 *C);
int Check(Clause1 *C);
//to perform if the index of lit is pos or neg
int sign(int num) 
{
	return (num > 0) - (num < 0);
}				

//----------------------------------------------------------------------------------//

//MAIN F//

int main()
{
	int op = 1;
	SudokuInitialize(Sudoku);
	printf("\nSudoku Initialization complete \n\n");
	Clause *head = NULL;
	Clause1 *present = NULL;
	head = (Clause*)malloc(sizeof(Clause));
	head->nextclause = NULL;
	head->Lit = NULL;
	char CNFfilename[100];
	char Solutionfilename[100];

	while (op >= 1 && op <= MAXOP)
	{
		system("cls");
		printf("\n\n");
		printf("------------------	\n\n");
		printf(" Macrohard Sudoku 	\n\n");
		printf("------------------	\n\n");
		printf("\n\nSelect op: \n\n");
		printf("1. input a Sudoku\n");
		printf("2. select Sudoku MAT file\n");
		printf("3. select Sudoku CNF file\n");
		printf("4. (test) SAT DPLL\n");
		printf("5. (test) Sudoku output\n");
		printf("6. Sudoku Game");
		do
		{
			printf("plz select valid op num (n to quit):\n");
			if (!scanf("%d", &op))
			{
				printf("system quit\n");
				printf("nexttime!\n");
				return 0;
			}
		} while (op < 1 || op > MAXOP);
		switch (op)
		{
		case 1:
		{
			SudokuInput(Sudoku);
			SudokuSAT(head, Sudoku);
			CreateCNF(head);
			getchar();
			getchar();
			break;
		}
		case 2:
		{
			printf("plz input a Sudoku Matrix filename:\n");
			scanf("%s", &CNFfilename);
			SudokuFileInput(CNFfilename, Sudoku);
			SudokuSAT(head, Sudoku);
			CreateCNF(head);
			getchar();
			getchar();
			break;
		}
		case 3:
		{
			int c;
			printf("plz enter a Sudoku CNF filename:\n");
			scanf("%s", CNFfilename);
			present = ReadCNF(CNFfilename);

			clock_t start = 0, end = 0;		//starttime and end time
			double t;						//Time Duration
			start = clock();

			int judge = DPLL(present);		//only record dpll time

			end = clock();
			t = (double)(end - start);
			t = t / 1000;					//covert to second

			if (judge == SATISFIABLE)
			{
				printf("satisfiable!\n");
				printf("Time used: %f (s)\n", t);
				printf("where do you want to output the solution?:\n");
				scanf("%s", Solutionfilename);
				WriteSudokuSolution(Solutionfilename);

				printf("output Sudoku Matrix?(1/0)\n");
				scanf("%d", &c);
				if (c == 0)
				{
					getchar();
					getchar();
					break;
				}
				if (c == 1)
				{
					SudokuOutput(Solutionfilename);
					printf("Do you want to print Sudoku Matrix?(1/0)\n");
					scanf("%d", &c);
					if (c == 0)
					{
						getchar();
						getchar();
						break;
					}
					if (c == 1)
					{
						if (SudokuFileOutput(matrix))
							printf("Output Successed\n");
						else
							printf("Fail to output");
						getchar();
						getchar();
						break;
					}
					else
					{
						getchar();
						getchar();
						break;
					}
				}
				else
				{
					getchar();
					getchar();
					break;
				}
			}
			else
			{
				printf("unsatisfiable\n");
			}
			getchar();
			getchar();
			break;
		}
		case 4:
		{
			printf("plz enter a CNF filename:\n");
			scanf("%s", CNFfilename);
			present = ReadCNF(CNFfilename);
			PrintCNF(present);

			clock_t start = 0, end = 0;		//starttime and end time
			double t;						//Time Duration
			start = clock();

			int judge = DPLL(present);		//only record dpll time

			end = clock();
			t = (double)(end - start);
			t = t / 1000;					//covert to second

			if (judge == SATISFIABLE)
			{
				printf("satisfiable!\n");
				printf("Time used: %f (s)\n", t);
				printf("where do you want to output the solution?:\n");
				scanf("%s", Solutionfilename);
				WriteSolution(Solutionfilename);
			}
			else
			{
				printf("unsatisfiable\n");
			}
			getchar();
			getchar();
			break;
		}
		case 5:
		{
			printf("plz input a sudoku solution test filename:\n");
			scanf("%s", Solutionfilename);
			SudokuOutput(Solutionfilename);
			getchar();
			getchar();
			break;
		}
		case 6:
		{
			int blanknum;
			printf("Greetings, let's game!\n");
			printf("you want to start a Sudoku with  blanks of ____ \n\n");
			scanf("%d", &blanknum);
			GenerateRandomSudoku(blanknum, ORIGINALFINAL);
		}
		}
	}
    return 0;
}

//----------------------------------------------------------------------------------//

//SUDOKU TO SATINCNF SECTION//

int SudokuInitialize(int p[10][10])		//in sudoku, r,c,n all minus 1
{
	int x, y;
	for (x = 0; x <= 9; x++)
	{
		for (y = 0; y <= 9; y++)
			p[x][y] = 0;
	}
	return 0;
}

//mannully input a Sudoku puzzle
int SudokuInput(int p[10][10])
{
	printf("plz enter row, cloumn and value of known-value cube:(-1 to complete)\n");
	SudokuInitialize(p);
	int c, x, y, i = 0;

	do {								//input
		i++;
		do {
			printf("please input valid row and column:\n");
			scanf_s("%d %d", &x, &y);
			c = x;
		} while ((x < 1 || x > 9 || y < 1 || y > 9) && (x != -1));
		do {
			printf("please input valid number of cube:\n");
			scanf_s("%d", &p[x][y]);
		} while (p[x][y] < 1 || p[x][y] > 9);
	} while (c != -1);
	
	for (x = 1; x <= 9; x++)			//print whole sudoku for checking
	{
		for (y = 1; y <= 9; y++)
				printf("%d\t", p[x][y]);
		printf("\n\n");
	}
	printf("this is your sudoku, number of known numbers: %d\n\n", i - 1);
	return 0;
}

//input a sudoku from current matrix file
int SudokuFileInput(char* filename, int p[10][10])
{
	FILE *fp;
	fp = fopen(filename, "r");

	SudokuInitialize(p);
	int x, y;
	for (x = 1; x <= 9; x++)			//fscan whole sudoku for inputing every number
	{
		for (y = 1; y <= 9; y++)
		{
			fscanf(fp, "%d", &p[x][y]);
		}
	}

	for (x = 1; x <= 9; x++)			//print whole sudoku for checking
	{
		for (y = 1; y <= 9; y++)
			printf("%d\t", p[x][y]);
		printf("\n\n");
	}
	printf("this is your sudoku:\n");
	fclose(fp);
	return 0;
}

//Write a Sudoku According to a solution of CNF
int SudokuOutput(char *filename)
{
	memset(matrix, 0, sizeof(matrix));
	int num, flag;
	int x, y;

	FILE *fp;
	char line[256];
	fp = fopen(filename, "r");
	if (fp == NULL) exit(1);
	while (fgets(line, sizeof(line), fp))
	{
		sscanf(line, "%d %d", &num, &flag); //if xyn flag1, means row x column y is n
		if (DEBUG) printf("%d %d\n", num, flag);
		if (flag == 1)
		{
			x = num / 100;
			if (DEBUG) printf("x = %d\n", x);
			y = (num - 100 * x) / 10;
			if (DEBUG) printf("y = %d\n", y);
			matrix[x][y] = (num - 100 * x - 10 * y);
			if (DEBUG) printf("value = %d\n", matrix[x][y]);
		}
		if (flag == 0)
		{
			if (DEBUG) printf("\n");
			continue;
		}
	}

	for (x = 1; x <= 9; x++)				//print whole sudoku for checking
	{
		for (y = 1; y <= 9; y++)
			printf("%d\t", matrix[x][y]);
		printf("\n\n");
	}
	printf("this is your sudoku\n");
	fclose(fp);
	return 0;
}

//Output sudoku matrix 
int SudokuFileOutput(int p[10][10])
{
	FILE *fp;
	char filename[50];
	printf("filename to output: (.sov)\n");
	scanf("%s", filename);
	if ((_access(filename, 0)) == -1)
	{
		printf("file not exist, create or not? y/n\n");
		char ch = getchar();
		while (ch != 'y' && ch != 'n')
			ch = getchar();
		if (ch == 'n')
			return 0;
	}
	fp = fopen(filename, "w");
	int x, y;
	for (x = 1; x <= 9; x++)				//print whole sudoku for checking
	{
		for (y = 1; y <= 9; y++)
			fprintf(fp, "%d\t", p[x][y]);
		fprintf(fp, "\n\n");
	}
	fclose(fp);
	return 1;
}

int SudokuFileOutput1(int p[10][10], char*internal)
{
	FILE *fp;
	fp = fopen(internal, "w");
	int x, y;
	for (x = 1; x <= 9; x++)				//print whole sudoku for checking
	{
		for (y = 1; y <= 9; y++)
			fprintf(fp, "%d\t", p[x][y]);
		fprintf(fp, "\n\n");
	}
	fclose(fp);
	return 1;
}

//perform a Sudoku in SAT
int SudokuSAT(Clause* p1, int q[10][10])	//note that in this sat all 1-9 becomes 0-8(r,c,n of a cube)
{
	Clause *p = p1;
	lit *r = NULL;
	int x, y, z, i, j, k, l;
	int temp1;
	int interlit[10][10][10];
	vnum = 729;
	cnum = 0;

	//transfer known cube into SAT number1, unknown to 0
	for (x = 1; x <= 9; x++)
	{
		for (y = 1; y <= 9; y++)
		{
			i = q[x][y];
			if (i == 0)
				for (z = 1; z <= 9; z++) interlit[x][y][z] = 0;
			if (i != 0)
			{
				for (z = 1; z <= 9; z++) interlit[x][y][z] = 0;

				//set unit clause by known condition
				p->nextclause = (Clause*)malloc(sizeof(Clause));	cnum++;
				p = p->nextclause;
				p->nextclause = NULL;
				p->Lit = (lit*)malloc(sizeof(lit));
				p->Lit->literal = 100 * x + 10 * y + i;
				p->Lit->status = 1;
				p->Lit->nextlit = NULL;				
			}
		}			
	}	
	if (DEBUG) printf("INTERNAL ARRAY set success!\n");

	//RULE 1
	for (x = 1; x <= 9; x++)			//x row*100
	{
		//printf("1 round of x\n");		//format control
		for (y = 1; y <= 9; y++)		//y column*10
		{
			//printf("1 round of y\n");	//format control
			temp1 = 100 * x + 10 * y;	//temp1 to record row and column
			p->nextclause = (Clause*)malloc(sizeof(Clause));	cnum++;
			p = p->nextclause;
			p->nextclause = NULL;
			p->Lit = (lit*)malloc(sizeof(lit));

			r = p->Lit;
			r->literal = 0;
			r->status = 0;
			r->nextlit = NULL;

			for (z = 1; z <= 9; z++)	//add literal to clause
			{
				r->literal = temp1 + z;
				//r->status = interlit[x][y][z];
				r->status = 1;
				r->nextlit = (lit*)malloc(sizeof(lit));
				r = r->nextlit;
				r->nextlit = NULL;
			}
		}								//finish def a clause
	}  
	if (DEBUG) printf("RULE 1 success!\n");

	//RULE 2
	for (y = 1; y <= 9; y++)
	{
		for (z = 1; z <= 9; z++)
		{
			temp1 = 10 * y + z;
			for (x = 1; x <= 8; x++)
			{
				for (i = x + 1; i <= 9; i++)
				{
					p->nextclause = (Clause*)malloc(sizeof(Clause));	cnum++;
					p = p->nextclause;
					p->nextclause = NULL;					//format clause
					p->Lit = (lit*)malloc(sizeof(lit));

					r = p->Lit;
					r->literal = temp1 + 100 * x;
					//r->status = 1 - interlit[x][y][z];
					r->status = 0;
					r->nextlit = (lit*)malloc(sizeof(lit));	//add lit !Sxyz

					r = r->nextlit;
					r->literal = temp1 + 100 * i;
					//r->status = 1 - interlit[i][y][z]; 		//add lit !Siyz
					r->status = 0;
					r->nextlit = NULL;
				}
			}
		}
	}	
	if (DEBUG) printf("RULE 2 success!\n");

	//RULE 3
	for (x = 1; x <= 9; x++)
	{
		for (z = 1; z <= 9; z++)
		{
			temp1 = 100 * x + z;
			for (y = 1; y <= 8; y++)
			{
				for (i = y + 1; i <= 9; i++)
				{
					p->nextclause = (Clause*)malloc(sizeof(Clause));	cnum++;
					p = p->nextclause;
					p->nextclause = NULL;					//format clause
					p->Lit = (lit*)malloc(sizeof(lit));

					r = p->Lit;
					r->literal = temp1 + 10 * y;
					//r->status = 1 - interlit[x][y][z];
					r->status = 0;
					r->nextlit = (lit*)malloc(sizeof(lit));	//add lit !Sxyz

					r = r->nextlit;
					r->literal = temp1 + 10 * i;
					//r->status = 1 - interlit[x][i][z];		//add lit !Siyz
					r->status = 0;
					r->nextlit = NULL;
				}
			}
		}
	}	
	if (DEBUG) printf("RULE 3 success!\n");

	//RULE 4
	for (z = 1; z <= 9; z++)			//cube num z as outer variables
	{
		for (i = 0; i <= 6; i = i + 3)
		{
			for (j = 0; j <= 6; j = j + 3)
			{
				for (x = 1; x <= 3; x++)
				{
					temp1 = 100 * (i + x) + 10 * j + z;
					for (y = 1; y <= 3; y++)
					{
						for (k = y + 1; k <= 3; k++)
						{
							p->nextclause = (Clause*)malloc(sizeof(Clause));	cnum++;
							p = p->nextclause;
							p->nextclause = NULL;						//format clause
							p->Lit = (lit*)malloc(sizeof(lit));

							r = p->Lit;
							r->literal = temp1 + 10 * y;
							//r->status = 1 - interlit[i + x][j + y][z];
							r->status = 0;
							r->nextlit = (lit*)malloc(sizeof(lit));		//add lit !S x+i y+j z

							r = r->nextlit;
							r->literal = temp1 + 10 * k;
							//r->status = 1 - interlit[i + x][j + k][z];		//add lit !S x+i k+j z
							r->status = 0;
							r->nextlit = NULL;
						}
					}
				}
			}
		}
	}   
	if (DEBUG) printf("RULE 4 success!\n");

	//RULE 5
	for (z = 1; z <= 9; z++)			//cube num z as outer variables
	{
		for (i = 0; i <= 6; i = i + 3)
		{
			for (j = 0; j <= 6; j = j + 3)
			{
				for (y = 1; y <= 3; y++)
				{
					temp1 = 100 * i + 10 * j + z;
					for (x = 1; x <= 3; x++)
					{
						for (k = x + 1; k <= 3; k++)
						{
							for (l = 1; l <= 3; l++)
							{
								p->nextclause = (Clause*)malloc(sizeof(Clause));	cnum++;
								p = p->nextclause;
								p->nextclause = NULL;							//format clause
								p->Lit = (lit*)malloc(sizeof(lit));

								r = p->Lit;
								r->literal = temp1 + 100 * x + 10 * y;
								//r->status = 1 - interlit[i + x][j + y][z];
								r->status = 0;
								r->nextlit = (lit*)malloc(sizeof(lit));			//add lit !S x+i y+j z

								r = r->nextlit;
								r->literal = temp1 + 100 * k + 10 * l;
								//r->status = 1 - interlit[i + k][j + l][z];		//add lit !S k+i l+j z
								r->status = 0;
								r->nextlit = NULL;
							}
						}
					}
				}
			}
		}
	}  
	if (DEBUG) printf("RULE 5 success!\n");
	if (DEBUG) printf("ALL success! Sudoku converted to SAT\n");
	return 1;
}

//create CNF of a SudokuSAT
int CreateCNF(Clause *p1)				//transform to CNF
{
	Clause *p = p1;
	lit *r = NULL;

	//print all NF on screen
	/*
	printf("c This is a Sudoku in SATproblem form as a CNF\n\n");
	printf("p cnf %d %d\n\n", vnum, cnum);
	p = p1->nextclause;
	while (p != NULL)
	{
		r = p->Lit;
		while (r != NULL)
		{
			if (r->status == 1)
			printf("%d ", r->literal);
			if (r->status == 0)
			printf("%d ",-(r->literal));
			r = r->nextlit;
		}
		printf("0");
		p = p->nextclause;
		printf("\n");
	}
	*/

	FILE *fp;
	char filename[50];
	printf("filename to output: (.cnf)\n");
	scanf("%s", filename);
	if ((_access(filename, 0)) == -1)
	{
		printf("file not exist, create or not? y/n\n");
		char ch = getchar();
		while (ch != 'y' && ch != 'n')
			ch = getchar();
		if (ch == 'n')
			return 0;
	}
	fp = fopen(filename, "w");
	fprintf(fp, "c This is a Sudoku in SATproblem form as a CNF\n");
	fprintf(fp, "p cnf %d %d\n", vnum, cnum);

	//print all NF
	p = p1->nextclause;										//printf("init success!\n");
	while (p != NULL)
	{
		r = p->Lit;											//printf("lit of cla found!\n");
		while (r != NULL)
		{
			if (r->status == 1)
				fprintf(fp, "%d ", r->literal);				
			if (r->status == 0)
				fprintf(fp, "%d ", - (r->literal));		
			r = r->nextlit;									//printf("printing next lit\n");
		}
		fprintf(fp, "0");
		p = p->nextclause;									//printf("next clause found!\n");
		if (p != NULL)										//avoid a space in file end
			fprintf(fp, "\n");
	}
	fclose(fp);
	
	//free all nodes
	p = p1->nextclause;
	Clause *m; lit *n;
	while (p != NULL)
	{
		r = p->Lit;
		while (r != NULL)
		{
			n = r;
			r = r->nextlit;
			free(n);
		}
		m = p;
		p = p->nextclause;
		free(m);
	}
	printf("free nodes success!\n");
	return 1;
}

int CreateCNF1(Clause *p1, char *internal2)				//transform to CNF
{
	Clause *p = p1;
	lit *r = NULL;
	FILE *fp;
	fp = fopen(internal2, "w");
	fprintf(fp, "c This is a Sudoku in SATproblem form as a CNF\n");
	fprintf(fp, "p cnf %d %d\n", vnum, cnum);

	//print all NF
	p = p1->nextclause;										//printf("init success!\n");
	while (p != NULL)
	{
		r = p->Lit;											//printf("lit of cla found!\n");
		while (r != NULL)
		{
			if (r->status == 1)
				fprintf(fp, "%d ", r->literal);
			if (r->status == 0)
				fprintf(fp, "%d ", -(r->literal));
			r = r->nextlit;									//printf("printing next lit\n");
		}
		fprintf(fp, "0");
		p = p->nextclause;									//printf("next clause found!\n");
		if (p != NULL)										//avoid a space in file end
			fprintf(fp, "\n");
	}
	fclose(fp);

	//free all nodes
	p = p1->nextclause;
	Clause *m; lit *n;
	while (p != NULL)
	{
		r = p->Lit;
		while (r != NULL)
		{
			n = r;
			r = r->nextlit;
			free(n);
		}
		m = p;
		p = p->nextclause;
		free(m);
	}
	return 1;
}

//----------------------------------------------------------------------------------//

//SUDOKU GAME SECTION//

//generate a sudoku with a known file
int GenerateRandomSudoku(int e, char *filename)
{
	FILE *fp;
	fp = fopen(filename, "r");

	SudokuInitialize(KNOWN);

	int x, y, value, i;
	for (x = 1; x <= 9; x++)						//fscan whole sudoku for inputing every number
	{
		for (y = 1; y <= 9; y++)
		{
			fscanf(fp, "%d", &KNOWN[x][y]);			//initialize KNOWN
		}
	}
	fclose(fp);
							
	for (i = 1; i <= e; i++)						//randomize a sudoku
	{	
		do
		{
			x = rand() % 9 + 1;
			y = rand() % 9 + 1;
		} while (KNOWN[x][y] == 0);
		KNOWN[x][y] = 0;
	}

	for (x = 1; x <= 9; x++)						//print whole sudoku for checking
	{
		for (y = 1; y <= 9; y++)
			printf("%d\t", KNOWN[x][y]);
		printf("\n\n");
	}
	printf("this is your sudoku puzzle\n");
	do
	{
		getchar();
		printf("please enter your number[row(1~9), column(1~9) and value(1~9)]:\n\n");
		do {
			printf("enter valid numbers below:\n\n");
			scanf("%d %d %d", &x, &y, &value);
		} while (x < 1 || x>9 || y < 1 || y>9 || value < 1 || value>9);
		printf("your answer %d %d %d is added to the Sudoku\n\n", x, y, value);
		KNOWN[x][y] = value;
		for (x = 1; x <= 9; x++)				//print whole sudoku for checking
		{
			for (y = 1; y <= 9; y++)
				printf("%d\t", KNOWN[x][y]);
			printf("\n\n");
		}
		printf("this is the sudoku now\n");
		getchar();
		getchar();
		printf("\n end inputing?(1/0)\n\n");
		scanf("%d", &i);
	} while (i = 0);
	getchar();
	printf("\nChecking answers\n");
	char internalfn[100] = "tempsudo.txt";
	char internalfn2[100] = "tempcnf.cnf";
	Clause *temphead = NULL;
	temphead = (Clause*)malloc(sizeof(Clause));
	temphead->nextclause = NULL;
	temphead->Lit = NULL;
	Clause1 *temppresent = NULL;
	SudokuFileOutput1(KNOWN, internalfn);
	SudokuFileInput(internalfn, KNOWN);
	SudokuSAT(temphead, KNOWN);
	CreateCNF1(temphead, internalfn2);
	temppresent = ReadCNF(internalfn2);
	if (DPLL(temppresent) == SATISFIABLE)
	{
		printf("congrats! you've solved the puzzle\n");
		getchar();
		getchar();
	}
	else
	{
		printf("sorry but you've failed");
		getchar();
		getchar();
	}
	return 0;
}



//----------------------------------------------------------------------------------//

//DPLL SOLVER SECTION//

//DPLL main function
int DPLL(Clause1 *C)
{
	if (DEBUG) printf("DPLL working\n");
	int solution = Check(C);
	if (DEBUG) printf("solution = %d\n", solution);
	if (solution != UNCERTAIN)
	{
		RemoveClause(C);
		return solution;
	}

	if (DEBUG) printf("unit propagationing\n");
	//unit propagation until no unit clause left
	while (1) 
	{
		solution = Check(C);
		if (solution != UNCERTAIN)
		{
			RemoveClause(C);
			return solution;
		}
		if (!UnitPropagation(C)) break;			//if can UP no more, break
	}
	if (DEBUG) printf("unit propa complete\n");

	if (DEBUG) printf("purelit eliminating\n");
	//purelit eliminate until no more PL left
	while (1)
	{
		solution = Check(C);
		if (solution != UNCERTAIN)
		{
			RemoveClause(C);
			return solution;
		}
		if (!PL_Elimination(C))
		{
			if (DEBUG) printf("cant eliminate\n");
			break;
		}
	}
	if (DEBUG) printf("purelit elimination complete\n");

	
	//if stuck, randomize a lit to branch on it
	/*
	int bridge = ChooseLiteral(C);
	if (DEBUG) printf("bridge chosen\n");
	if (DPLL(Branch(C, bridge)) == SATISFIABLE) 
		return SATISFIABLE;
	return DPLL(Branch(C, -bridge));
	*/
	//------------------------------------------------old dpll in paper
	
	int *tempvaluation = (int*)calloc(MAXVNUM + 1, sizeof(int));
	for (int i = 0; i <= MAXVNUM; i++)
		tempvaluation[i] = valuation[i];
	
	int L = ChooseLiteral(C);
	if (DPLL(Branch(C, L)) == SATISFIABLE)
		return SATISFIABLE;
	for (int i = 0; i <= MAXVNUM; i++)
		valuation[i] = tempvaluation[i];
	free(tempvaluation);

	return DPLL(Branch(C, -L));
}

//Read a cnf file and create a Linked List of it
Clause1* ReadCNF(char *filename)
{
	FILE *fp;
	char line[256];
	unsigned int length = 0;
	fp = fopen(filename, "r");
	if (fp == NULL) exit(1);

	//loop variables
	char *token;
	Clause1 *C = NULL;
	Clause1 *cur = NULL;
	Clause1 *pre = NULL;
	Literal *lcur = NULL;
	Literal *lpre = NULL;

	while (fgets(line, sizeof(line), fp))
	{
		// when comment, ignore lines
		if (line[0] == 'c') continue;
		// this line is metadata information
		if (line[0] == 'p')
		{
			sscanf(line, "p cnf %d %d", &vnum, &cnum);
			printf("Number of variables: %d\n", vnum);
			printf("Number of clauses: %d\n", cnum);
		}
		else
		{
			cur = (Clause1*)malloc(sizeof(Clause1));
			cur->Lit = NULL;
			cur->nextclause = NULL;
			if (C == NULL)
			{
				printf("setting head of clause\n\n");
				C = cur;
			}
			if (pre != NULL)
			{
				pre->nextclause = cur;
			}

			token = strtok(line, " ");				//read a literal
			while (token != NULL)
			{
				int L = atoi(token);
				if (DEBUG) printf("%d\n", L);
				//printf("a literal is read as %d\n",Literal);
				lcur = (Literal*)malloc(sizeof(Literal));

					lcur->index = L;
					lcur->next = NULL;
					//printf("a literal is added as %d %d\n", lcur->literal, lcur->status);
				
				if (L != 0)
				{
					if (cur->Lit == NULL)
						cur->Lit = lcur;
					if (lpre != NULL)
						lpre->next = lcur;
					//printf("a literal is added to the current clause\n");
				}
				lpre = lcur;
				token = strtok(NULL, " ");
			}
			pre = cur;
			//printf("a clause is created--------------------\n");
		}
	}
	printf("read CNF success!\n");

	valuation = (int*)calloc(MAXVNUM + 1, sizeof(int));
	int i;
	for (i = 0; i < MAXVNUM + 1; i++)
		valuation[i] = -1;
	printf("valuation of variables set\n\n");
	fclose(fp);
	return C;
}

//print CNF
int PrintCNF(Clause1 *C)
{
	Clause1 *p = C;
	while (p != NULL)
	{
		Literal *l = p->Lit;
		while (l != NULL)
		{
			printf("%d\t", l->index);
			l = l->next;
		}
		printf("\n");
		p = p->nextclause;
	}
	return 0;
}

//Write valuation of variables after DPLL
int WriteSolution(char * filename) 
{
	FILE *fp;
	if ((_access(filename, 0)) == -1)
	{
		printf("file not exist, create or not? y/n\n");
		char ch = getchar();
		while (ch != 'y' && ch != 'n')
			ch = getchar();
		if (ch == 'n')
			return 0;
	}
	fp = fopen(filename, "w");
	// iterate over valuation array to print the values of each literal
	int i;
	for (i = 1; i < vnum + 1; i++) 
	{
		fprintf(fp, "%d %d\n", i, valuation[i]);
	}
	printf("solution written\n");
	fclose(fp);
	return 1;
}

//Sudoku variable number is not start from 1
int WriteSudokuSolution(char * filename)
{
	FILE *fp;
	if ((_access(filename, 0)) == -1)
	{
		printf("file not exist, create or not? y/n\n");
		char ch = getchar();
		while (ch != 'y' && ch != 'n')
			ch = getchar();
		if (ch == 'n')
			return 0;
	}
	fp = fopen(filename, "w");
	
	int x, y, vanum, z;
	for (x = 1; x <= 9; x++)			//x row*100
	{
		//printf("1 round of x\n");		//format control
		for (y = 1; y <= 9; y++)		//y column*10
		{
			for (z = 1; z <= 9; z++)
			{
				vanum = 100 * x + 10 * y + z;						//max of 1000(999 + 1)
				fprintf(fp, "%d %d\n", vanum, valuation[vanum]);
			}
		}
	}
	printf("Sudoku solution written\n");
	fclose(fp);
	return 1;
}

void PrintClause(Clause1 *C) 
{
	Clause1 *temp = C;
	while (temp != NULL)
	{
		Literal *l = temp->Lit;
		while (l != NULL)
		{
			printf("%d ", l->index);
			l = l->next;
		}
		printf("\n");
		temp = temp->nextclause;
	}
}

Clause1* AddClause(Clause1 *C)					//add a close next to C
{
	Clause1 *temp;
	temp = (Clause1*)malloc(sizeof(Clause1));
	temp->Lit = NULL;
	temp->nextclause = C->nextclause;
	C->nextclause = temp;
	return temp;
}

Clause1* CloneClause(Clause1 *C)
{
	Clause1 *clonec = (Clause1*)malloc(sizeof(Clause1));
	clonec->Lit = NULL;
	clonec->nextclause = NULL;

	Literal *lcur = C->Lit;
	Literal *lpre = NULL;

	while (lcur != NULL)
	{
		Literal *clonel = (Literal*)malloc(sizeof(Literal));
		clonel->index = lcur->index;
		clonel->next = NULL;				//first lit into clause
		if (clonec->Lit == NULL)
		{
			clonec->Lit = clonel;
		}
		if (lpre != NULL)
		{
			lpre->next = clonel;			//insert clonelit to pre's next
		}
		lpre = clonel;						//lpre move to next
		lcur = lcur->next;
	}
	return clonec;
}

void RemoveLiteral(Literal *L)
{
	while (L != NULL) {
		struct Literal *next = L->next;
		free(L);
		L = next;
	}
}

void RemoveClause(Clause1 *C)
{
	while (C != NULL) 
	{
		Clause1 * next = C->nextclause;
		if (C->Lit != NULL) RemoveLiteral(C->Lit);
		free(C);
		C = next;
	}
}

int Find_NEXT_UnitClause(Clause1 *C)
{
	Clause1 *p = C;
	while (p != NULL)
	{
		if (p->Lit == NULL)
		{
			if (DEBUG)	printf("empty clause");
			continue;
		}
		if (p->Lit->next == NULL)
		{
				return p->Lit->index;
		}
		p = p->nextclause;
	}
	return 0;
}

int Find_Pure_Literal(Clause1 * C)
{
	int *lookup = (int*)calloc(MAXVNUM + 1, sizeof(int));	//lookup to record pureness
	Clause1 *p = C;
	while (p != NULL)
	{
		Literal *l = p->Lit;
		while (l != NULL)
		{
			int temp = lookup[abs(l->index)];
			if (temp == 0)
				lookup[abs(l->index)] = sign(l->index);
			else if (temp == -1 && sign(l->index) == 1)		//if being reversed lookup =2
				lookup[abs(l->index)] = 2;
			else if (temp == 1 && sign(l->index) == -1)
				lookup[abs(l->index)] = 2;
			l = l->next;
		}
		p = p->nextclause;
	}

	int i;
	for (i = 1; i < MAXVNUM + 1; i++)
	{
		if (lookup[i] == -1 || lookup[i] == 1)
		{
			return i*lookup[i];
		}
	}
	return 0;	//return 0 if there is no more pure lit
}

int UnitPropagation(Clause1 *C)
{
	int UnitLit = Find_NEXT_UnitClause(C);
	if (DEBUG) printf("unit clause found: %d\n", UnitLit);
	if (UnitLit == 0) 
		return 0;						//no more unit literal return 0

	valuation[abs(UnitLit)] = UnitLit > 0 ? 1 : 0;
										//value the unitlit to propagate
										//iterate the clause set
	Clause1 *p = C;
	Clause1 *prev = NULL;
	while (p != NULL) 
	{
		Literal *lcur = p->Lit;
		Literal *lpre = NULL; 
		lpre = (Literal*)malloc(sizeof(Literal));
		lpre->index = 0;
		lpre->next = NULL;
		while (lcur != NULL)
		{
			//if find unitclause, remove it
			if (lcur->index == UnitLit)	
			{
				if (p == C)				//if first clause unit, move root
				{
					*C = *(C->nextclause);
					p = NULL;
				}
				else
				{
					prev->nextclause = p->nextclause;
					p = prev;
				}
				break;
			}
			//if find negative form, remove the lit, others stay
			else if (lcur->index == -UnitLit)
			{
				if (lcur == p->Lit)
					p->Lit = lcur->next;
				else
					lpre->next = lcur->next;
				lcur = lpre;
				continue;
			}	//if head clause is removed, move pointer to new root
			lpre = lcur;
			lcur = lcur->next;
		}
		prev = p;
		p = (p == NULL ? C : p->nextclause);
	}
	if (DEBUG) printf("1 time unit prop complete\n");
	return 1;
}

//eliminate clause with pure literal
int PL_Elimination(Clause1 *C)
{
	int PureLit = Find_Pure_Literal(C);
	if (DEBUG) printf("pure literal found: %d\n", PureLit);
	if (PureLit == 0) 
		return 0;						//return 0 if no more pure lit

	//set valuation for literal
	if (DEBUG) printf("Setting value of literal %d as %d\n", abs(PureLit), PureLit > 0 ? 1 : 0);
	valuation[abs(PureLit)] = PureLit > 0 ? 1 : 0;
	//iterate the clause set, remove all clause containing the PureLit
	Clause1 *p = C;
	Clause1 *prev = NULL;

	while (p != NULL)
	{
		Literal *l = p -> Lit;
		while (l != NULL)
		{
			if (l->index == PureLit)	//find pure lit,remove whole clause
			{
				if (DEBUG) printf("Removing the clause that starts with %d\n", p->Lit->index);
				if (p == C)
				{
					*C = *(C->nextclause);
					p = NULL;
				}
				else
				{
					prev->nextclause = p->nextclause;
					p = prev;
				}
				break;
			}
			l = l->next;
		}
		if (DEBUG) printf("A clause is deleted\n");
		prev = p;
		p = p == NULL ? C : p->nextclause;		//if delete head clause,move to new head
	}
	return 1;
}

//simply choose the root clause
int ChooseLiteral(Clause1 *C)
{
	return C->Lit->index;
}

//choose the min length clause's root literal
int ChooseLiteral2(Clause1 *C) 
{
	int chosen = 0;
	int count, min = 100;
	struct Clause1 *p = C;
	while (p != NULL) 
	{
		struct Literal *l = p->Lit;
		for (count = 0; l != NULL; count++) 
		{
			l = l->next;
		}
		if (count == 1) 
		{
			chosen = p->Lit->index;
			return chosen;
		}
		if (count < min)
		{
			min = count;
			chosen = p->Lit->index;
		}
		p = p->nextclause;
	}
	return chosen;
}

//when stuck branch deal with a new lit produced by ChooseLiteral func
Clause1* Branch(Clause1 *C, int e)
{
	valuation[abs(e)] = e > 0 ? 1 : 0;

	Clause1 *newclone = NULL;
	Clause1 *cur = NULL;
	Clause1 *pre = NULL;
	Clause1 *p = C;

	while (p != NULL)
	{
		Clause1 *clone = CloneClause(p);
		if (newclone == NULL)
		{
			newclone = clone;
		}
		if (pre != NULL)
		{
			pre->nextclause = clone;
		}
		pre = clone;
		p = p->nextclause;
	}

	//initialize addedclause
	Clause1 *addedc = (Clause1*)malloc(sizeof(Clause1));
	addedc->Lit = NULL;
	addedc->nextclause = NULL;

	//add literal addedl as a pure lit into added clause
	Literal* addedl = (Literal*)malloc(sizeof(Literal));
	addedl->index = e;
	addedl->next = NULL;

	//insert addedl into the head of addedc
	addedc->Lit = addedl;
	addedc->nextclause = newclone;
	return addedc;
}

//checking f//

//1 contains, 0 not
int IfContainEmptyClause(Clause1 *C)
{
	Clause1 *p = C;
	while (p != NULL)
	{
		if (p->Lit == NULL)
			return 1;
		p = p->nextclause;
	}
	return 0;
}

//1 all unit, 0 not
int IfAllUnitClause(Clause1 *C)
{
	int *lookup = (int*)calloc(MAXVNUM + 1, sizeof(int));	//lookup to record pureness
	Clause1 *p = C;
	Literal *l = NULL;
	while (p != NULL)
	{
		l = p->Lit;
		while (l != NULL)
		{
			int temp = lookup[abs(l->index)];
			if (temp == 0)
				lookup[abs(l->index)] = sign(l->index);
			//if lit come with opposite sign, return false to signify unsatisfiable
			else if (temp == -1 && sign(l->index) == 1)	return 0;
			else if (temp == 1 && sign(l->index) == -1)	return 0;
			l = l->next;
		}
		p = p->nextclause;
	}
	p = C;
	while (p != NULL)
	{
		l = p->Lit;
		while (l != NULL)
		{
			valuation[abs(l->index)] = l->index > 0 ? 1 : 0;
			l = l->next;
		}
		p = p->nextclause;
	}
	return 1;
}

int Check(Clause1 *C)
{
	if (IfContainEmptyClause(C))
	{
		if (DEBUG) printf("Empty clause found\n");
		return UNSATIFIABLE;
	}
	if (IfAllUnitClause(C))
	{
		if (DEBUG) printf("all unit clause\n");
		return SATISFIABLE;
	}	
	return UNCERTAIN;
}

//----------------------------------------------------------------------------------//