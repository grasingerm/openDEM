#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <gtk/gtk.h>
#include "debug.h"
#include "record.h"
#include "particle.h"

/* dimensions and grid size */
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define GRID_ROWS 10
#define GRID_COLS 1

/* Surface to store current scribbles */
static cairo_surface_t *surface = NULL;

static void clear_surface()
{
    cairo_t *cr;

    cr = cairo_create (surface);

    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    cairo_destroy (cr);
}

/* Create a new surface of the appropriate size to store our scribbles */
static gboolean configure_event_cb (GtkWidget *widget, GdkEventConfigure *event,
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
    cairo_set_source_surface(cr, surface, 0, 0);
    cairo_paint(cr);

    return FALSE;
}

/* Draw a rectangle on the surface at the given position */
static void draw_brush(GtkWidget *widget, gdouble x, gdouble y)
{
    cairo_t *cr;

    /* Paint to the surface, where we store our state */
    cr = cairo_create(surface);

    cairo_rectangle(cr, x - 3, y - 3, 6, 6);
    cairo_fill(cr);

    cairo_destroy(cr);

    /* Now invalidate the affected region of the drawing area. */
    gtk_widget_queue_draw_area(widget, x - 3, y - 3, 6, 6);
}

/* Handle button press events by either drawing a rectangle
 * or clearing the surface, depending on which button was pressed.
 * The ::button-press signal handler receives a GdkEventButton
 * struct which contains this information.
 */
static gboolean button_press_event_cb(GtkWidget *widget,
    GdkEventButton *event, gpointer data)
{
    /* paranoia check, in case we haven't gotten a configure event */
    if (surface == NULL)
        return FALSE;

    if (event->button == GDK_BUTTON_PRIMARY)
    {
        draw_brush(widget, event->x, event->y);
    }
    else if (event->button == GDK_BUTTON_SECONDARY)
    {
        clear_surface();
        gtk_widget_queue_draw(widget);
    }

    /* We've handled the event, stop processing */
    return TRUE;
}

/* Handle motion events by continuing to draw if button 1 is
 * still held down. The ::motion-notify signal handler receives
 * a GdkEventMotion struct which contains this information.
 */
static gboolean motion_notify_event_cb(GtkWidget *widget,
    GdkEventMotion *event, gpointer data)
{
    /* paranoia check, in case we haven't gotten a configure event */
    if (surface == NULL)
        return FALSE;

    if (event->state & GDK_BUTTON1_MASK)
        draw_brush(widget, event->x, event->y);

    /* We've handled it, stop processing */
    return TRUE;
}

static void close_window (void)
{
    if (surface)
        cairo_surface_destroy(surface);

    gtk_main_quit();
}

int main (int argc, char *argv[])
{
    GtkWidget *window;
    GtkWidget *da;
    GtkWidget *grid;
    GtkWidget *play_button;

    gtk_init(&argc, &argv);

    /* create a new window, and set its title */
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW (window), "Animation");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), WINDOW_WIDTH,
        WINDOW_HEIGHT);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);

    /* Here we construct the container that is going pack our buttons */
    grid = gtk_grid_new();
    play_button = gtk_button_new_with_label("play");
    da = gtk_drawing_area_new();

    /* set up grid */
    gtk_grid_attach(GTK_GRID(grid), play_button, 0, 0, 1, GRID_COLS);
    gtk_grid_attach(GTK_GRID(grid), da, 0, 1, 1, GRID_COLS);

    /* Signals used to handle the backing surface */
    g_signal_connect(da, "draw", G_CALLBACK (draw_cb), NULL);
    g_signal_connect(da, "configure-event", G_CALLBACK(configure_event_cb),
        NULL);

    /* Event signals */
    g_signal_connect(da, "motion-notify-event",
                    G_CALLBACK (motion_notify_event_cb), NULL);
    g_signal_connect(play_button, "button-press-event",
                    G_CALLBACK (button_press_event_cb), NULL);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}

