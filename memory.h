
void allocate_fptr() 
{
	int PAGESIZE = 1024;	//number of entries in a page
	int NUMLEVELS = 4;
	int C0SIZE = 10000;	//number of entries in the first level
	int** fptr[NUMLEVELS];
	int cursize = C0SIZE;
	int LEVEL_MULT = 10;
	int** levels[NUMLEVELS];

	int i, j;

	for (i = 0; i < NUMLEVELS; i++)
	{
		fptr[i]=(int**)malloc(sizeof(int*) * (cursize / PAGESIZE + 1));
		cursize *= LEVEL_MULT;
	}
	//initialize fptr array
	cursize = C0SIZE;
	for (i = 0;  i < NUMLEVELS; i++)
	{
		levels[i] = (int**)malloc(sizeof(int*) * cursize);
		cursize *= LEVEL_MULT;
	}
	//store the fence pointers
	cursize = C0SIZE;
	for (i = 0; i < NUMLEVELS; i++)
	{
		for (j = 0; j < cursize; j+= PAGESIZE)
		{
			fptr[i][j/PAGESIZE] = &(levels[i][j]);
		}
		cursize *= LEVEL_MULT;
	}


