//**DO NOT** CHANGE THE PROTOTYPES FOR THE FUNCTIONS GIVEN TO YOU. WE TEST EACH
//FUNCTION INDEPENDENTLY WITH OUR OWN MAIN PROGRAM.
#include "../include/map_reduce.h"
#include <math.h>

DIR* directory;
int sum = 0;
int number_of_files;
int mode = 0;
int min = 1000;
int max = 0;


int cmpfunc (const void * a, const void * b)
{
   return ( *(int*)a - *(int*)b );
}

//Implement map_reduce.h functions here.
int validateargs(int argc, char** argv)
{
	if(argc == 1 || argc >= 6) return -1;

	else if(strcmp(argv[1], "-h") == 0) return 0;

	else if(strcmp(argv[1], "ana") == 0 && is_directory(argv[2])) return 1;

	else if(strcmp(argv[1], "stats") == 0 && is_directory(argv[2])) return 2;

	else if(strcmp(argv[1], "-v") == 0 && strcmp(argv[2], "ana") == 0 && is_directory(argv[3])) return 3;

	else if(strcmp(argv[1], "-v") == 0 && strcmp(argv[2], "stats") == 0 && is_directory(argv[3])) return 4;

	else return -1;
}

int is_directory(char* path)
{
	directory = opendir(path);
	if (directory)
	{
		closedir(directory);
		return 1;
	}
	else if(!directory)
	{
	    closedir(directory);
	    return -1;
	}
	else
	{
		printf("opendir error.\n");
		closedir(directory);
		return -1;
	}
}

int nfiles(char* dir)
{
	number_of_files = 0;
	struct dirent * files;
	directory = opendir(dir);

	while((files = readdir(directory)) != NULL) 
	{
	    if(files -> d_type == DT_REG) number_of_files++;
	}

	closedir(directory);
	return number_of_files;	
}

int map(char* dir, void* results, size_t size, int (*act)(FILE* f, void* res, char* fn))
{
	struct dirent * file_names;
	directory = opendir(dir);
	int map_result = 0;

	while((file_names = readdir(directory)) != NULL)
	{
		if(file_names -> d_type == DT_REG)
		{
			char f_name[strlen(dir) + strlen(file_names -> d_name)];
			strcpy(f_name, dir);
			strcat(f_name, "/");
			strcat(f_name, file_names -> d_name);
			
			FILE *fp;
			fp = fopen(f_name, "r");
			if(fp != NULL)
			{				
				map_result += (act(fp, results, file_names -> d_name));
				results = results + size;
				fclose(fp);
			}
			else
			{
				return -1;
				printf("File does not exist.\n");
			}
		}
	}
	return map_result;	
}

int analysis(FILE* f, void* res, char* filename)
{
	int line_length = 0;
	int line_num = 0;
	int bytes = 0;
	int c;

	struct Analysis *ana = (struct Analysis *)res;

	ana->filename = strdup(filename);

	while((c = fgetc(f)) != EOF) 
	{
		ana->ascii[c] += 1;
		line_length++;
		bytes++;

        if(c == '\n')
        {
        	line_num++;
        	if(line_length > ana->lnlen)
	        {
	        	ana->lnlen = line_length - 1;
	        	ana->lnno = line_num;
	        }
        	line_length = 0;
        }
    }
    return bytes;
}

int stats(FILE* f, void* res, char* filename)
{
	int sum = 0;
	int n = 0;
	
	Stats *stats = (struct Stats *)res;
	stats->filename = filename;

	int index = 0;
	if(stats->filename[0] == '.')
	{

	}
	else
	{
		while(fscanf(f, "%d", &index) != EOF)
		{
			if(index > max) max = index;
			if(index < min) min = index;
			stats->histogram[index] += 1;
			sum += index;
			n++;
		}
		
		stats->sum = sum;
		stats->n = n;
	}
	
	return EXIT_SUCCESS;
}

struct Analysis analysis_reduce(int n, void* results)
{
	struct Analysis *ana = (struct Analysis *)results;
	struct Analysis reduced_ana = {{0}, 0, 1, ""};
	int i;
	int j;

	for(i = 0; i < n; i++)
	{
		for(j = 0; j < 128; j++)
		{
			reduced_ana.ascii[j] = reduced_ana.ascii[j] + ana[i].ascii[j];
		}
		if(ana[i].lnlen > reduced_ana.lnlen) 
		{
			reduced_ana.lnlen = ana->lnlen;
			reduced_ana.lnno = ana[i].lnno;
			reduced_ana.filename = ana[i].filename;
		}
	}
	return reduced_ana;
}

Stats stats_reduce(int n, void* results)
{
	Stats *stats = results;
	Stats reduced_stats = {{0}, 0, 0, ""};
	int i;
	int j;

	reduced_stats.filename = NULL;

	for(i = 0; i < n; i++)
	{
		for(j = 0; j < NVAL; j++)
		{
			reduced_stats.histogram[j]  = reduced_stats.histogram[j] + stats[i].histogram[j];

			if(reduced_stats.histogram[j] > mode)
			{
				mode = reduced_stats.histogram[j];
			}
		}
		reduced_stats.sum += stats[i].sum;
		reduced_stats.n += stats[i].n;
	}
	return reduced_stats;
}

void analysis_print(struct Analysis res, int nbytes, int hist)
{
	int i;
	int j;

	if(hist == 1)
	{
		printf("File: %s\n", res.filename);
		printf("Longest line length: %d\n", res.lnlen);
		printf("Longest line number: %d\n\n", res.lnno);
		
	}
	else if(hist == 0)
	{
		printf("FILE: %s\n", res.filename);
        printf("Longest line length: %d\n", res.lnlen);
        printf("Longest line number: %d\n\n", res.lnno);
        printf("Total Bytes in directory: %d\n", nbytes);
		printf("Histogram:\n");

		for(i = 0; i < 128; i++)
		{
			if(res.ascii[i] != 0) 
			{
				printf(" %d:", i);
				for(j = 0; j < res.ascii[i]; j++)
				{
					printf("-");
				}
				printf("\n");
			}
		}
	}
}

void stats_print(Stats res, int hist)
{
	int i;
	int j;

	if(hist == 1)
	{
		//-------------------------------------------
		printf("Histogram: \n");
		for(i = 0; i < NVAL; i++)
		{
			if(res.histogram[i] == 0)
			{

			}
			if(res.histogram[i] != 0) 
			{
				printf("%d   :", i);
				for(j = 0; j < res.histogram[i]; j++)
				{
					printf("-");
				}
					printf("\n");
			}	
		}
		//-------------------------------------------
		printf("Count: %d\n", res.n);
		//-------------------------------------------
		printf("Mean: %f\n", (double)res.sum / res.n);
		//-------------------------------------------
		int i;
		printf("Mode:");
		for(i = 0; i < NVAL; i++)
		{
			if(res.histogram[i] == mode)
			{
				printf(" %d ", i);
			}
		}
		printf("\n");
		//-------------------------------------------

		int med_index = (res.n + 1) / 2;

			int mn;
			for(mn = 0; mn < NVAL; mn++)
			{
				med_index = med_index - res.histogram[mn];
				if(med_index <= 0)
				{
					printf("Median: %f\n", (float)mn);
					break;
				}
			}
		//-------------------------------------------
		int first_q = (res.n + 1) / 4;

		int m;
		for(m = 0; m < NVAL; m++)
		{
			first_q = first_q - res.histogram[m];
			if(first_q <= 0)
			{
				printf("Q1: %f\n", (float)m);
				break;
			}
		}
		//-------------------------------------------
		int third_q = (res.n + 1) * 0.75;
		int n;
		for(n = 0; n < NVAL; n++)
		{
			third_q = third_q - res.histogram[n];
			if(third_q <= 0)
			{
				printf("Q3: %f\n", (float)n);
				break;
			}
		}
		//-------------------------------------------
		printf("Min: %d\n", min);
		//-------------------------------------------
		printf("Max: %d\n", max);
	}
	else if(hist == 0)
	{

		if(res.filename[0] == '.')
		{

		}
		else
		{
			printf("File: %s\n", res.filename);
			//-------------------------------------------
			printf("Count: %d\n", res.n);
			//-------------------------------------------
			printf("Mean: %f\n", (double)res.sum / res.n);
			//-------------------------------------------
			int i;
			int m_mode = 0;

			for(i = 0; i < NVAL; i++)
			{
				if(res.histogram[i] > m_mode)
				{
					m_mode = res.histogram[i];
				}
			}
			int j;
			for(j = 0; j < NVAL; j++)
			{
				if(res.histogram[j] == m_mode)
				{
					printf("Mode: %d\n", j);
				}
			}
			//-------------------------------------------
			int median_index = (res.n + 1) / 2;

			int me;
			for(me = 0; me < NVAL; me++)
			{
				median_index = median_index - res.histogram[me];
				if(median_index <= 0)
				{
					printf("Median: %f\n", (float)me);
					break;
				}
			}
			//-------------------------------------------
			int f_q = (res.n + 1) / 4;

			int mn;
			for(mn = 0; mn < NVAL; mn++)
			{
				f_q = f_q - res.histogram[mn];
				if(f_q <= 0)
				{
					printf("Q1: %f\n", (float)mn);
					break;
				}
			}
			//-------------------------------------------
			int t_q = (res.n + 1) * 0.75;
			int no;
			for(no = 0; no < NVAL; no++)
			{
				t_q = t_q - res.histogram[no];
				if(t_q <= 0)
				{
					printf("Q3: %f\n", (float)no);
					break;
				}
			}
			//-------------------------------------------
			int mmin = 0;
			int ll;
			for(ll = 0; ll < NVAL; ll++)
			{
				if(res.histogram[ll] != 0)
				{
					mmin = ll;
					break;
				}
			}
			printf("Min: %d\n", mmin);
			//-------------------------------------------
			int mmax = 0;
			int kk;
			for(kk = 0; kk < NVAL; kk++)
			{
				if(res.histogram[kk] > mmax) mmax = kk;
			}
			printf("Max: %d\n\n", mmax);
		}
	}
}