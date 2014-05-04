#include <gtk/gtk.h>
#include <cairo.h>
#include <unistd.h>
#include <pthread.h>

//the global csurf that will serve as our buffer
static cairo_surface_t *csurf = NULL;

gboolean on_window_configure_event(GtkWidget * da, GdkEventConfigure * event, gpointer user_data){
    static int oldw = 0;
    static int oldh = 0;
    //make our selves a properly sized csurf if our window has been resized
    if (oldw != event->width || oldh != event->height){
        //create our new csurf with the correct size.
        cairo_surface_t *tmpcsurf = cairo_image_surface_create(CAIRO_FORMAT_A1, event->width,  event->height);
        //copy the contents of the old csurf to the new csurf.  This keeps ugly uninitialized
        //csurfs from being painted upon resize
        int minw = oldw, minh = oldh;
        if( event->width < minw ){ minw =  event->width; }
        if( event->height < minh ){ minh =  event->height; }
        gdk_draw_drawable(tmpcsurf, da->style->fg_gc[GTK_WIDGET_STATE(da)], csurf, 0, 0, 0, 0, minw, minh);
        //we're done with our old csurf, so we can get rid of it and replace it with our properly-sized one.
        cairo_surface_destroy(csurf);
        csurf = tmpcsurf;
    }
    oldw = event->width;
    oldh = event->height;
    return TRUE;
}

gboolean on_draw_event(GtkWidget * da, GdkEventExpose * event, gpointer user_data){
    gdk_draw_drawable(da->window,
        da->style->fg_gc[GTK_WIDGET_STATE(da)], csurf,
        // Only copy the area that was exposed.
        event->area.x, event->area.y,
        event->area.x, event->area.y,
        event->area.width, event->area.height);
    return TRUE;
}


static int currently_drawing = 0;
//do_draw will be executed in a separate thread whenever we would like to update
//our animation
void *do_draw(void *ptr){

    currently_drawing = 1;

    int width, height;
    gdk_threads_enter();
    gdk_drawable_get_size(csurf, &width, &height);
    gdk_threads_leave();

    //create a gtk-independant surface to draw on
    cairo_surface_t *cst = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cairo_t *cr = cairo_create(cst);

    //do some time-consuming drawing
    static int i = 0;
    ++i; i = i % 300;   //give a little movement to our animation
    cairo_set_source_rgb (cr, .9, .9, .9);
    cairo_paint(cr);
    int j,k;
    for(k=0; k<100; ++k){   //lets just redraw lots of times to use a lot of proc power
        for(j=0; j < 1000; ++j){
            cairo_set_source_rgb (cr, (double)j/1000.0, (double)j/1000.0, 1.0 - (double)j/1000.0);
            cairo_move_to(cr, i,j/2);
            cairo_line_to(cr, i+100,j/2);
            cairo_stroke(cr);
        }
    }
    cairo_destroy(cr);


    //When dealing with gdkcsurf's, we need to make sure not to
    //access them from outside gtk_main().
    gdk_threads_enter();

    cairo_t *cr_csurf = gdk_cairo_create(csurf);
    cairo_set_source_surface (cr_csurf, cst, 0, 0);
    cairo_paint(cr_csurf);
    cairo_destroy(cr_csurf);

    gdk_threads_leave();

    cairo_surface_destroy(cst);

    currently_drawing = 0;

    return NULL;
}

gboolean timer_exe(GtkWidget * window){

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
    int width, height;
    gdk_drawable_get_size(csurf, &width, &height);
    gtk_widget_queue_draw_area(window, 0, 0, width, height);

    first_execution = FALSE;

    return TRUE;

}


int main (int argc, char *argv[]){


    //we need to initialize all these functions so that gtk knows
    //to be thread-aware
    if (!g_thread_supported ()){ g_thread_init(NULL); }
    gdk_threads_init();
    gdk_threads_enter();

    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(G_OBJECT(window), "draw", G_CALLBACK(on_draw_event), NULL);
    g_signal_connect(G_OBJECT(window), "configure_event", G_CALLBACK(on_window_configure_event), NULL);

    //this must be done before we define our csurf so that it can reference
    //the colour depth and such
    gtk_widget_show_all(window);

    //set up our csurf so it is ready for drawing
    csurf = gdk_csurf_new(window->window,500,500,-1);
    //because we will be painting our csurf manually during expose events
    //we can turn off gtk's automatic painting and double buffering routines.
    gtk_widget_set_app_paintable(window, TRUE);
    gtk_widget_set_double_buffered(window, FALSE);

    (void)g_timeout_add(33, (GSourceFunc)timer_exe, window);


    gtk_main();
    gdk_threads_leave();

    return 0;
}

