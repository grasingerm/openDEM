#include <cairo.h>
#include <gtk/gtk.h>
#include <unistd.h>
#include <pthread.h>

#define _USE_MATH_DEFINES
#include <math.h>

static void do_drawing(cairo_t *);

static gboolean on_draw_event(GtkWidget *widget, cairo_t *cr,
    gpointer user_data)
{
  do_drawing(cr);

  return FALSE;
}

static void do_drawing(cairo_t *cr)
{
  cairo_set_source_rgb(cr, 0, 0, 256);

  cairo_arc (cr, 128.0, 128.0, 76.8, 0, 2 * M_PI);
  cairo_fill(cr);
}

static gboolean clicked(GtkWidget *widget, GdkEventButton *event,
    gpointer user_data)
{
    if (event->button == 1) {
        gtk_widget_queue_draw(widget);
    }

    return TRUE;
}

/*
gboolean timer_exe(GtkWidget *widget)
{
    static gboolean first_execution = TRUE;

    //use a safe function to get the value of currently_drawing so
    //we don't run into the usual multithreading issues
    int drawing_status = g_atomic_int_get(&currently_drawing);

    //if we are not currently drawing anything, launch a thread to
    //update our csurf
    if(drawing_status == 0){
        static pthread_t thread_info;
        int  iret;
        if(first_execution != TRUE){
            pthread_join(thread_info, NULL);
        }
        iret = pthread_create( &thread_info, NULL, do_draw, NULL);
    }

    //tell our window it is time to draw our animation.
    gtk_widget_queue_draw_area(widget);

    first_execution = FALSE;

    return TRUE;
}
*/

#define WINDOW_HEIGHT 800
#define WINDOW_WIDTH 600

int main(int argc, char *argv[])
{
  GtkWidget *window;
  GtkWidget *darea;

  gtk_init(&argc, &argv);

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  darea = gtk_drawing_area_new();
  gtk_container_add(GTK_CONTAINER(window), darea);

  gtk_widget_add_events(window, GDK_BUTTON_PRESS_MASK);

  g_signal_connect(G_OBJECT(darea), "draw", G_CALLBACK(on_draw_event), NULL);
  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  g_signal_connect(window, "button-press-event", G_CALLBACK(clicked), NULL);

  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(GTK_WINDOW(window), WINDOW_HEIGHT, WINDOW_WIDTH);
  gtk_window_set_title(GTK_WINDOW(window), "Particle Simulation");

  gtk_widget_show_all(window);

  gtk_main();

  return 0;
}

