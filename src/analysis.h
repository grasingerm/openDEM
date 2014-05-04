#ifndef __ANALYSIS_H

#define __ANALYSIS_H 1

void odem_run_analysis(sqlite3 *, struct odem_particle_node* const,
    const double[], const int, const double, const int);

#endif  /* __ANALYSIS_H */

