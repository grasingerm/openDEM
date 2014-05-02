#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <math.h>
#include <errno.h>

enum dofs { X, Y, Z };

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

#define DOF 2

#define NORM_2D(x, y) sqrt((x)*(x)+(y)*(y))
#define NORM_3D(x, y, z) sqrt((x)*(x)+(y)*(y)+(z)*(z))

#define NORM_2D_VEC(v) NORM_2D(v[X], v[Y])
#define NORM_3D_VEC(v) NORM_3D(v[X], v[Y], v[Z])

/**
 * Particle structure
 *
 * @member mass Particle mass
 * @member radius Particle radius
 * @memeber centroid Coordinates of the particle centroid
 * @member velocity Components of the velocity vector
 */
struct particle
{
    double mass;
    double radius;
    double centroid[DOF];
    double velocity[DOF];
};

/**
 * Allocate a particle on the heap
 *
 * @param mass Mass of the particle
 * @param radius Radius of the particle
 * @param centroid Coordinates of the particle centroid
 * @param velocity Components of the velocity vector
 * @return Pointer to a new particle
 */
struct particle* alloc_particle(const double mass, const double radius, 
    const double centroid[], const double velocity[])
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

/**
 * Move a particle for a given time step, mutator
 *
 * @param ppart Pointer to particle to move
 * @param delta_time Time of step
 */
void mmove_particle(struct particle* ppart, const double delta_time)
{
    int i;
    for (i = 0; i < DOF; i++)
        ppart->centroid[i] += ppart->velocity[i] * delta_time;
}

void maccelerate_particle(struct particle* ppart, const double delta_time,
    const double acceleration[])
{
    int i;
    for (i = 0; i < DOF; i++)
        ppart->velocity[i] += acceleration[i] * delta_time;
}

/**
 * Unit vector normal to particle 1 in the direction of particle 2, mutator
 *
 * @param e12 Array to store unit normal vector in
 * @param pp1 Pointer to particle 1
 * @param pp2 Pointer to particle 2
 */
void me12_particle(double e12[], const struct particle* pp1, 
    const struct particle* pp2)
{
    int i;
    double norm;
    
    for (i = 0; i < DOF; i++)
        e12[i] = pp2->centroid[i] - pp1->centroid[i];
        
    #if DOF == 2
        norm = NORM_2D_VEC(e12);
    #elif DOF == 3
        norm = NORM_3D_VEC(e12);
    #endif
    
    if (norm != 0)
    {
        for (i = 0; i < DOF; i++)
            e12[i] /= norm;
    }
}

/**
 * Distance between two particles
 *
 * @param pp1 Pointer to particle 1
 * @param pp2 Pointer to particle 2
 * @return Distance between particles
 */
double delta_particle(const struct particle* pp1, const struct particle* pp2)
{
    #if DOF == 2
        return NORM_2D(pp1->centroid[X] - pp2->centroid[X],
            pp1->centroid[Y] - pp2->centroid[Y]) - pp1->radius - pp2->radius;
    #elif DOF == 3
        return NORM_3D(pp1->centroid[X] - pp2->centroid[X],
            pp1->centroid[Y] - pp2->centroid[Y],
            pp1->centroid[Z] - pp2->centroid[Z],) - pp1->radius - pp2->radius;
    #endif
}

/**
 * Particle-particle collision model, spring; mutator
 *
 * @param force_vec Array to store force vector in
 * @param pp1 Pointer to particle 1
 * @param pp2 Pointer to particle 2
 * @param spring_constant Spring constant, k
 */
void mforce_collision_spring_particle(double force_vec[], 
    const struct particle* pp1, const struct particle* pp2, 
    const double spring_constant)
{
    int i;
    double delta;

    delta = delta_particle(pp1, pp2);    
    if (delta < 0)
    {
        me12_particle(force_vec, pp1, pp2);
        for (i = 0; i < DOF; i++)
            force_vec[i] *= delta * spring_constant;
    }
    else
    {
        for (i = 0; i < DOF; i++)
            force_vec[i] = 0;
    }
}

/*
 * Main function
 */
int main(int argc, char* argv[])
{
    double c1[DOF] = {0.0, 5.0};
    double c2[DOF] = {5.0, 0.0};
    
    double v1[DOF] = {1.0, 0.0};
    double v2[DOF] = {0.0, 1.0};
    
    double a1[DOF];
    double a2[DOF];
    
    double e12[DOF];
    double f12[DOF] = {0.0, 0.0};
    
    struct particle* ppa = alloc_particle(12.1, 3.2, c1, v1);
    struct particle* ppb = alloc_particle(3.2, 1.0, c2, v2);
    
    const int iters = 100;
    const double delta_t = 0.1;
    double time = 0;
    int i, j;
    
    int const k = 10;
    
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
  
    if( argc!=2 ){
      fprintf(stderr, "Usage: %s OUTFILE\n", argv[0]);
      return(1);
    }
    rc = sqlite3_open(argv[1], &db);
    if( rc ){
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      sqlite3_close(db);
      return(1);
    }
    rc = sqlite3_exec(db, argv[2], callback, 0, &zErrMsg);
    if( rc!=SQLITE_OK ){
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
    }
    sqlite3_close(db);
    
    printf("%8s %8s %8s %8s %8s %8s %8s\n","time","x1","y1","x2","y2", "f1", "f2");
    for (i = 0; i < iters; i++)
    {
        printf("%8g %8g %8g %8g %8g %8g %8g\n", time, ppa->centroid[X], 
            ppa->centroid[Y], ppb->centroid[X], ppb->centroid[Y], f12[X],
            f12[Y]);
            
        mmove_particle(ppa, delta_t);
        mmove_particle(ppb, delta_t);
        mforce_collision_spring_particle(f12, ppa, ppb, k);
        for (j = 0; j < DOF; j++)
        {
            a1[j] = f12[j]/ppa->mass;
            a2[j] = -f12[j]/ppb->mass;
        }
        maccelerate_particle(ppa, delta_t, a1);
        maccelerate_particle(ppb, delta_t, a2);
        
        time += delta_t;
    }
    
    free(ppa);
    free(ppb);
    
    return 0;
}
