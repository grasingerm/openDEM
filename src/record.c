#include <stdio.h>
#include <sqlite3.h>
#include "debug.h"
#include "record.h"

/*
int odem_has_table(sqlite3 *db, const char* table_name)
{
    #define HAS_TABLE_SQL "SELECT
}
*/

/**
 * Execute a noselect statement
 *
 * @param db Database connection
 * @param stmt sqlite3 statement
 * @param sql Query to execute
 * @return sqlite3 code
 */
int odem_exec_noselect_db(sqlite3 *db, const char* sql)
{
    int rc;
    char msg[100];
    sqlite3_stmt* stmt;
    
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    rc = sqlite3_step(stmt);
    
    if (rc != SQLITE_DONE)
    {
        snprintf(msg, sizeof(msg), "ERROR: %s\n", sqlite3_errmsg(db));
        die(msg);
    }
    
    sqlite3_finalize(stmt);
    
    return rc;
}

/**
 * Initialize a odem results database
 *
 * @param db Database connection
 * @param stmt sqlite3 statement
 * @return sqlite3 code
 */
void odem_init_results_db(sqlite3 *db)
{   
    odem_exec_noselect_db(db, "PRAGMA foreign_keys = ON");
    
    odem_exec_noselect_db(db, "CREATE TABLE particle"
        "(particle_id INTEGER PRIMARY KEY AUTOINCREMENT, mass REAL,"
        " radius REAL)");
    
    #if ODEM_DOF == 3
        odem_exec_noselect_db(db, 
            "CREATE TABLE motion (time REAL, particle_id INTEGER, x REAL,"
            " y REAL, z REAL, v_x REAL, v_y REAL, v_z REAL, a_x REAL, a_y REAL,"
            " a_z REAL, f_x REAL, f_y REAL, f_z REAL,"
            " FOREIGN KEY(particle_id) REFERENCES particle(particle_id))");
    #elif ODEM_DOF == 2
        odem_exec_noselect_db(db, 
            "CREATE TABLE motion (time REAL, particle_id INTEGER, x REAL,"
            " y REAL, v_x REAL, v_y REAL, a_x REAL, a_y REAL, f_x REAL,"
            " f_y REAL, FOREIGN KEY(particle_id) REFERENCES"
            " particle(particle_id))");
    #endif
    odem_exec_noselect_db(db, 
        "CREATE INDEX time_particle_id_idx ON motion (time, particle_id)");
}

/**
 * Record particle attributes
 *
 * @param db Database connection
 * @param stmt sqlite3 statement
 * @param ppart_list Particle linked list
 */
void odem_record_particle_data(sqlite3 *db, struct odem_particle_node* 
    const ppart_list)
{
    char sql[256];
    
    struct odem_particle_node* curr_node;
    
    size_t sql_size = sizeof(sql);
    
    for (curr_node = ppart_list; curr_node != NULL; curr_node = curr_node->next)
    {
        snprintf(sql, sql_size, "INSERT INTO particle VALUES (NULL, %lf, %lf)",
            curr_node->ppart->mass, curr_node->ppart->radius);
        odem_exec_noselect_db(db, sql);
    }
}

void odem_record_motion(sqlite3 *db, const double time,
    const int particle_id, const struct odem_particle* ppart, 
    const double accel_vec[], const double force_vec[])
{
    char sql[512];
    
    #if ODEM_DOF == 3
        snprintf(sql, sizeof(sql), "INSERT INTO motion"
            " VALUES (%lf, %d, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf,"
            " %lf, %lf)",
            time, particle_id, ppart->centroid[X], ppart->centroid[Y],
            ppart->centroid[Z], ppart->velocity[X], ppart->velocity[Y],
            accel_vec[X], accel_vec[Y], accel_vec[Z], force_vec[X], 
            force_vec[Y], force_vec[Z]);
    #elif ODEM_DOF == 2
        snprintf(sql, sizeof(sql), "INSERT INTO motion"
            " VALUES (%lf, %d, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf)",
            time, particle_id, ppart->centroid[X], ppart->centroid[Y],
            ppart->velocity[X], ppart->velocity[Y], accel_vec[X], accel_vec[Y],
            force_vec[X], force_vec[Y]);
    #endif
    
    odem_exec_noselect_db(db, sql);
}
