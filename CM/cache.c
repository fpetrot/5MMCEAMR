/* Petit programme avec un nid de boucle */
#include <stdint.h>
volatile double x[1024][1024] = {{1.,1.}};
int main(void)
{
   int32_t i, j, k;
   /* On répète 1024 fois pour avoir un temps significatif */
   for (k = 0; k < 1024; k++)
           for (j = 0; j < 1024; j++) /* j : indice des lignes */
       for (i = 0; i < 1024; i++)   /* i : indice des colonnes */
            x[i][j] = 2 * x[i][j]+k;
   return 0;
}
