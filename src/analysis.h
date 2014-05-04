#ifndef __ANALYSIS_H

#define __ANALYSIS_H 1

void odem_run_analysis(sqlite3 *, sqlite3_stmt*, 
    struct odem_particle_node* const, const int, const double, const int);

#endif  /* __ANALYSIS_H */
