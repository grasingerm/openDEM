#ifndef __RECORD_H

#define __RECORD_H 1

#include <sqlite3.h>
#include "particle.h"

void odem_init_results_db(sqlite3 *);
int odem_exec_noselect_db(sqlite3 *, const char*);
void odem_record_particle_data(sqlite3 *, struct odem_particle_node* const);
void odem_record_motion(sqlite3 *, const double, const int, 
    const struct odem_particle*, const double[], const double[]);
    
#endif  /* __RECORD_H */
