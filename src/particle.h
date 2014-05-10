#ifndef __PARTICLE_H

#define __PARTICLE_H 1

enum odem_ODEM_DOF { X, Y, Z };

#define ODEM_DOF 2

#define ODEM_NORM_2D(x, y) sqrt((x)*(x)+(y)*(y))
#define ODEM_NORM_3D(x, y, z) sqrt((x)*(x)+(y)*(y)+(z)*(z))

#define ODEM_NORM_2D_VEC(v) ODEM_NORM_2D(v[X], v[Y])
#define ODEM_NORM_3D_VEC(v) ODEM_NORM_3D(v[X], v[Y], v[Z])

// data structures

/**
 * Particle structure
 *
 * @member mass Particle mass
 * @member radius Particle radius
 * @memeber centroid Coordinates of the particle centroid
 * @member velocity Components of the velocity vector
 */
struct odem_particle
{
    double mass;
    double radius;
    double centroid[ODEM_DOF];
    double velocity[ODEM_DOF];
};

/**
 * Particle list node
 *
 * @member ppart Pointer to the particle
 * @member next Link to next node
 */
struct odem_particle_node
{
    struct odem_particle* ppart;
    struct odem_particle_node* next;
};


// function interfaces
struct odem_particle* odem_alloc_particle(const double, const double, const double[],
    const double[]);
void odem_mmove_particle(struct odem_particle*, const double);
void odem_maccel_particle(struct odem_particle*, const double, const double[]);
void odem_me12(double[], const struct odem_particle*, const struct odem_particle*);
double odem_delta(const struct odem_particle*, const struct odem_particle*);
int odem_mforce_collision_spring(double[], const struct odem_particle*,
    const struct odem_particle*, const double);
int odem_mforce_boundary_collision_spring(double[], const struct odem_particle*,
    const double[], const double);

struct odem_particle_node* odem_alloc_particle_node(struct odem_particle* const);
void odem_mparticle_list_push(struct odem_particle_node**,
    struct odem_particle* const);
void odem_dealloc_particle_list(struct odem_particle_node*);

#endif  /* __PARTICLE_H */

