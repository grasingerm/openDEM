#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <math.h>
#include <errno.h>

#include "particle.h"

/**
 * Exit the program and print error message
 *
 * @param message Error message
 */
void die(const char *message)
{
    if(errno)
    {
        perror(message);
    }
    else
    {
        printf("ERROR: %s\n", message);
    }

    exit(1);
}

/*
 * Main function
 */
int main(int argc, char* argv[])
{
    double c1[ODEM_DOF] = {0.0, 5.0};
    double c2[ODEM_DOF] = {5.0, 0.0};
    
    double v1[ODEM_DOF] = {1.0, 0.0};
    double v2[ODEM_DOF] = {0.0, 1.0};
    
    double a1[ODEM_DOF];
    double a2[ODEM_DOF];
    
    double e12[ODEM_DOF];
    double f12[ODEM_DOF] = {0.0, 0.0};
    
    struct odem_particle* ppa = odem_alloc_particle(12.1, 3.2, c1, v1);
    struct odem_particle* ppb = odem_alloc_particle(3.2, 1.0, c2, v2);
    
    const int iters = 100;
    const double delta_time = 0.1;
    double time = 0;
    int i, j;
    
    int const k = 10;
    
    printf("%8s %8s %8s %8s %8s %8s %8s\n","time","x1","y1","x2","y2", "f1", "f2");
    for (i = 0; i < iters; i++)
    {
        printf("%8g %8g %8g %8g %8g %8g %8g\n", time, ppa->centroid[X], 
            ppa->centroid[Y], ppb->centroid[X], ppb->centroid[Y], f12[X],
            f12[Y]);
            
        odem_mmove_particle(ppa, delta_time);
        odem_mmove_particle(ppb, delta_time);
        odem_mforce_collision_spring(f12, ppa, ppb, k);
        for (j = 0; j < ODEM_DOF; j++)
        {
            a1[j] = f12[j]/ppa->mass;
            a2[j] = -f12[j]/ppb->mass;
        }
        odem_maccel_particle(ppa, delta_time, a1);
        odem_maccel_particle(ppb, delta_time, a2);
        
        time += delta_time;
    }
    
    free(ppa);
    free(ppb);
    
    return 0;
}
