#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <pthread.h>

#define WINDOW_WIDTH  100
#define WINDOW_HEIGHT 100

static gboolean draw_cb(GtkWidget *widget, cairo_t *cr, gpointer data)
{
    cairo_set_source_rgb(cr,0,0,0);
    cairo_paint(cr);

    printf("Draw\n");
    
    return FALSE;
}

void *calc_thread(GtkWidget *draw_area)
{
    while(TRUE)
    {
        usleep(100000);

        printf("Thread tick\n");

        gdk_threads_enter();
        gtk_widget_queue_draw(draw_area);
        gdk_threads_leave();
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    GtkWidget *window;
    GtkWidget *draw_area;
    pthread_t calc_tid;

    gdk_threads_init();
    gdk_threads_enter();

    gtk_init(&argc,&argv);

    window= gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(window,"destroy",G_CALLBACK(gtk_main_quit),NULL);

    draw_area= gtk_drawing_area_new();
    gtk_widget_set_size_request(draw_area,WINDOW_WIDTH,WINDOW_HEIGHT);
    g_signal_connect(draw_area,"draw",G_CALLBACK(draw_cb),NULL);

    gtk_container_add(GTK_CONTAINER(window),draw_area);
    gtk_widget_show(draw_area);
    gtk_widget_show (window);

    pthread_create(&calc_tid,NULL,(void *(*)(void *))calc_thread,draw_area);

    gtk_main();
    gdk_threads_leave();

    return 0;
}
