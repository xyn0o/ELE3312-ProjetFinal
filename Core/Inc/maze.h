#include <stdint.h>

uint16_t maze[] = {
6, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 1, 0, 1, 6, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 
1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 
1, 0, 6, 2, 4, 0, 1, 0, 2, 2, 4, 0, 0, 1, 1, 0, 1, 1, 0, 0, 2, 2, 2, 2, 0, 0, 2, 2, 2, 2, 2, 1, 
1, 0, 1, 0, 1, 0, 1, 0, 0, 6, 5, 0, 0, 3, 5, 0, 3, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 
1, 0, 3, 2, 5, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 2, 2, 2, 11, 
1, 0, 0, 0, 0, 0, 1, 0, 2, 5, 0, 0, 0, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 
1, 0, 2, 10, 2, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 15, 0, 0, 0, 2, 2, 2, 2, 0, 2, 2, 2, 2, 2, 0, 0, 1, 
1, 0, 0, 1, 0, 0, 3, 2, 4, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 11, 
1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 2, 5, 0, 3, 2, 0, 0, 6, 4, 0, 6, 4, 0, 0, 0, 6, 0, 0, 1, 
7, 2, 2, 4, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 7, 5, 0, 7, 5, 0, 0, 0, 8, 2, 2, 5, 
2, 2, 2, 5, 0, 0, 0, 0, 0, 1, 0, 1, 0, 6, 2, 0, 2, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 2, 2, 2, 
0, 0, 0, 0, 0, 0, 2, 2, 2, 11, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 6, 2, 4, 0, 0, 0, 0, 0, 0, 0, 0, 
2, 2, 2, 4, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 16, 0, 1, 0, 0, 0, 3, 2, 5, 0, 0, 0, 0, 6, 2, 2, 2, 
6, 2, 10, 5, 0, 0, 0, 0, 0, 0, 0, 1, 0, 7, 2, 2, 2, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 2, 2, 4, 
1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 10, 2, 2, 0, 2, 2, 5, 0, 0, 1, 
1, 0, 1, 0, 2, 2, 2, 2, 2, 4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 
1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 6, 2, 4, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 10, 2, 2, 2, 11, 
1, 0, 6, 2, 2, 2, 2, 2, 0, 1, 0, 0, 0, 0, 7, 2, 5, 0, 0, 0, 7, 2, 2, 2, 5, 0, 1, 1, 0, 0, 0, 1, 
1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 5, 0, 0, 0, 1, 
1, 0, 1, 0, 1, 0, 2, 2, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 4, 0, 0, 0, 0, 0, 0, 0, 2, 2, 11, 
1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 4, 0, 6, 4, 0, 6, 4, 0, 0, 0, 3, 2, 4, 0, 0, 0, 0, 0, 0, 0, 1, 
1, 0, 2, 2, 9, 2, 2, 0, 1, 0, 7, 5, 0, 1, 1, 0, 1, 1, 6, 4, 0, 0, 0, 3, 2, 2, 2, 2, 0, 2, 2, 11, 
1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 1, 6, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 
7, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 5, 1, 0, 1, 7, 9, 9, 9, 9, 2, 2, 2, 2, 2, 2, 2, 2, 2, 5, 
};