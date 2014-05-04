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
    
    struct odem_particle_node* ppart_list = odem_alloc_particle_node(ppa);
    odem_mparticle_list_push(&ppart_list, ppb);
    
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
    
    struct odem_particle_node* curr_node;
    printf("%8s %8s\n", "mass", "radius");
    for (curr_node = ppart_list; curr_node != NULL; 
        curr_node = curr_node->next)
        printf("%8g %8g\n", curr_node->ppart->mass, curr_node->ppart->radius);
    
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;
    char *errmsg;
    #define BUFFER_SIZE 256
    char sqlite_msg[BUFFER_SIZE];
    size_t msg_size = sizeof(sqlite_msg);
    
    /*
     * open SQLite database file test.db
     * use ":memory:" to use an in-memory database
     */
    rc = sqlite3_open("results.db", &db);
    if (rc != SQLITE_OK) {
        snprintf(sqlite_msg, msg_size, "ERROR opening database: %s\n", 
            sqlite3_errmsg(db));
        die(sqlite_msg);
    }
    
    odem_init_results_db(db, stmt);
    odem_record_particle_data(db, stmt, ppart_list);
    
    sqlite3_finalize(stmt);
    odem_dealloc_particle_list(ppart_list);
    
    return 0;
}
