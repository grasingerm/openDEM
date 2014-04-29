#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>

enum dofs { X, Y, Z };

/*
struct point
{
    double x;
    double y;
}; */

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

#define DOF 2

struct particle
{
    double mass;
    double radius;
    double centroid[DOF];
    double velocity[DOF];
};

struct particle* alloc_particle(double mass, double radius, double centroid[],
    double velocity[])
{
    int i;

    struct particle* new_particle = malloc(sizeof(struct particle));
    if (new_particle == NULL) die("Memory allocation error");
    new_particle->mass = mass;
    new_particle->radius = radius;
    for (i = 0; i < DOF; i++)
    {
        new_particle->centroid[i] = centroid[i];
        new_particle->velocity[i] = velocity[i];
    }
}

void mmove_particle(struct particle* ppart, const double delta_time)
{
    int i;
    for (i = 0; i < DOF; i++)
        ppart->centroid[i] += ppart->velocity[i] * delta_time;
}

void maccelerate_particle(struct particle* ppart, const double delta_time,
    double acceleration[])
{
    int i;
    for (i = 0; i < DOF; i++)
        ppart->velocity[i] += acceleration[i] * delta_time;
}

#define NORM_2D(x, y) sqrt((x)*(x)+(y)*(y))
#define NORM_3D(x, y, z) sqrt((x)*(x)+(y)*(y)+(z)*(z))

/*
double overlap_particle(struct particle* pp1, struct particle* pp2)
{
    double e_12[DOF];
    double norm;
    #if DOF == 2
        norm = 2D_NORM( 
*/

int main(int argc, char* argv[])
{
    double c1[DOF] = {0.0, 0.0};
    double c2[DOF] = {0.0, 0.0};
    
    double v1[DOF] = {1.0, 0.0};
    double v2[DOF] = {0.0, 1.0};
    
    double a2[DOF] = {0.0, -0.1};
    
    struct particle* ppa = alloc_particle(12.1, 3.2, c1, v1);
    struct particle* ppb = alloc_particle(3.2, 1.0, c2, v2);
    
    const int iters = 10;
    const double delta_t = 0.5;
    double time = 0;
    int i;
    
    printf("%8s %8s %8s %8s %8s\n","time","x1","y1","x2","y2");
    for (i = 0; i < iters; i++)
    {
        printf("%8g %8g %8g %8g %8g\n", time, ppa->centroid[X], 
            ppa->centroid[Y], ppb->centroid[X], ppb->centroid[Y]);
        mmove_particle(ppa, delta_t);
        mmove_particle(ppb, delta_t);
        maccelerate_particle(ppb, delta_t, a2);
        time += delta_t;
    }
    
    free(ppa);
    free(ppb);
    
    return 0;
}
