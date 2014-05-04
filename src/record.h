#ifndef __RECORD_H

#define __RECORD_H 1

#include <sqlite3.h>
#include "particle.h"

void odem_init_results_db(sqlite3 *, sqlite3_stmt *);
int odem_exec_noselect_db(sqlite3 *, sqlite3_stmt *, const char*);
void odem_record_particle_data(sqlite3 *, sqlite3_stmt *,
    struct odem_particle_node* const);
    
#endif  /* __RECORD_H */
