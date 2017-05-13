#include "lsmt.h"

void quick_sort(int* arr, int low, int high)
{
	int pi;
    if (low < high)
    {
        /* pi is partitioning index, arr[p] is now
           at right place */
        pi = partition(arr, low, high);

        quick_sort(arr, low, pi - 1);  // Before pi
        quick_sort(arr, pi + 1, high); // After pi
    }
}

int partition (int* arr, int low, int high)
{
    int i, j;
    // pivot (Element to be placed at right position)
    int pivot = arr[high];

    i = (low - 1);  // Index of smaller element

    for (j = low; j <= high- 1; j++)
    {
        // If current element is smaller than or
        // equal to pivot
        if (arr[j] <= pivot)
        {
            i++;    // increment index of smaller element
            int temp = arr[i];
	    arr[i] = arr[j];
	    arr[j] = temp;
        }
    }
    int temp = arr[i + 1];
    arr[i + 1] = arr[high];
    arr[high] = temp;
    return (i + 1);
}

//print the tree, first the avl tree and then all the levels
void print_stats(lsmtree* l)
{
	int i,j,k;
	int key, value;
	//print AVL tree
	avl_traverse_dfs(l->c0_tree);
	putchar('\n');
	//print all the levels
	for(i = 0; i < l->numlevel; i++)
	{
		FILE* f = l->fptrs[i];
		fseek(f, 0, SEEK_SET);
		for(j = 0; j < l->file_sizes[i]; j++)
		{
			fscanf(f, "%d", &key);
			fscanf(f, "%d", &value);
			printf("%d:%d:L%d ", key, value, i+2);
		}
		putchar('\n');
	}
}

//mark key to be deleted in the deleted_nodes array
//if full then flush avl tree to disk
void delete_key(lsmtree* l, int key)
{
	if(l->curdeletes >= l->maxdeletes)
	{
		flush_to_disk(l);
		l->curdeletes = 0;
	}
	l->deleted_nodes[l->curdeletes++] = key;
}
	
//returns all the keys in the tree l within the range (key1, key2)
list* range(lsmtree* l, int key1, int key2)
{
	//search all the keys in the avl tree
	int key, i;
	list* lp = NULL;
	int low, high, mid;
	int k, v, kprev, vprev;

	quick_sort(l->deleted_nodes, 0, l->maxdeletes - 1);
	for(key = key1; key < key2; key++)
	{
        //skip if key is present in deleted_nodes
        if(binsearch(key, l->deleted_nodes, 0, l->maxdeletes - 1) == 1)
            continue;
		avl_node* an = avl_find(l->c0_tree, key);
		if( an != NULL)
		{
			list* temp = (struct list*)malloc(sizeof(list));
			if(temp == NULL)
				error_msg(1, "cannot allocate temp in range\n");
			(temp->p).key = an->key;
			(temp->p).value = an->value;

			if(lp == NULL)
			{
				lp = temp;
				lp->next=NULL;
			}
			else
			{
				temp->next = lp;
				lp = temp;
			}
		}
	}
	//search all the remaining levels
	for(i = 0; i < l->numlevel; i++)
	{
		int found = 0;
		int index = 0;
		//find the key closest to key1 in level i using binary search among the keys in the fence_ptrs 
		low = 0;
		high = l->file_sizes[i];
		FILE* f = l->fptrs[i];
		while(!found && low <= high)
		{
			int mid = (low + high) / 2;
			fseek(f, (mid-1) * 2 * (sizeof(int) + sizeof(char)), SEEK_SET);
			fscanf(f, "%d", &kprev);
			fscanf(f, "%d", &vprev);
			fscanf(f, "%d", &k);
			fscanf(f, "%d", &v);
			if(k == key1 || (k > key1 && kprev < key1)) 
			{
				index = mid;
				found = 1;
			}
			if(k < key1)
				low = mid + 1;
			else 
				high = mid - 1;
		}
		if(found)
		{
			//do a sequential search from index until key2
			fseek(f, mid * 2 * (sizeof(int) + sizeof(char)), SEEK_SET);
			fscanf(f, "%d", &k);
			fscanf(f, "%d", &v);
			do
			{
                //skip if k is deleted
                if(binsearch(k, l->deleted_nodes, 0, l->maxdeletes - 1) == 1)
                {
                    fscanf(f, "%d", &k);
                    fscanf(f, "%d", &v);
                    continue;
                }
				list* temp = malloc(sizeof(list));
				if(temp == NULL)
					error_msg(1, "cannot allocate temp in range\n");
				(temp->p).key = k; 
				(temp->p).value = v;

				if(lp == NULL)
				{
					lp = temp;
					lp->next=NULL;
				}
				else
				{
					temp->next = lp;
					lp = temp;
				}
				fscanf(f, "%d", &k);
				fscanf(f, "%d", &v);
			}
			while(k <= key2);
		}
	}
	return lp;
}

			
//search for the key in lsmtree l
//returns -1 if the key is not found
//returns +1 if the key is actually found
//otherwise value contains the key
int search(lsmtree* l, int key, int* value)
{
	int k = key;
	int i;
    //if the key is present in the list deleted_nodes, then the key is not present
	quick_sort(l->deleted_nodes, 0, l->curdeletes);
    if(binsearch(key, l->deleted_nodes, 0, l->curdeletes) == 1)
        return -1;
	//search in the avl tree
	avl_node* an = avl_find(l->c0_tree,key);
	if(an != NULL)
	{
		*value = an->value;
		return 1;
	}
	//search in the levels
	for(i = 0; i < l->numlevel; i++)
	{
		//search at i-th level
		if(i <= l->unfiltered)
		{
			if(test_bloom(l->bloom[i],&k))
			{
				//check for false positive
                //first search in the fence_ptrs
                int loc = binsearch_in_fence(l->fence_ptrs[i], 0, l->file_sizes[i] / l->pagesize, key);
                if(loc == -1)
                    continue;
				int found = search_in_page(l->fptrs[i], 0, l->file_sizes[i], l->pagesize, key, value);
				if(found == 1)
					return 1;
                
			}
		}
		else
		{
            //first search in the fence_ptrs
            int loc = binsearch_in_fence(l->fence_ptrs[i], 0, l->file_sizes[i] / l->pagesize, key);
            if(loc == -1)
                continue;
			int found = search_in_page(l->fptrs[i], 0, l->file_sizes[i], l->pagesize, key, value);
			if(found == 1)
				return 1;
		}	
	}
	//key not found
	return -1;
}

//returns the loc such that arr[loc] <= key < arr[loc + 1]
//or arr[end] <= key
//otherwise returns -1
int binsearch_in_fence(int* arr, int start, int end, int key)
{
    if(arr[end] <= key)
        return end;

    if(arr[start] > key)
        return -1;
    //now key is guaranteed to be within the range
    int low = start;
    int high = end;
    int mid;

    while(low <= high)
    {
        mid = (low + high) / 2;
        if(key <= arr[mid+1] && key >= arr[mid])
            return mid;
        else if(key > arr[mid+1])
            low = mid + 1;
        else if(arr[mid-1] < key)
            return mid-1;
        else
            high = mid-1;
    }
    return -1;
}
//search key in the page
//start : the starting page
//pagesize : page size
int search_in_page(FILE* f, int start, int filesize, int pagesize, int key, int* value) 
{
    int low = start * pagesize;
    int end = (start + 1)*pagesize > filesize ? filesize : (start + 1)*pagesize;

    return search_in_file(f, low, end, key, value);
}

//binary search on file pointed to by f
int search_in_file(FILE* f, int start, int end, int key, int* value)
{
	int low = start;
	int high = end;
	int mid, cur;
	int k, v;

	while(low <= high)
	{
		int mid = (low + high) / 2;
		fseek(f, mid * 2 * (sizeof(int) + sizeof(char)), SEEK_SET);
		fscanf(f, "%d", &k);
		fscanf(f, "%d", &v);
		if(key == k)
		{
			*value = v;
			return 1;
		}
		if(key > k)
			low = mid + 1;
		else
			high = mid - 1;
	}
	return -1;
}


//binsearch: binary search in an array arr
//0 if not found and 1 if found
int binsearch(int key, int* arr, int start, int end)
{
	int low = start;
	int high = end;
	int mid;

	while(low <= high)
	{
		int mid = (low + high)/ 2;
		if(arr[mid] == key)
			return 1;
		if(key > arr[mid])
			low = mid + 1;
		else
			high = mid - 1;
	}
	return 0;
}

void flush_tree_to_disk(lsmtree* l)
{
	//write the contents of the avl tree to a file
	FILE* f = fopen("temp", "w");
	if (f == NULL)
		error_msg(1, "cannot open file temp\n");
	flush_tree_to_file(f, l->c0_tree->root);
	l->c0_tree->size = 0;
	free(l->c0_tree->root);
	l->c0_tree->root = NULL;
	fclose(f);
}	

void flush_to_disk(lsmtree* l)
{	
	int loc, key, value;
	int key1, key2, value1, value2;
	int limit = pow(2,l->maxdepth)-1;
	pair* del_nodes;

	//flush the entries of the avl tree to disk
	flush_tree_to_disk(l);
	//now iteratively merge temp with all the files
	//read from file
	FILE* f = fopen("temp", "r");
	if (f == NULL)
		error_msg(1, "cannot open temp in lsmt_insert\n");
	if ( (del_nodes = (pair*)malloc(sizeof(pair)*limit)) == NULL)
		error_msg(1, "cannot allocate del_nodes\n");
	loc = 0;
	while(!feof(f))
	{
		//read key value pair and store in del_nodes
		fscanf(f, "%d", &key);
		fscanf(f, "%d", &value);
		del_nodes[loc].key = key;
		del_nodes[loc].value = value;
	}
	fclose(f);
	//iteratively flush the del_nodes to higher levels
	int pos = 0;

	loc = 0;

	quick_sort(l->deleted_nodes, 0, l->maxdeletes - 1);
	while(1)
	{
		//reset the bloom filter
		reset_bloom(l->bloom[pos]);
		//merging l->fptrs[pos] with the remaining entries of the array del_nodes
		FILE* fp = fopen("tempfile.txt", "w");
		fseek(l->fptrs[pos], 0, SEEK_SET);
		key1 = del_nodes[loc].key;
		value1 = del_nodes[loc].value;
		if(l->file_sizes[pos] != 0)
		{
			fscanf(l->fptrs[pos], "%d", &key2);
			fscanf(l->fptrs[pos], "%d", &value2);
		}
		while(l->file_sizes[pos] < l->level_sizes[pos] && loc < limit)
		{
			if(key1 < key2 && !binsearch(key1, l->deleted_nodes, 0, l->maxdeletes - 1))
			{
				fprintf(fp, "%d %d ", key1, value1);
				//add to bloom filter if i is not unfiltered
				if(pos <= l->unfiltered)
				{
					int tempkey = key1;
					add_to_bloom(l->bloom[pos], &tempkey);
				}
				//read the next key and value from the array
				loc++;
				key1 = del_nodes[loc].key;
				value1 = del_nodes[loc].value;
			}
			else if(!binsearch(key2, l->deleted_nodes, 0, l->maxdeletes - 1))
			{
				fprintf(fp,  "%d %d ", key2, value2);
				//need to add to bloom filter if i is not unfiltered
				if(pos <= l->unfiltered)
				{
					int tempkey = key2;
					add_to_bloom(l->bloom[pos], &tempkey);
				}
				//read the next entry from the file
				fscanf(l->fptrs[pos], "%d", &key2);
				fscanf(l->fptrs[pos], "%d", &value2);
			}
			l->file_sizes[pos]++;
		}
		fclose(l->fptrs[pos]);
        fclose(fp);
		//write temp in place of l->fptrs[pos]
        char s[10];
        sprintf(s, "file%d", pos+1);
        //copy tempfile.txt to s
        FILE* fpsrc = fopen("tempfile.txt", "r");
        FILE* fpdst = fopen(s, "w");

        int count = 0;
        while(!feof(fpsrc))
        {
            int k, v;
            fscanf(fpsrc, "%d %d ", &k, &v);
            fprintf(fpdst, "%d %d ", k, v);
            if(count % l->pagesize == 0)
                l->fence_ptrs[pos][count % l->pagesize] = k;
        }
        fclose(fpsrc);
        fclose(fpdst);

		if (loc == limit)
			break;
		else if(pos >= l->maxdepth)
			error_msg(1,"pos exceeds maxdepth!\n");
		else
			pos++;
	}

}
	
//insert: if the size of the avl tree exceeds limit flush the entire
//avl tree to disk and then insert in the avl tree
void lsmt_insert(lsmtree* l,  int key, int value)
{
	if (l->c0_tree->size >= pow(2,l->maxdepth))
	{
		flush_to_disk(l);	
	}
	//finally insert at the c0 tree
	avl_insert(l->c0_tree, key, value);
	//printf("size of the avl tree %d\n", l->c0_tree->size);
}

//maxdepth : maximum depth of the c0 tree (AVL tree)
//numlevel : number of levels after the first level
//p1	   : false positive rate of the first level, all subsequent
//levels are multiplied by T
//unfiltered : the false positive rates of all levels after unfiltered are 1

lsmtree* initialize(unsigned int maxdepth, unsigned int T, int numlevel, double p1, int unfiltered, unsigned int maxdels, int pg)
{
	lsmtree* l;
	int i;
	char s[8];

	if( (l = malloc(sizeof(lsmtree))) == NULL)
		error_msg(1,"cannot allocate lsmtree\n");
	
	l->c0_tree = avl_create();
     	l->maxdepth = maxdepth;
	l->T = T;
	l->numlevel = numlevel;
	l->unfiltered = unfiltered;
	
	//init the false positive rates
	if( (l->false_pos = malloc(sizeof(double)*numlevel)) == NULL)
		error_msg(1,"cannot allocate false positive probabilities\n");

	for(i = 0; i < numlevel; i++)
		if(i > unfiltered)
			l->false_pos[i] = 1;
		else
			l->false_pos[i] = pow(T,i)*p1;

	//init the file pointers
	if( (l->fptrs = malloc(sizeof(FILE*) * numlevel)) == NULL)
		error_msg(1,"cannot allocate file pointers\n");

	if( (l->level_sizes = malloc(sizeof(int) * numlevel)) == NULL)
		error_msg(1, "cannot allocate level_sizes\n");

	if( (l->file_sizes = malloc(sizeof(int) * numlevel)) == NULL)
		error_msg(1, "cannot allocate file_sizes\n");

	for(i = 0; i < numlevel; i++)
	{
		sprintf(s, "file%d", i+1);
		l->fptrs[i] = fopen(s, "w");
		l->file_sizes[i] = 0;
		l->level_sizes[i] = pow(2,maxdepth) * pow(T,i+1);
		if (l->fptrs[i] == NULL)
			error_msg(3, "cannot open ", s, "\n");
	}
	

	//init the bloom filters
	if ( (l->bloom = malloc(sizeof(bloom_filter*)*numlevel)) == NULL)
		error_msg(1,"cannot allocate bloom\n");

	for(i = 0; i < numlevel; i++)
	{
		if(i <= unfiltered)
		{
			//calculate number of bytes for i-th bloom filter
			unsigned int bits = (ceil(  (pow(2,maxdepth) * pow(T,i+1) * log(1/l->false_pos[i]))  / pow(log(2),2))) / 8;
			printf("%d %d\n", i, bits);
			l->bloom[i] = create_bloom(bits);
			//calculate number of hash functions for the i-th bloom filter
			double entries = pow(2,maxdepth) * pow(T,i);
			int k = log(2)*((double)bits*8 / entries);
			for(int j = 0; j < k; j++)
				add_hash_bloom(l->bloom[i], djb2, j);
		}
	}
	
	l->maxdeletes = maxdels;
	if( (l->deleted_nodes = (int*)malloc(sizeof(int)*maxdels)) == NULL)
		error_msg(1, "cannot allocate deleted_nodes in initialize\n");

	l->curdeletes = 0;
    l->pagesize = pg;

    if( (l->fence_ptrs = (int**)malloc(sizeof(int*) * l->numlevel)) == NULL)
        error_msg(1, "cannot allocate fence_ptrs\n");
    for(i = 0; i < l->numlevel; i++)
        if( (l->fence_ptrs[i] = (int*)malloc(sizeof(int) * (l->level_sizes[i] / l->pagesize))) == NULL)
            error_msg(1, "cannot allocate fence ptrs i \n");
	return l;
}
