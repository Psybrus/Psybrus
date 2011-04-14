/* ///////////////////////////////////////////////////////////////////////////
// Name:        src/gtk/win_gtk.c
// Purpose:     Native GTK+ widget for wxWidgets, based on GtkLayout and
//              GtkFixed. It makes use of the gravity window property and
//              therefore does not work with GTK 1.0.
// Author:      Robert Roebling
// Id:          $Id: win_gtk.c 57904 2009-01-08 12:30:10Z RR $
// Copyright:   (c) 1998 Robert Roebling
// Licence:     wxWidgets licence
/////////////////////////////////////////////////////////////////////////// */

#ifdef VMS
#define XCheckIfEvent XCHECKIFEVENT
#endif

#include "wx/platform.h"
#include "wx/gtk/win_gtk.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _GtkPizzaChild GtkPizzaChild;
typedef struct _GtkPizzaClass GtkPizzaClass;

struct _GtkPizzaClass
{
  GtkContainerClass parent_class;

  void  (*set_scroll_adjustments)   (GtkPizza     *pizza,
                                     GtkAdjustment  *hadjustment,
                                     GtkAdjustment  *vadjustment);
};

struct _GtkPizzaChild
{
    GtkWidget *widget;
    gint x;
    gint y;
};

static void gtk_pizza_class_init    (GtkPizzaClass    *klass);
static void gtk_pizza_init          (GtkPizza         *pizza);

static void gtk_pizza_realize       (GtkWidget        *widget);
static void gtk_pizza_unrealize     (GtkWidget        *widget);

static void gtk_pizza_map           (GtkWidget        *widget);

static void gtk_pizza_size_request  (GtkWidget        *widget,
                                     GtkRequisition   *requisition);
static void gtk_pizza_size_allocate (GtkWidget        *widget,
                                     GtkAllocation    *allocation);
static void gtk_pizza_style_set     (GtkWidget *widget,
                                     GtkStyle  *previous_style);
static void gtk_pizza_add           (GtkContainer     *container,
                                     GtkWidget        *widget);
static void gtk_pizza_remove        (GtkContainer     *container,
                                     GtkWidget        *widget);
static void gtk_pizza_forall        (GtkContainer     *container,
                                     gboolean          include_internals,
                                     GtkCallback       callback,
                                     gpointer          callback_data);

static void     gtk_pizza_allocate_child     (GtkPizza      *pizza,
                                              GtkPizzaChild *child);

static GtkType gtk_pizza_child_type (GtkContainer     *container);

static void  gtk_pizza_scroll_set_adjustments (GtkPizza      *pizza,
                                               GtkAdjustment *hadj,
                                               GtkAdjustment *vadj);

static GtkWidgetClass* pizza_parent_class;

GtkType
gtk_pizza_get_type ()
{
    static GtkType pizza_type = 0;

    if (!pizza_type)
    {
        static const GTypeInfo pizza_info =
        {
            sizeof (GtkPizzaClass),
            NULL,           /* base_init */
            NULL,           /* base_finalize */
            (GClassInitFunc) gtk_pizza_class_init,
            NULL,           /* class_finalize */
            NULL,           /* class_data */
            sizeof (GtkPizza),
            16,             /* n_preallocs */
            (GInstanceInitFunc) gtk_pizza_init,
            NULL
        };
        pizza_type = g_type_register_static (GTK_TYPE_CONTAINER, "GtkPizza", &pizza_info, (GTypeFlags)0);
    }

    return pizza_type;
}

/* Marshaller needed for set_scroll_adjustments signal,
   generated with GLib-2.4.6 glib-genmarshal */
#define g_marshal_value_peek_object(v)   g_value_get_object (v)
static void
g_cclosure_user_marshal_VOID__OBJECT_OBJECT (GClosure     *closure,
                                             GValue       *return_value,
                                             guint         n_param_values,
                                             const GValue *param_values,
                                             gpointer      invocation_hint,
                                             gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__OBJECT_OBJECT) (gpointer     data1,
                                                    gpointer     arg_1,
                                                    gpointer     arg_2,
                                                    gpointer     data2);
  register GMarshalFunc_VOID__OBJECT_OBJECT callback;
  register GCClosure *cc = (GCClosure*) closure;
  register gpointer data1, data2;

  g_return_if_fail (n_param_values == 3);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_VOID__OBJECT_OBJECT) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            g_marshal_value_peek_object (param_values + 1),
            g_marshal_value_peek_object (param_values + 2),
            data2);
}

static void
gtk_pizza_class_init (GtkPizzaClass *klass)
{
    GtkObjectClass *object_class;
    GtkWidgetClass *widget_class;
    GtkContainerClass *container_class;

    object_class = (GtkObjectClass*) klass;
    widget_class = (GtkWidgetClass*) klass;
    container_class = (GtkContainerClass*) klass;
    pizza_parent_class = gtk_type_class (GTK_TYPE_CONTAINER);

    widget_class->map = gtk_pizza_map;
    widget_class->realize = gtk_pizza_realize;
    widget_class->unrealize = gtk_pizza_unrealize;
    widget_class->size_request = gtk_pizza_size_request;
    widget_class->size_allocate = gtk_pizza_size_allocate;
    widget_class->style_set = gtk_pizza_style_set;

    container_class->add = gtk_pizza_add;
    container_class->remove = gtk_pizza_remove;
    container_class->forall = gtk_pizza_forall;

    container_class->child_type = gtk_pizza_child_type;

    klass->set_scroll_adjustments = gtk_pizza_scroll_set_adjustments;

    widget_class->set_scroll_adjustments_signal =
        g_signal_new(
            "set_scroll_adjustments",
            G_TYPE_FROM_CLASS(object_class),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET(GtkPizzaClass, set_scroll_adjustments),
            NULL,
            NULL,
            g_cclosure_user_marshal_VOID__OBJECT_OBJECT,
            G_TYPE_NONE,
            2,
            GTK_TYPE_ADJUSTMENT,
            GTK_TYPE_ADJUSTMENT);
}

static GtkType
gtk_pizza_child_type (GtkContainer     *container)
{
    return GTK_TYPE_WIDGET;
}

static void
gtk_pizza_init (GtkPizza *pizza)
{
    GTK_WIDGET_UNSET_FLAGS (pizza, GTK_NO_WINDOW);

    pizza->children = NULL;

    pizza->bin_window = NULL;

    pizza->m_xoffset = 0;
    pizza->m_yoffset = 0;
}

GtkWidget*
gtk_pizza_new ()
{
    GtkPizza *pizza;

    pizza = g_object_new (gtk_pizza_get_type (), NULL);

    return GTK_WIDGET (pizza);
}

gint       gtk_pizza_get_xoffset     (GtkPizza          *pizza)
{
    g_return_val_if_fail ( (pizza != NULL), -1 );
    g_return_val_if_fail ( (GTK_IS_PIZZA (pizza)), -1 );

    return pizza->m_xoffset;
}

gint       gtk_pizza_get_yoffset     (GtkPizza          *pizza)
{
    g_return_val_if_fail ( (pizza != NULL), -1 );
    g_return_val_if_fail ( (GTK_IS_PIZZA (pizza)), -1 );

    return pizza->m_yoffset;
}

void       gtk_pizza_set_xoffset     (GtkPizza          *pizza, gint xoffset)
{
    g_return_if_fail (pizza != NULL);
    g_return_if_fail (GTK_IS_PIZZA (pizza));

    pizza->m_xoffset = xoffset;
    /* do something */
}

void       gtk_pizza_set_yoffset     (GtkPizza          *pizza, gint yoffset)
{
    g_return_if_fail (pizza != NULL);
    g_return_if_fail (GTK_IS_PIZZA (pizza));

    pizza->m_xoffset = yoffset;
    /* do something */
}

gint       gtk_pizza_get_rtl_offset  (GtkPizza          *pizza)
{
    gint border;

    g_return_val_if_fail ( (pizza != NULL), 0 );
    g_return_val_if_fail ( (GTK_IS_PIZZA (pizza)), 0 );
    
    if (!pizza->bin_window) return 0;
    
    border = pizza->container.border_width;
        
    return GTK_WIDGET(pizza)->allocation.width - border*2;
}


static void
gtk_pizza_scroll_set_adjustments (GtkPizza     *pizza,
                                    GtkAdjustment  *hadj,
                                    GtkAdjustment  *vadj)
{
   /* We handle scrolling in the wxScrolledWindow, not here. */
}

void
gtk_pizza_put (GtkPizza   *pizza,
               GtkWidget  *widget,
               gint        x,
               gint        y,
               gint        width,
               gint        height)
{
    GtkPizzaChild *child_info;

    g_return_if_fail (pizza != NULL);
    g_return_if_fail (GTK_IS_PIZZA (pizza));
    g_return_if_fail (widget != NULL);

    child_info = g_new (GtkPizzaChild, 1);

    child_info->widget = widget;
    child_info->x = x;
    child_info->y = y;

    pizza->children = g_list_append (pizza->children, child_info);

    if (GTK_WIDGET_REALIZED (pizza))
        gtk_widget_set_parent_window (widget, pizza->bin_window);

    gtk_widget_set_parent (widget, GTK_WIDGET (pizza));

    gtk_widget_set_size_request( widget, width, height );
    if (GTK_WIDGET_REALIZED (pizza))
        gtk_pizza_allocate_child (pizza, child_info);
}

void
gtk_pizza_set_size (GtkPizza   *pizza,
                    GtkWidget  *widget,
                    gint        x,
                    gint        y,
                    gint        width,
                    gint        height)
{
    GtkPizzaChild *child;
    GList *children;

    g_return_if_fail (pizza != NULL);
    g_return_if_fail (GTK_IS_PIZZA (pizza));
    g_return_if_fail (widget != NULL);

#ifndef WX_WARN_ILLEGAL_SETSIZE
    /* this really shouldn't happen -- but it does, a lot, right now and we
       can't pass negative values to gtk_widget_set_size_request() without getting
       a warning printed out, so filter them out here */
    if ( width < 0 )
        width = 0;
    if ( height < 0 )
        height = 0;
#endif

    children = pizza->children;
    while (children)
    {
        child = children->data;
        children = children->next;

        if (child->widget == widget)
        {
            if (child->x != x || child->y != y)
            {
                child->x = x;
                child->y = y;
                gtk_widget_queue_resize(widget);
            }

            gtk_widget_set_size_request (widget, width, height);

            return;
        }
    }
}

static void
gtk_pizza_map (GtkWidget *widget)
{
    GtkPizza *pizza;
    GtkPizzaChild *child;
    GList *children;

    g_return_if_fail (widget != NULL);
    g_return_if_fail (GTK_IS_PIZZA (widget));

    GTK_WIDGET_SET_FLAGS (widget, GTK_MAPPED);
    pizza = GTK_PIZZA (widget);

    children = pizza->children;
    while (children)
    {
        child = children->data;
        children = children->next;

        if ( GTK_WIDGET_VISIBLE (child->widget) &&
            !GTK_WIDGET_MAPPED (child->widget) )
        {
            gtk_widget_map (child->widget);
        }
    }

    gdk_window_show (widget->window);
    gdk_window_show (pizza->bin_window);
}

static void
gtk_pizza_realize (GtkWidget *widget)
{
    GtkPizza *pizza;
    GdkWindowAttr attributes;
    gint attributes_mask;
    GtkPizzaChild *child;
    GList *children;
    int border;

    g_return_if_fail (widget != NULL);
    g_return_if_fail (GTK_IS_PIZZA (widget));

    pizza = GTK_PIZZA (widget);
    GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);

    attributes.window_type = GDK_WINDOW_CHILD;

    attributes.x = widget->allocation.x;
    attributes.y = widget->allocation.y;
    attributes.width = widget->allocation.width;
    attributes.height = widget->allocation.height;

    border = pizza->container.border_width;
    attributes.x += border;
    attributes.y += border;
    attributes.width -= 2 * border;
    attributes.height -= 2 * border;

    /* minimal size */
    if (attributes.width < 2) attributes.width = 2;
    if (attributes.height < 2) attributes.height = 2;

    attributes.wclass = GDK_INPUT_OUTPUT;
    attributes.visual = gtk_widget_get_visual (widget);
    attributes.colormap = gtk_widget_get_colormap (widget);
    attributes.event_mask = GDK_VISIBILITY_NOTIFY_MASK;
    attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

    widget->window = gdk_window_new(gtk_widget_get_parent_window (widget),
                                     &attributes, attributes_mask);
    gdk_window_set_user_data (widget->window, widget);

    attributes.x = 0;
    attributes.y = 0;

    attributes.event_mask = gtk_widget_get_events (widget);
    attributes.event_mask |= GDK_EXPOSURE_MASK              |
                             GDK_SCROLL_MASK                |
                             GDK_POINTER_MOTION_MASK        |
                             GDK_POINTER_MOTION_HINT_MASK   |
                             GDK_BUTTON_MOTION_MASK         |
                             GDK_BUTTON1_MOTION_MASK        |
                             GDK_BUTTON2_MOTION_MASK        |
                             GDK_BUTTON3_MOTION_MASK        |
                             GDK_BUTTON_PRESS_MASK          |
                             GDK_BUTTON_RELEASE_MASK        |
                             GDK_KEY_PRESS_MASK             |
                             GDK_KEY_RELEASE_MASK           |
                             GDK_ENTER_NOTIFY_MASK          |
                             GDK_LEAVE_NOTIFY_MASK          |
                             GDK_FOCUS_CHANGE_MASK;

    pizza->bin_window = gdk_window_new(widget->window,
                                          &attributes, attributes_mask);
    gdk_window_set_user_data (pizza->bin_window, widget);

    widget->style = gtk_style_attach (widget->style, widget->window);
    gtk_style_set_background (widget->style, widget->window, GTK_STATE_NORMAL);
    gtk_style_set_background (widget->style, pizza->bin_window, GTK_STATE_NORMAL );

/*
    gdk_window_set_back_pixmap( widget->window, NULL, FALSE );
    gdk_window_set_back_pixmap( pizza->bin_window, NULL, FALSE );
*/

    /* cannot be done before realisation */
    children = pizza->children;
    while (children)
    {
        child = children->data;
        children = children->next;

        gtk_widget_set_parent_window (child->widget, pizza->bin_window);
    }
}

static void
gtk_pizza_unrealize (GtkWidget *widget)
{
    GtkPizza *pizza;

    g_return_if_fail (widget != NULL);
    g_return_if_fail (GTK_IS_PIZZA (widget));

    pizza = GTK_PIZZA (widget);

    gdk_window_set_user_data (pizza->bin_window, NULL);
    gdk_window_destroy (pizza->bin_window);
    pizza->bin_window = NULL;

    if (pizza_parent_class->unrealize)
        pizza_parent_class->unrealize(widget);
}

static void
gtk_pizza_size_request (GtkWidget      *widget,
                        GtkRequisition *requisition)
{
    GtkPizza *pizza;
    GtkPizzaChild *child;
    GList *children;
    GtkRequisition child_requisition;

    g_return_if_fail (widget != NULL);
    g_return_if_fail (GTK_IS_PIZZA (widget));
    g_return_if_fail (requisition != NULL);

    pizza = GTK_PIZZA (widget);

    children = pizza->children;
    while (children)
    {
        child = children->data;
        children = children->next;

        if (GTK_WIDGET_VISIBLE (child->widget))
        {
            gtk_widget_size_request (child->widget, &child_requisition);
        }
    }

    /* request very little, I'm not sure if requesting nothing
       will always have positive effects on stability... */
    requisition->width = 2;
    requisition->height = 2;
}

static void
gtk_pizza_size_allocate (GtkWidget     *widget,
                         GtkAllocation *allocation)
{
    GtkPizza *pizza;
    gint border;
    gint x,y,w,h;
    GtkPizzaChild *child;
    GList *children;
    gboolean only_resize;

    g_return_if_fail (widget != NULL);
    g_return_if_fail (GTK_IS_PIZZA(widget));
    g_return_if_fail (allocation != NULL);

    pizza = GTK_PIZZA (widget);

    only_resize = ((widget->allocation.x == allocation->x) &&
                   (widget->allocation.y == allocation->y));
    widget->allocation = *allocation;

    border = pizza->container.border_width;
    
    x = allocation->x + border;
    y = allocation->y + border;
    w = allocation->width - border*2;
    h = allocation->height - border*2;
    if (w < 0)
        w = 0;
    if (h < 0)
        h = 0;

    if (GTK_WIDGET_REALIZED (widget))
    {
        if (only_resize)
            gdk_window_resize( widget->window, w, h );
        else
            gdk_window_move_resize( widget->window, x, y, w, h );

        gdk_window_resize( pizza->bin_window, w, h );
    }

    children = pizza->children;
    while (children)
    {
        child = children->data;
        children = children->next;

        gtk_pizza_allocate_child (pizza, child);
    }
}

static void
gtk_pizza_style_set(GtkWidget *widget, GtkStyle  *previous_style)
{
    if (GTK_WIDGET_REALIZED(widget))
    {
        gtk_style_set_background(widget->style, widget->window, GTK_STATE_NORMAL);
        gtk_style_set_background(widget->style, GTK_PIZZA(widget)->bin_window, GTK_STATE_NORMAL );
    }

    pizza_parent_class->style_set(widget, previous_style);
}

static void
gtk_pizza_add (GtkContainer *container,
               GtkWidget    *widget)
{
    g_return_if_fail (container != NULL);
    g_return_if_fail (GTK_IS_PIZZA (container));
    g_return_if_fail (widget != NULL);

    gtk_pizza_put (GTK_PIZZA (container), widget, 0, 0, 20, 20 );
}

static void
gtk_pizza_remove (GtkContainer *container,
                  GtkWidget    *widget)
{
    GtkPizza *pizza;
    GtkPizzaChild *child;
    GList *children;

    g_return_if_fail (container != NULL);
    g_return_if_fail (GTK_IS_PIZZA (container));
    g_return_if_fail (widget != NULL);

    pizza = GTK_PIZZA (container);

    children = pizza->children;
    while (children)
    {
        child = children->data;

        if (child->widget == widget)
        {
            gtk_widget_unparent (widget);

            /* security checks */
            g_return_if_fail (GTK_IS_WIDGET (widget));

            pizza->children = g_list_remove_link (pizza->children, children);
            g_list_free (children);
            g_free (child);

            /* security checks */
            g_return_if_fail (GTK_IS_WIDGET (widget));

            break;
        }

        children = children->next;
    }
}

static void
gtk_pizza_forall (GtkContainer *container,
                  gboolean      include_internals,
                  GtkCallback   callback,
                  gpointer      callback_data)
{
    GtkPizza *pizza;
    GtkPizzaChild *child;
    GList *children;

    g_return_if_fail (container != NULL);
    g_return_if_fail (GTK_IS_PIZZA (container));
    g_return_if_fail (callback != (GtkCallback)NULL);

    pizza = GTK_PIZZA (container);

    children = pizza->children;
    while (children)
    {
        child = children->data;
        children = children->next;

        (* callback) (child->widget, callback_data);
    }
}

static void
gtk_pizza_allocate_child (GtkPizza      *pizza,
                          GtkPizzaChild *child)
{
    GtkAllocation allocation;
    GtkRequisition requisition;

    allocation.x = child->x - pizza->m_xoffset;
    allocation.y = child->y - pizza->m_yoffset;
    gtk_widget_get_child_requisition (child->widget, &requisition);
    allocation.width = requisition.width;
    allocation.height = requisition.height;

    if (gtk_widget_get_direction( GTK_WIDGET(pizza) ) == GTK_TEXT_DIR_RTL)
    {
        /* reverse horizontal placement */
        gint offset,border; 
        
        offset = GTK_WIDGET(pizza)->allocation.width;
        border = pizza->container.border_width;
        offset -= border*2;
            
        allocation.x = offset - child->x - allocation.width + pizza->m_xoffset; 
    }
    
    gtk_widget_size_allocate (child->widget, &allocation);
}

typedef struct {
    GdkWindow* window;
    int dx, dy;
} AdjustData;

// Adjust allocations for all widgets using the GdkWindow which was just scrolled
static void scroll_adjust(GtkWidget* widget, void* data)
{
    const AdjustData* p = data;
    widget->allocation.x += p->dx;
    widget->allocation.y += p->dy;
    
    if (widget->window == p->window)
    {
        // GtkFrame requires a queue_resize, otherwise parts of
        // the frame newly exposed by the scroll are not drawn.
        // To be safe, do it for all widgets.
        gtk_widget_queue_resize(widget);
        if (GTK_IS_CONTAINER(widget))
            gtk_container_forall(GTK_CONTAINER(widget), scroll_adjust, data);
    }
}

void
gtk_pizza_scroll (GtkPizza *pizza, gint dx, gint dy)
{
    pizza->m_xoffset += dx;
    pizza->m_yoffset += dy;

    if (pizza->bin_window)
    {
        AdjustData data = { pizza->bin_window, -dx, -dy };
        gdk_window_scroll( pizza->bin_window, -dx, -dy );
        // Adjust child allocations. Doing a queue_resize on the children is not
        // enough, sometimes they redraw in the wrong place during fast scrolling.
        gtk_container_forall(GTK_CONTAINER(pizza), scroll_adjust, &data);
    }
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
