#include "../include/map_reduce.h"

//Space to store the results for analysis map
struct Analysis analysis_space[NFILES];
//Space to store the results for stats map
Stats stats_space[NFILES];

//Sample Map function action: Print file contents to stdout and returns the number bytes 
//in the file.
int cat(FILE* f, void* res, char* filename) 
{
    char c;
    int n = 0;
    printf("%s\n", filename);
    while((c = fgetc(f)) != EOF) {
        printf("%c", c);
        n++;
    }
    printf("\n");
    return n;
}

int main(int argc, char** argv) 
{
    int number_of_files = 0;
    int map_it = 0;
    int check = validateargs(argc, argv);
    int v = 0;

    if(check == -1)
    {
        USAGE(argv[0]);
        return EXIT_FAILURE;
    }
    else if(check == 0)
    {
        USAGE(argv[0]);
    }
    else if(check >= 1)
    {
        if(strcmp(argv[1], "-v") == 0) v = 1;

        number_of_files = nfiles(argv[argc - 1]);

        if(number_of_files == 0)
        {
            printf("No files present in the directory.\n");
        }
        else if(number_of_files > 0)
        {
            if(check == 1)
            {
                map_it = map(argv[2], analysis_space, sizeof(analysis_space) / NFILES, analysis);

                if(map_it == -1) return EXIT_FAILURE;
                else 
                    {
                        struct Analysis a_reduce = analysis_reduce(number_of_files, analysis_space);
                        analysis_print(a_reduce, map_it, v);
                    }
            }
            else if(check == 2)
            {
                map_it = map(argv[2], stats_space, sizeof(stats_space) / NFILES, stats);
                if(map_it == -1) return EXIT_FAILURE;
                else 
                {
                    Stats s_reduce = stats_reduce(number_of_files, stats_space);
                    stats_print(s_reduce, 1);
                }            
            }
            else if(check == 3)
            {
                map_it = map(argv[3], analysis_space, sizeof(analysis_space) / NFILES, analysis);
                if(map_it == -1) return EXIT_FAILURE;
                else
                {
                    int i;
                    struct Analysis a_reduce = analysis_reduce(number_of_files, analysis_space);
                    for(i = 0; i < number_of_files; i++)
                    {
                        analysis_print(analysis_space[i], map_it, 1);
                    }
                    analysis_print(a_reduce, map_it, 0);
                }
            }
            else if(check == 4)
            {
                map_it = map(argv[3], stats_space, sizeof(stats_space) / NFILES, stats);
                if(map_it == -1) return EXIT_FAILURE;
                else
                {
                    int j;
                    Stats s_reduce = stats_reduce(number_of_files, stats_space);
                    for(j = 0; j < number_of_files; j++)
                    {
                        stats_print(stats_space[j], 0);
                    }
                    stats_print(s_reduce, 1);
                } 
            }
        }
        else if(number_of_files < 0)
        {
            printf("There was a failure.\n");
            return EXIT_FAILURE;
        } 
    }
    return EXIT_SUCCESS;
}
