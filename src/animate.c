#include <gtk/gtk.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <stdio.h>

#include "particle.h"
#include "debug.h"

/* database columns */
enum odem_model_col
{
    ITERS,
    DELTA_TIME,
    X_MIN,
    X_MAX,
    Y_MIN,
    Y_MAX
};

enum odem_particle_col
{
    PARTICLE_ID,
    MASS,
    RADIUS
};

/* set up data structures for animation */
struct particle_data_node
{
    double mass;
    double radius;
    struct particle_data_node* next;
};

/* singly linked lists are the best! */
struct particle_data_node* alloc_particle_data_node(const double mass,
    const double radius)
{
    struct particle_data_node* new_node = (struct particle_data_node*)malloc(
        sizeof(struct particle_data_node));
    if (new_node == NULL) die("Memory allocation error.");
    new_node->next = NULL;
    new_node->mass = mass;
    new_node->radius = radius;
    return new_node;
}

void mparticle_data_push(struct particle_data_node** head, const double mass,
    const double radius)
{
    struct particle_data_node* new_node = alloc_particle_data_node(mass,
        radius);
    new_node->next = *head;
    *head = new_node;
}

void dealloc_particle_data_list(struct particle_data_node* head)
{
    struct particle_data_node *curr_node, *node_to_free;

    curr_node = head;
    while (curr_node != NULL)
    {
        node_to_free = curr_node;
        curr_node = curr_node->next;
        free(node_to_free);
    }
}

/* animation data and logic */
static sqlite3_stmt* glob_stmt;
static cairo_surface_t *surface = NULL;
static int some_number = 0;

static void clear_surface()
{
    cairo_t *cr;

    cr = cairo_create(surface);

    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    cairo_destroy(cr);
}

/* Create a new surface of the appropriate size to store our scribbles */
static gboolean configure_event_cb(GtkWidget *widget, GdkEventConfigure *event,
    gpointer data)
{
    if (surface)
        cairo_surface_destroy (surface);

    surface = gdk_window_create_similar_surface(gtk_widget_get_window(widget),
        CAIRO_CONTENT_COLOR, gtk_widget_get_allocated_width(widget),
        gtk_widget_get_allocated_height(widget));

    /* Initialize the surface to white */
    clear_surface();

    /* We've handled the configure event, no need for further processing. */
    return TRUE;
}

/* Redraw the screen from the surface. Note that the ::draw
 * signal receives a ready-to-be-used cairo_t that is already
 * clipped to only draw the exposed areas of the widget
 */
static gboolean draw_cb(GtkWidget *widget, cairo_t *cr, gpointer data)
{
    int rc;
    char buffer[256];
    size_t buff_size = sizeof(buffer);

    printf("Draw event fired. %d\n", some_number);

    clear_surface();

    if ( (rc = sqlite3_step(glob_stmt)) == SQLITE_ROW)
    {
        some_number++;
        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
          CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(cr, 40.0);

        cairo_move_to(cr, 10.0, 50.0);
        snprintf(buffer, buff_size, "Disziplin ist Macht. %d", some_number);
        cairo_show_text(cr, buffer);
    }

    return FALSE;
}

/* Draw a rectangle on the surface at the given position */
static gboolean da_click(GtkWidget *widget, GdkEventButton *event,
    gpointer data)
{
    printf("Click event fired.\n");

    /* paranoia check, in case we haven't gotten a configure event */
    if (surface == NULL)
        return FALSE;

    //if (event->button == GDK_BUTTON_PRIMARY)
        gtk_widget_queue_draw(widget);

    /* We've handled the event, stop processing */
    return TRUE;
}


/* main program */
int main(int argc, char* argv[])
{
    sqlite3* db;
    int rc;
    char* errmsg;
    #define BUFFER_SIZE 256
    char sqlite_msg[BUFFER_SIZE];
    size_t msg_size = sizeof(sqlite_msg);

    const char* data_file = "results.db";

    /* TODO: pass database file name as command-line argument */
    rc = sqlite3_open(data_file, &db);
    if (rc != SQLITE_OK)
    {
        snprintf(sqlite_msg, msg_size, "ERROR opening database: %s\n",
            sqlite3_errmsg(db));
        die(sqlite_msg);
    }

    /* read in model data */
    sqlite3_stmt *stmt;
    double delta_time;
    int iters;
    double bounds[2*ODEM_DOF];

    sqlite3_prepare_v2(db, "SELECT * FROM model", -1, &stmt, NULL);
    if ( (rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        iters = sqlite3_column_int(stmt, ITERS);
        delta_time = sqlite3_column_double(stmt, DELTA_TIME);
        bounds[2*X] = sqlite3_column_double(stmt, X_MIN);
        bounds[2*X + 1] = sqlite3_column_double(stmt, X_MAX);
        bounds[2*Y] = sqlite3_column_double(stmt, Y_MIN);
        bounds[2*Y + 1] = sqlite3_column_double(stmt, Y_MAX);
    }

    sqlite3_finalize(stmt);

    /* print model data */
    printf("Model data...\n");
    printf("%5s %8s %8s %8s %8s %8s\n", "iters", "delta_t", "x_min",
        "x_max", "y_min", "y_max");
    printf("%5d %8g %8g %8g %8g %8g\n", iters, delta_time, bounds[0], bounds[1],
        bounds[2], bounds[3]);
    printf("\n");

    /* read in particle data */
    struct particle_data_node* ppart_data_list;

    sqlite3_prepare_v2(db, "SELECT * FROM particle", -1, &stmt, NULL);
    if ( (rc = sqlite3_step(stmt)) == SQLITE_ROW)
        ppart_data_list = alloc_particle_data_node(
            sqlite3_column_double(stmt, MASS),
            sqlite3_column_double(stmt, RADIUS));
    while ( (rc = sqlite3_step(stmt)) == SQLITE_ROW)
        mparticle_data_push(&ppart_data_list, sqlite3_column_double(stmt, MASS),
            sqlite3_column_double(stmt, RADIUS));

    sqlite3_finalize(stmt);

    /* print particle data */
    struct particle_data_node* curr_node;
    printf("Particle data...\n");
    printf("%8s %8s\n", "Mass", "Radius");
    for (curr_node = ppart_data_list; curr_node != NULL;
        curr_node = curr_node->next)
        printf("%8g %8g\n", curr_node->mass, curr_node->radius);
    printf("\n");

    /* set up GUI and animation */
    GtkWidget *window;
    GtkWidget *da;

    gtk_init(&argc, &argv);

    #define WINDOW_WIDTH 800
    #define WINDOW_HEIGHT 600
    /* create a new window, and set its title */
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW (window), "Animation");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), WINDOW_WIDTH,
        WINDOW_HEIGHT);

    da = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(window), da);

    /* set events */
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(da, "draw", G_CALLBACK (draw_cb), NULL);
    /*g_signal_connect(da, "configure-event", G_CALLBACK(configure_event_cb),
        NULL);*/
    g_signal_connect(da, "button-press-event", G_CALLBACK(da_click), NULL);
    /* Ask to receive events the drawing area doesn't normally
    * subscribe to. In particular, we need to ask for the
    * button press and motion notify events that want to handle.
    */
    gtk_widget_set_events (da, gtk_widget_get_events (da)
                             | GDK_BUTTON_PRESS_MASK
                             | GDK_POINTER_MOTION_MASK);


    /* prepare statement */
    sqlite3_prepare_v2(db, "SELECT * FROM motion", -1, &glob_stmt, NULL);

    /* start mainloop */
    gtk_widget_show_all(window);
    gtk_main();

    /* clean up */
    printf("Freeing dynamic memory...\n");
    sqlite3_finalize(glob_stmt);
    sqlite3_close(db);
    dealloc_particle_data_list(ppart_data_list);

    return 0;
}

