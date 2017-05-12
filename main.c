#include "lsmt.h"

int main(int argc, char* argv[])
{
	FILE* fpIn;
	char action;
	int key, value;
	int start_key, end_key;
	int i,j;
	int cursize;

	if (argc == 1)
		error_msg(1, "input format : ./a.out filename\n");
	

	if ( (fpIn = fopen(argv[1], "r")) == NULL)
		error_msg(3, "bad file ", argv[1], "\n");

	unsigned int maxdepth = 40;
	unsigned int T = 2;
	int numlevel = 10;
	double p1 = 0.01;
	int unfiltered = 7;
	unsigned int maxdels = pow(maxdepth, 5);
	lsmtree* l = initialize(maxdepth, T, numlevel, p1, unfiltered, maxdels);

	while(!feof(fpIn))
	{
		fscanf(fpIn, "%c", &action);
		if (action == 'p')
		{
			//read (key, value) pair
			fscanf(fpIn, "%d %d", &key, &value);
			lsmt_insert(l, key, value);
			//printf("p %d %d\n", key, value);
		}
		else if (action == 'g')
		{
			//read key
			fscanf(fpIn, "%d", &key);
			if(search(l, key, &value) == 1)
				printf("%d\n", value);
			else
				printf("not found\n");
			//printf("g %d\n", key);
		}
		else if (action == 'd')
		{
			//read key
			fscanf(fpIn, "%d", &key);
			delete_key(l, key);
			//printf("d %d\n", key);
		}
		else if (action == 'r')
		{
			//read start and end key
			fscanf(fpIn, "%d %d", &start_key, &end_key);
			list* lkeys = range(l, start_key, end_key);
			list* temp = lkeys;
			while(temp != NULL)
			{
				printf("%d:%d ", (temp->p).key, (temp->p).value);
				temp = temp->next;
			}
			printf("r %d %d\n", start_key, end_key);
		}
		else if (action == 'o')
		{
			//print stats
			print_stats(l);
		}
	}
	fclose(fpIn);
	return 0;
}


