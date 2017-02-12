#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

int main () {

   int n[100]; 
   int i,j;
   char check = 'A';
   unsigned index = (unsigned)check;

   /* initialize elements of array n to 0 */         
   for ( i = 0; i < 100; i++ ) {
      n[ i ] = i; 
   }
   
   /* output each array element's value */
   for (j = 0; j < 100; j++ ) {
   		if(n[index] == 'A')
   		{
   			n[index] += 100;
   		}
    	printf("Element[%d] = %d\n", j, n[j] );
   }
 
   return 0;
}
