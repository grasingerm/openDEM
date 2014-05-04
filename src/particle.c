#include <stdlib.h>
#include <math.h>

#include "debug.h"
#include "particle.h"

/**
 * Allocate a particle on the heap
 *
 * @param mass Mass of the particle
 * @param radius Radius of the particle
 * @param centroid Coordinates of the particle centroid
 * @param velocity Components of the velocity vector
 * @return Pointer to a new particle
 */
struct odem_particle* odem_alloc_particle(const double mass, const double radius,
    const double centroid[], const double velocity[])
{
    int i;

    struct odem_particle* new_particle = malloc(sizeof(struct odem_particle));
    if (new_particle == NULL) die("Memory allocation error");
    new_particle->mass = mass;
    new_particle->radius = radius;
    for (i = 0; i < ODEM_DOF; i++)
    {
        new_particle->centroid[i] = centroid[i];
        new_particle->velocity[i] = velocity[i];
    }
}

/**
 * Allocate a particle node on the heap
 *
 * @param ppart Pointer to a particle
 * @return Pointer to a new particle node
 */
struct odem_particle_node* odem_alloc_particle_node(
    struct odem_particle* const ppart)
{
    struct odem_particle_node* new_node = malloc(sizeof(
        struct odem_particle_node));
    if (new_node == NULL) die("Memory allocation error.");
    new_node->next = NULL;
    new_node->ppart = ppart;
    return new_node;
}

/**
 * Push a particle node onto a linked list
 *
 * @param head Pointer to the head node
 * @param ppart Pointer to particle to be added
 */
void odem_mparticle_list_push(struct odem_particle_node** head,
    struct odem_particle* const ppart)
{
    struct odem_particle_node* new_node = odem_alloc_particle_node(ppart);
    new_node->next = *head;
    *head = new_node;
}

/**
 * Free memory from a particle linked list
 *
 * @param head Pointer to head node of list
 */
void odem_dealloc_particle_list(struct odem_particle_node* head)
{
    struct odem_particle_node *curr_node, *node_to_free;

    curr_node = head;
    while (curr_node != NULL)
    {
        node_to_free = curr_node;
        curr_node = curr_node->next;
        free(node_to_free->ppart);
        free(node_to_free);
    }
}

/**
 * Move a particle for a given time step, mutator
 *
 * @param ppart Pointer to particle to move
 * @param delta_time Time of step
 */
void odem_mmove_particle(struct odem_particle* ppart, const double delta_time)
{
    int i;
    for (i = 0; i < ODEM_DOF; i++)
        ppart->centroid[i] += ppart->velocity[i] * delta_time;
}

/**
 * Accelerate a particle for a given time step, mutator
 *
 * @param ppart Pointer to particle to accelerate
 * @param delta_time Time of step
 */
void odem_maccel_particle(struct odem_particle* ppart, const double delta_time,
    const double acceleration[])
{
    int i;
    for (i = 0; i < ODEM_DOF; i++)
        ppart->velocity[i] += acceleration[i] * delta_time;
}

/**
 * Unit vector normal to particle 1 in the direction of particle 2, mutator
 *
 * @param e12 Array to store unit ODEM_NORMal vector in
 * @param pp1 Pointer to particle 1
 * @param pp2 Pointer to particle 2
 */
void odem_me12(double e12[], const struct odem_particle* pp1,
    const struct odem_particle* pp2)
{
    int i;
    double ODEM_NORM;

    for (i = 0; i < ODEM_DOF; i++)
        e12[i] = pp2->centroid[i] - pp1->centroid[i];

    #if ODEM_DOF == 2
        ODEM_NORM = ODEM_NORM_2D_VEC(e12);
    #elif ODEM_DOF == 3
        ODEM_NORM = ODEM_NORM_3D_VEC(e12);
    #endif

    if (ODEM_NORM != 0)
    {
        for (i = 0; i < ODEM_DOF; i++)
            e12[i] /= ODEM_NORM;
    }
}

/**
 * Distance between two particles
 *
 * @param pp1 Pointer to particle 1
 * @param pp2 Pointer to particle 2
 * @return Distance between particles
 */
double odem_delta(const struct odem_particle* pp1, const struct odem_particle* pp2)
{
    #if ODEM_DOF == 2
        return ODEM_NORM_2D(pp1->centroid[X] - pp2->centroid[X],
            pp1->centroid[Y] - pp2->centroid[Y]) - pp1->radius - pp2->radius;
    #elif ODEM_DOF == 3
        return ODEM_NORM_3D(pp1->centroid[X] - pp2->centroid[X],
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
 * @return whether or not a collision has occurred
 */
int odem_mforce_collision_spring(double force_vec[],
    const struct odem_particle* pp1, const struct odem_particle* pp2,
    const double spring_constant)
{
    int i;
    double delta;

    delta = odem_delta(pp1, pp2);
    if (delta < 0)
    {
        odem_me12(force_vec, pp1, pp2);
        for (i = 0; i < ODEM_DOF; i++)
            force_vec[i] *= delta * spring_constant;
        return 1;
    }
    else
    {
        for (i = 0; i < ODEM_DOF; i++)
            force_vec[i] = 0;
        return 0;
    }
}

/**
 * Particle-boundary collision model, spring; mutator
 *
 * @param force_vec Array to store force vector in
 * @param ppart Pointer to particle
 * @param bounds Array containing boundaries
 * @param spring_constant Spring constant, k
 * @return whether or not a collision has occurred
 */
int odem_mforce_boundary_collision_spring(double force_vec[],
    const struct odem_particle* ppart, const double bounds[],
    const double spring_constant)
{
    int i, collision = 0;
    double delta;

    for (i = 0; i < ODEM_DOF; i++)
    {
        /* check for collision at min dof boundary */
        delta = ppart->centroid[i] - bounds[2*i] - ppart->radius;
        if (delta < 0)
        {
            collision = 1;
            force_vec[i] = -delta * spring_constant;
            continue;
        }

        /* check for collision at max dof boundary */
        delta = bounds[2*i+1] - ppart->centroid[i] - ppart->radius;
        if (delta < 0)
        {
            collision = 1;
            force_vec[i] = delta * spring_constant;
        }
    }

    return collision;
}

