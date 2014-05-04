#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <time.h>

#include "debug.h"
#include "particle.h"
#include "record.h"
#include "analysis.h"

/* local data structure */

struct double_node
{
    double data[ODEM_DOF];
    struct double_node* next;
};

struct double_node* alloc_double_node(double data[])
{
    int i;

    struct double_node* new_node = (struct double_node*)malloc(sizeof(
        struct double_node));
    if (new_node == NULL) die("Memory allocation error");
    new_node->next = NULL;
    for (i = 0; i < ODEM_DOF; i++)
        new_node->data[i] = data[i];
    return new_node;
}

void double_push(struct double_node** double_list, double data[])
{
    struct double_node* new_node = alloc_double_node(data);
    new_node->next = *double_list;
    *double_list = new_node;
}

void set_double_data(struct double_node* curr_double_node, double data[])
{
    int i;
    for (i = 0; i < ODEM_DOF; i++)
        curr_double_node->data[i] = data[i];
}

/* analysis logic */

void odem_run_analysis(sqlite3 *db, struct odem_particle_node* const ppart_list,
    const double bounds[], const int iters, const double delta_time,
    const int verbose)
{
    printf("Starting analysis...\n");

    /* set up profile */
    clock_t begin, end;
    double time_spent;
    begin = clock();

    /* set up data structures */
    /* TODO: fix this heuristic */
    const double k = 10.0;

    int i, j, particle_id, collisions;
    double time = 0.0;
    struct odem_particle_node *node_i, *node_j;
    struct double_node* double_list;
    double_list = alloc_double_node(ppart_list->ppart->velocity);
    for (node_i = ppart_list->next; node_i != NULL; node_i = node_i->next)
        double_push(&double_list, node_i->ppart->velocity);
    struct double_node* curr_double_node;

    #if ODEM_DOF == 2
        double force_vec[ODEM_DOF] = {0.0, 0.0};
        double accel_vec[ODEM_DOF] = {0.0, 0.0};
    #elif ODEM_DOF == 3
        double force_vec[ODEM_DOF] = {0.0, 0.0, 0.0};
        double accel_vec[ODEM_DOF] = {0.0, 0.0, 0.0};
    #endif

    /* main analysis */
    for (i = 0; i < iters; i++)
    {
        /* move each particle for time step */
        for (node_i = ppart_list; node_i != NULL; node_i = node_i->next)
            odem_mmove_particle(node_i->ppart, delta_time);

        /* check particles for boundary collisions */
        for (node_i = ppart_list; node_i != NULL; node_i = node_i->next)
        {
            if (odem_mforce_boundary_collision_spring(force_vec, node_i->ppart,
                bounds, k))
            {
                collisions++;
                /* accelerate particle */
                for (j = 0; j < ODEM_DOF; j++)
                    accel_vec[j] = force_vec[j]/node_i->ppart->mass;
                odem_maccel_particle(node_i->ppart, delta_time, accel_vec);
            }
        }

        /* check particles for collisions */
        collisions = 0;
        for (node_i = ppart_list; node_i != NULL; node_i = node_i->next)
        {
            for (node_j = node_i->next; node_j != NULL; node_j = node_j->next)
            {
                if (odem_mforce_collision_spring(force_vec, node_i->ppart,
                    node_j->ppart, k))
                    collisions++;

                /* accelerate first particle */
                for (j = 0; j < ODEM_DOF; j++)
                    accel_vec[j] = force_vec[j]/node_i->ppart->mass;
                odem_maccel_particle(node_i->ppart, delta_time, accel_vec);

                /* accelerate second particle in opposite direction */
                for (j = 0; j < ODEM_DOF; j++)
                    accel_vec[j] = -force_vec[j]/node_j->ppart->mass;
                odem_maccel_particle(node_j->ppart, delta_time, accel_vec);
            }
        }

        /* display info */
        if(verbose) printf("\titer: %d, collisions: %d\n", i, collisions);

        /* increment time */
        time += delta_time;

        /* write data */
        double acceleration[ODEM_DOF];
        double force[ODEM_DOF];
        for (particle_id = 1, node_i = ppart_list,
                curr_double_node = double_list;
            node_i != NULL && curr_double_node != NULL;
            particle_id++, node_i = node_i->next,
                curr_double_node = curr_double_node->next)
        {
            for (j = 0; j < ODEM_DOF; j++)
            {
                acceleration[j] = curr_double_node->data[j] -
                    node_i->ppart->velocity[j];
                force[j] = node_i->ppart->mass * acceleration[j];
            }
            odem_record_motion(db, time, particle_id, node_i->ppart,
                acceleration, force);
        }
    }

    /* clean up data structures */
    struct double_node* node_to_clean;
    curr_double_node = double_list;
    while (curr_double_node != NULL)
    {
        node_to_clean = curr_double_node;
        curr_double_node = curr_double_node->next;
        free(node_to_clean);
    }

    /* display profile result */
    end = clock();
    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Analysis completed in %g seconds.\n", time_spent);
}

