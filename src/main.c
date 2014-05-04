#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <math.h>
#include <errno.h>

#include "debug.h"
#include "particle.h"
#include "record.h"

/*
 * Main function
 */
int main(int argc, char* argv[])
{
    /* TODO: read model data in from a file */
    /* model data */
    double c1[ODEM_DOF] = {0.0, 5.0};
    double c2[ODEM_DOF] = {5.0, 0.0};
    double c3[ODEM_DOF] = {10.0, 0.0};
    double c4[ODEM_DOF] = {0.0, 10.0};

    double v1[ODEM_DOF] = {1.0, 0.0};
    double v2[ODEM_DOF] = {0.0, 1.0};
    double v3[ODEM_DOF] = {0.5, 0.0};
    double v4[ODEM_DOF] = {0.0, 0.7};

    struct odem_particle_node* ppart_list = odem_alloc_particle_node(
        odem_alloc_particle(12.1, 3.2, c1, v1));
    odem_mparticle_list_push(&ppart_list,
        odem_alloc_particle(3.2, 1.0, c2, v2));
    odem_mparticle_list_push(&ppart_list,
        odem_alloc_particle(3.2, 0.7, c3, v3));
    odem_mparticle_list_push(&ppart_list,
        odem_alloc_particle(3.2, 1.0, c4, v4));

    const int iters = 550;
    const double delta_time = 0.1;
    double bounds[2*ODEM_DOF] = {0.0, 20.0, 0.0, 20.0};

    sqlite3 *db;
    int rc;
    char *errmsg;
    #define BUFFER_SIZE 256
    char sqlite_msg[BUFFER_SIZE];
    size_t msg_size = sizeof(sqlite_msg);

    const char* data_file = "results.db";

    /* TODO: pass database file name as command-line argument */
    rc = sqlite3_open(data_file, &db);
    if (rc != SQLITE_OK) {
        snprintf(sqlite_msg, msg_size, "ERROR opening database: %s\n",
            sqlite3_errmsg(db));
        die(sqlite_msg);
    }

    struct odem_particle_node* curr_node;
    for (curr_node = ppart_list; curr_node != NULL; curr_node = curr_node->next)
        printf("%g %g\n", curr_node->ppart->mass, curr_node->ppart->radius);

    /* run analysis and write results to the database */
    printf("Initializing results database: %s\n", data_file);
    odem_init_results_db(db);
    odem_record_particle_data(db, ppart_list);
    odem_record_model_data(db, iters, delta_time, bounds);
    odem_run_analysis(db, ppart_list, bounds, iters, delta_time, 1);

    /* clean up */
    printf("Freeing dynamic memory...\n");
    sqlite3_close(db);
    odem_dealloc_particle_list(ppart_list);

    return 0;
}

