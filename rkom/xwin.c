
#include <sys/stat.h>

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>

#include <X11/Shell.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Text.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Form.h>
#include <X11/Intrinsic.h>

#include "xwin.h"
#include "parse.h"
#include "rkom.h"

int nflag;

typedef XFontStruct *XFont;
XtAppContext  lsx_app_con;

typedef void (*ButtonCB)(Widget w, void *data);
XFont GetWidgetFont(Widget w);
Widget MakeLabel(char *txt);
int   TextWidth(XFont f, char *txt);
Widget  MakeTextWidget(char *txt, int is_file, int editable, int w, int h);
Widget MakeButton(char *label, ButtonCB function, void *data);
void   SetCurrentWindow(Widget w);
void   CloseWindow(void);
void  SetWidgetPos(Widget w, int where1, Widget from1,int where2,Widget from2);
void AttachEdge(Widget w, int edge, int attach_to);
void   ShowDisplay(void);
void  SetLabel(Widget w, char *txt);
int    OpenDisplay(int argc, char **argv);
Widget MakeWindow(char *window_name, char *display_name, int exclusive);
void   SetDrawArea(Widget w);
void  FreeFont(XFont f);
typedef void (*RedisplayCB)(Widget w, int new_width, int new_height, void *d);
typedef void (*MouseButtonCB)(Widget w, int button, int x, int y, void *dat);
typedef void (*KeyCB)(Widget w, char *input, int up_or_down, void *data);
typedef void (*MotionCB)(Widget w, int x, int y, void *data);
typedef void (*StringCB)(Widget w, char *string, void *data);
Widget MakeStringEntry(char *txt, int size, StringCB func, void *data);

void SetOkay(Widget w, XEvent* x, String*s, Cardinal*c);
void MainLoop(void);


typedef struct wininfo
{
  Widget window, text_widget, label_widget;
  int *num_windows;
  char *cur_path;
}WinInfo;

#define PLACE_UNDER   0x02 /* place me under the specified gadget */
#define PLACE_RIGHT   0x01 /* place me to the right of specified gadget */
#define RIGHT_EDGE     0x01
#define ATTACH_LEFT    0x00   /* attach given edge to the left side of form */
#define ATTACH_TOP     0x02   /* attach given edge to the top of the form */
#define ATTACH_RIGHT   0x01   /* attach given edge to the right side of form */
#define ATTACH_BOTTOM  0x03   /* attach given edge to the bottom of the form */
#define BOTTOM_EDGE    0x03
#define LEFT_EDGE      0x00   /* These #define's specify which edge we want */
#define TOP_EDGE       0x02
#define ATTACH_BOTTOM  0x03   /* attach given edge to the bottom of the form */
#define NONEXCLUSIVE_WINDOW  0
#define NO_CARE       0x00 /* don't care where the gadget is placed */
#define ORIGINAL_WINDOW  NULL

static void
quit(Widget foo, void *arg)
{

  SetCurrentWindow(XtParent(XtParent(foo)));
  CloseWindow();
  exit(0);
}

static Widget main_widget, input_widget, return_widget;

static void
gotcmd(Widget foo, void *arg)
{
	int n;
	Arg wargs[2];
	char *text;

	n = 0;
	XtSetArg(wargs[n], XtNstring, &text); n++;
	XtGetValues(input_widget, wargs, n);

	xputstr("\n");

	exec_cmd(text);

	rprintf("\n%s - ", prompt);
}

#define MAXLABEL  80

static void
make_text_viewer(char *fname, WinInfo *arg)
{
  Widget w[10];
  static char dummy_label[MAXLABEL];
  int i, width;
  XFontStruct *xf;

  for(i=0; i < MAXLABEL-1; i++)
    dummy_label[i] = ' ';
  dummy_label[i] = '\0';

  w[0] = MakeLabel(dummy_label);

  xf = GetWidgetFont(w[0]);
  if (xf != NULL)
    width = TextWidth(xf, dummy_label);
  else
    width = 600;

  w[1] = MakeTextWidget(fname, FALSE, FALSE, width, 500);
  w[2] = MakeButton("Sluta", quit, arg);
  return_widget = w[3] = MakeButton("Vagnretur", gotcmd, arg);
  w[4] = MakeButton("New Window", quit, arg);
  
  input_widget = MakeStringEntry(NULL, width, NULL, NULL);
  SetWidgetPos(input_widget, PLACE_UNDER, w[1], NO_CARE, NULL);
  
  SetWidgetPos(w[1], PLACE_UNDER, w[0], NO_CARE, NULL);
  SetWidgetPos(w[2], PLACE_UNDER, input_widget, NO_CARE, NULL);
  SetWidgetPos(w[3], PLACE_UNDER, input_widget, PLACE_RIGHT, w[2]);
  SetWidgetPos(w[4], PLACE_UNDER, input_widget, PLACE_RIGHT, w[3]);

  AttachEdge(input_widget, LEFT_EDGE,   ATTACH_LEFT);
  AttachEdge(input_widget, RIGHT_EDGE,  ATTACH_RIGHT);
  AttachEdge(input_widget, TOP_EDGE,    ATTACH_TOP);
  AttachEdge(input_widget, BOTTOM_EDGE, ATTACH_BOTTOM);
  
  AttachEdge(w[0], RIGHT_EDGE,  ATTACH_LEFT);
  AttachEdge(w[0], BOTTOM_EDGE, ATTACH_TOP);
  
  AttachEdge(w[1], LEFT_EDGE,   ATTACH_LEFT);
  AttachEdge(w[1], RIGHT_EDGE,  ATTACH_RIGHT);
  AttachEdge(w[1], TOP_EDGE,    ATTACH_TOP);
  AttachEdge(w[1], BOTTOM_EDGE, ATTACH_BOTTOM);
  
  AttachEdge(w[2], LEFT_EDGE,   ATTACH_LEFT);
  AttachEdge(w[2], RIGHT_EDGE,  ATTACH_LEFT);
  AttachEdge(w[2], TOP_EDGE,    ATTACH_BOTTOM);
  AttachEdge(w[2], BOTTOM_EDGE, ATTACH_BOTTOM);

  AttachEdge(w[3], LEFT_EDGE,   ATTACH_LEFT);
  AttachEdge(w[3], RIGHT_EDGE,  ATTACH_LEFT);
  AttachEdge(w[3], TOP_EDGE,    ATTACH_BOTTOM);
  AttachEdge(w[3], BOTTOM_EDGE, ATTACH_BOTTOM);

  AttachEdge(w[4], LEFT_EDGE,   ATTACH_LEFT);
  AttachEdge(w[4], RIGHT_EDGE,  ATTACH_LEFT);
  AttachEdge(w[4], TOP_EDGE,    ATTACH_BOTTOM);
  AttachEdge(w[4], BOTTOM_EDGE, ATTACH_BOTTOM);
  
  arg->text_widget  = w[1];
  arg->label_widget = w[0];

  ShowDisplay();

  SetLabel(w[0], fname);   /* set the real filename */
}

static char *dummy_argv[] = { "Rkom", NULL };
static int   dummy_argc  = 1;

void
xwininit()
{
	static WinInfo wi;

	if (OpenDisplay(dummy_argc, dummy_argv) == 0)
		errx(1, "Can't open display");

	MakeWindow("Rkom", NULL, NONEXCLUSIVE_WINDOW);
	make_text_viewer("nisse", &wi);

	main_widget = wi.text_widget;
}

/*
 * Structures and variables private to libsx.
 */

typedef struct WindowState
{
  int      screen;
  int      window_shown;
  Window   window;
  Display *display;
  Widget   toplevel, toplevel_form, form_widget, last_draw_widget;
  int      has_standard_colors;
  int      named_colors[256];
  int      color_index;
  Colormap     cmap;
  Pixmap       check_mark;
  XFontStruct *font;

  struct WindowState *next;
}WindowState;

/*
 * Private static variables.
 */
static WindowState  *lsx_windows  = NULL,
                     empty_window = { 0, 0, 0, NULL, NULL, NULL, NULL, 0,},
                    *orig_window  = NULL,
                    *new_curwin   = NULL;
WindowState  *lsx_curwin = &empty_window;
static Display *base_display=NULL;



typedef struct DrawInfo
{
  RedisplayCB   redisplay;
  MouseButtonCB button_down;
  MouseButtonCB button_up;
  KeyCB         keypress;
  MotionCB      motion;

  GC            drawgc;       /* Graphic Context for drawing in this widget */

  int           foreground;   /* need to save these so we know what they are */
  int           background; 
  unsigned long mask;
  XFontStruct  *font;

  void        *user_data;

  Widget widget;
  struct DrawInfo *next;
}DrawInfo;


DrawInfo *libsx_find_draw_info(Widget w);  /* private internal function */


void SetCurrentWindow(Widget w)
{
  WindowState *win;

  if (w == ORIGINAL_WINDOW)
   {
     if (orig_window)
       lsx_curwin = orig_window;
     else if (lsx_windows)            /* hmm, don't have orig_window */
       lsx_curwin = lsx_windows;
     else
       lsx_curwin = &empty_window;    /* hmm, don't have anything left */

     SetDrawArea(lsx_curwin->last_draw_widget);
     return;
   }

  for(win=lsx_windows; win; win=win->next)
    if (win->toplevel == w && win->display == XtDisplay(w))
      break;

  if (win == NULL)
    return;

  lsx_curwin = win;    
  SetDrawArea(lsx_curwin->last_draw_widget);
}

void CloseWindow(void)
{
  WindowState *tmp;
  int is_transient;
  
  if (lsx_curwin->toplevel == NULL)
    return;

  is_transient = XtIsTransientShell(lsx_curwin->toplevel);
  
  XtDestroyWidget(lsx_curwin->toplevel);

  if (lsx_curwin->display != base_display)
   {
     FreeFont(lsx_curwin->font);
     XtCloseDisplay(lsx_curwin->display);
   }

  
  /*
   * if the window to delete is not at the head of the list, find
   * it in the list of available windows.
   * else, just assign tmp to the head of the list.
   */
  if (lsx_curwin != lsx_windows)
    for(tmp=lsx_windows; tmp && tmp->next != lsx_curwin; tmp=tmp->next)
      ;
  else
    tmp = lsx_windows;
  
  if (tmp == NULL)  /* bogus window deletion... */
   {
     return;
   }
  else if (lsx_curwin == lsx_windows)   /* delete head of list */
   {
     lsx_windows = lsx_curwin->next;
     tmp = lsx_windows;
   }
  else
   {
     tmp->next = lsx_curwin->next;
   }

  if (lsx_curwin == orig_window)
    orig_window = NULL;
  
  if (is_transient)
   {
     lsx_curwin->window_shown = FALSE;

     /*
      * Store tmp in a global so MainLoop() can change lsx_curwin
      * to what it should be after it frees everything.
      */
     new_curwin = tmp; 
   }
  else    /* it's a normal window, so get rid of it completely */
   {
     free(lsx_curwin);
     lsx_curwin = tmp;
   }
  
}

/*
 * Text Label Creation.
 */

Widget MakeLabel(char *txt)
{
  int    n = 0;
  int    bg_color = -1;
  Arg    wargs[5];              /* Used to set widget resources */
  Widget label;

  if (lsx_curwin->toplevel == NULL && OpenDisplay(0, NULL) == 0)
    return NULL;

  n = 0;
  if (txt)
   {
     XtSetArg(wargs[n], XtNlabel, txt);               n++;  
   }

  label = XtCreateManagedWidget("label", labelWidgetClass,
                                lsx_curwin->form_widget,wargs,n);
  if (label == NULL)
    return NULL;

  /* this little contortion here is to make sure there is no
   * border around the label (else it looks exactly like a command
   * button, and that's confusing)
   */

  n = 0;
  XtSetArg(wargs[n], XtNbackground, &bg_color);       n++;  
  XtGetValues(label, wargs, n);

  n = 0;
  XtSetArg(wargs[n], XtNborder, bg_color);            n++;  
  XtSetValues(label, wargs, n);

  return label;
}                    /* end of MakeLabel() */

XFont  GetWidgetFont(Widget w)
{
  int    n = 0;
  Arg    wargs[1];              /* Used to set widget resources */
  XFont  f=NULL;
  DrawInfo *di;

  if (lsx_curwin->toplevel == NULL || w == NULL)
    return NULL;


  if ((di=libsx_find_draw_info(w)) != NULL)
   {
     return di->font;
   }

  n = 0;
  XtSetArg(wargs[n], XtNfont, &f);                   n++;  
  XtGetValues(w, wargs, n);

  return f;
}

int TextWidth(XFont f, char *txt)
{
  if (f && txt)
    return XTextWidth(f, txt, strlen(txt));
  else
    return -1;
}

static char *
slurp_file(char *fname)
{
  struct stat st;
  char *buff;
  int   fd, count;

  if (stat(fname, &st) < 0)
    return NULL;

  if (S_ISDIR(st.st_mode) == TRUE)   /* don't want to edit directories */
    return NULL;
    
  buff = (char *)malloc(sizeof(char)*(st.st_size+1));
  if (buff == NULL)
    return NULL;

  fd = open(fname, O_RDONLY);
  if (fd < 0)
   {
     free(buff);
     return NULL;
   }

  count = read(fd, buff, st.st_size);
  buff[count] = '\0';        /* null terminate */
  close(fd);

  return (buff);
}


Widget
MakeTextWidget(char *txt, int is_file, int editable, int w, int h)
{
  int n;
  Arg wargs[10];
  Widget text;
  char *real_txt;
  static int already_done = FALSE;
  static XtTranslations trans;

  if (lsx_curwin->toplevel == NULL && OpenDisplay(0, NULL) == 0)
    return NULL;

  if (already_done == FALSE)
   {
     already_done = TRUE;
     trans = XtParseTranslationTable("#override\n\
                                      <Key>Prior: previous-page()\n\
                                      <Key>Next:  next-page()\n\
                                      <Key>Home:  beginning-of-file()\n\
                                      <Key>End:   end-of-file()\n\
                                      Ctrl<Key>Up:    beginning-of-file()\n\
                                      Ctrl<Key>Down:  end-of-file()\n\
                                      Shift<Key>Up:   previous-page()\n\
                                      Shift<Key>Down: next-page()\n");
   }

  n=0;
  XtSetArg(wargs[n], XtNwidth,            w);                        n++;
  XtSetArg(wargs[n], XtNheight,           h);                        n++;
  XtSetArg(wargs[n], XtNscrollHorizontal, XawtextScrollWhenNeeded);  n++;
  XtSetArg(wargs[n], XtNscrollVertical,   XawtextScrollWhenNeeded);  n++;
  XtSetArg(wargs[n], XtNautoFill,         TRUE);                     n++;
  XtSetArg(wargs[n], XtNtranslations, trans);                        n++;

  if (is_file && txt)
   {
     real_txt = slurp_file(txt);
   }
  else
    real_txt = txt;
  
  if (real_txt)
    { XtSetArg(wargs[n], XtNstring,       real_txt);                 n++; }
  if (editable)
    { XtSetArg(wargs[n], XtNeditType,     XawtextEdit);              n++; }

  text = XtCreateManagedWidget("text", asciiTextWidgetClass,
                               lsx_curwin->form_widget,wargs,n);


  if (real_txt != txt && real_txt != NULL) 
    free(real_txt);                         /* we're done with the buffer */

  return text;
}

/*
 * Command Button Creation routine.
 */

Widget MakeButton(char *txt, ButtonCB func, void *data)
{
  int    n = 0;
  Arg    wargs[5];              /* Used to set widget resources */
  Widget button;

  if (lsx_curwin->toplevel == NULL && OpenDisplay(0, NULL) == 0)
    return NULL;

  n = 0;
  if (txt)
   {
     XtSetArg(wargs[n], XtNlabel, txt);                   n++;
   }


  button = XtCreateManagedWidget("button", commandWidgetClass,
                                 lsx_curwin->form_widget,wargs,n);
  if (button == NULL)
    return NULL;

  if (func)
    XtAddCallback(button, XtNcallback, (XtCallbackProc)func, data);

  return button;
}    /* end of MakeButton() */

void SetWidgetPos(Widget w, int where1, Widget from1, int where2, Widget from2)
{
  int  n = 0;
  Arg  wargs[5];                /* Used to set widget resources */

  if (lsx_curwin->toplevel == NULL || w == NULL)
    return;

  /*
   * Don't want to do this for menu item widgets
   */
  if (XtName(w) && strcmp(XtName(w), "menu_item") == 0)
    return;

  /*
   * This if statement handles the case that the widget we were passed
   * was a List widget.  The reason we use the parent of the widget 
   * instead of the widget we were given is because we really want to
   * set the position of the viewport widget that is the parent of the
   * List widget (because when we create a List widget, its parent is
   * a Viewport widget, not the lsx_curwin->form_widget like everyone else.
   *
   * The extra check for the name of the widget not being "form" is to
   * allow proper setting of multiple form widgets.  In the case that
   * we are setting multiple form widgets, the parent of the widget
   * we are setting (and those it is relative to) will not be
   * lsx_curwin->form_widget.  When this is the case, we just want to
   * set the widget itself, not the parent.  Basically this is an
   * exception to the previous paragraph (and it should be the only
   * one, I think).
   *
   * Kind of hackish.  Oh well...
   */
  if (XtParent(w) != lsx_curwin->form_widget && strcmp(XtName(w), "form") != 0)
    w = XtParent(w);

  /*
   * We also change the from1 and from2 fields just as above just so
   * that positioning relative to a list widget works correctly.
   *
   * If we just used the list widget, we'd use its full size for positioning
   * even though the size of the viewport widget is all that's really visible.
   */
  if (from1 && XtParent(from1) != lsx_curwin->form_widget
      && strcmp(XtName(from1), "form") != 0)
    from1 = XtParent(from1);

  if (from2 && XtParent(from2) != lsx_curwin->form_widget
      && strcmp(XtName(from2), "form") != 0)
    from2 = XtParent(from2);



  /*
   * Here we do the first half of the positioning.
   */
  if (where1 == PLACE_RIGHT && from1)
   { 
     XtSetArg(wargs[n], XtNfromHoriz, from1);              n++; 
   }
  else if (where1 == PLACE_UNDER && from1)
   { 
     XtSetArg(wargs[n], XtNfromVert,  from1);              n++; 
   }


  /*
   * Now do the second half of the positioning
   */
  if (where2 == PLACE_RIGHT && from2)
   { 
     XtSetArg(wargs[n], XtNfromHoriz, from2);              n++; 
   }
  else if (where2 == PLACE_UNDER && from2)
   { 
     XtSetArg(wargs[n], XtNfromVert,  from2);              n++; 
   }


  if (n)                      /* if any values were actually set */
    XtSetValues(w, wargs, n);
}

void AttachEdge(Widget w, int edge, int attach_to)
{
  char *edge_name;
  static char *edges[]    = { XtNleft,     XtNright,     XtNtop,
                              XtNbottom };
  static int   attached[] = { XtChainLeft, XtChainRight, XtChainTop,
                              XtChainBottom };
  int   a;
  Arg wargs[5];
  int n=0;


  if (w == NULL || edge < 0 || edge > BOTTOM_EDGE
      || attach_to < 0 || attach_to > ATTACH_BOTTOM)
    return;
  
  edge_name = edges[edge];
  a         = attached[attach_to];
  
  n=0;
  XtSetArg(wargs[n], edge_name, a);     n++;

  XtSetValues(w, wargs, n);
}

#define min(x, y)                     (((x) < (y)) ? (x) : (y))
#define max(x, y)                     (((x) > (y)) ? (x) : (y))


static Bool is_expose_event(Display *d, XEvent *xev, char *blorg)
{
  if (xev->type == Expose)
    return TRUE;
  else
    return FALSE;
}

static void
PositionPopup(Widget shell_widget)
{
  int n;
  Arg wargs[10];
  Position popup_x, popup_y, top_x, top_y;
  Dimension popup_width, popup_height, top_width, top_height, border_width;

  n = 0;
  XtSetArg(wargs[n], XtNx, &top_x); n++;
  XtSetArg(wargs[n], XtNy, &top_y); n++;
  XtSetArg(wargs[n], XtNwidth, &top_width); n++;
  XtSetArg(wargs[n], XtNheight, &top_height); n++;
  XtGetValues(XtParent(shell_widget), wargs, n);

  n = 0;
  XtSetArg(wargs[n], XtNwidth, &popup_width); n++;
  XtSetArg(wargs[n], XtNheight, &popup_height); n++;
  XtSetArg(wargs[n], XtNborderWidth, &border_width); n++;
  XtGetValues(shell_widget, wargs, n);

  popup_x = max(0, 
                min(top_x + ((Position)top_width - (Position)popup_width) / 2, 
                    (Position)DisplayWidth(XtDisplay(shell_widget), 
                               DefaultScreen(XtDisplay(shell_widget))) -
                    (Position)popup_width - 2 * (Position)border_width));
  popup_y = max(0, 
                min(top_y+((Position)top_height - (Position)popup_height) / 2,
                    (Position)DisplayHeight(XtDisplay(shell_widget), 
                               DefaultScreen(XtDisplay(shell_widget))) -
                    (Position)popup_height - 2 * (Position)border_width));
  n = 0;
  XtSetArg(wargs[n], XtNx, popup_x); n++;
  XtSetArg(wargs[n], XtNy, popup_y); n++;
  XtSetValues(shell_widget, wargs, n);
}


void ShowDisplay(void)
{
  XEvent xev;

  if (lsx_curwin->toplevel == NULL || lsx_curwin->window_shown == TRUE)
    return;

  XtRealizeWidget(lsx_curwin->toplevel);

  if (XtIsTransientShell(lsx_curwin->toplevel))  /* do popups differently */
   {
     PositionPopup(lsx_curwin->toplevel);
     XtPopup(lsx_curwin->toplevel, XtGrabExclusive);
     
     lsx_curwin->window  = (Window   )XtWindow(lsx_curwin->toplevel);
     lsx_curwin->window_shown = TRUE;

     return;
   }

  /*
   * wait until the window is _really_ on screen
   */
  while(!XtIsRealized(lsx_curwin->toplevel))
    ;

  /*
   * Now make sure it is really on the screen.
   */
  XPeekIfEvent(XtDisplay(lsx_curwin->toplevel), &xev, is_expose_event, NULL);


  SetDrawArea(lsx_curwin->last_draw_widget);

  lsx_curwin->window  = (Window   )XtWindow(lsx_curwin->toplevel);
  lsx_curwin->window_shown = TRUE;
}   /* end of ShowDisplay() */

void SetLabel(Widget w, char *txt)
{
  int    n = 0;
  Arg    wargs[1];              /* Used to set widget resources */

  if (lsx_curwin->toplevel == NULL || w == NULL)
    return;

  n = 0;
  XtSetArg(wargs[n], XtNlabel, txt);                 n++;  

  XtSetValues(w, wargs, n);
}

typedef struct StringInfo
{
  Widget str_widget;
  void (*func)(Widget, char *, void *);
  void *user_data;

  struct StringInfo *next;
}StringInfo;

static StringInfo *string_widgets = NULL;


/*
 * Private internal callback for string entry widgets.
 */ 
static void
libsx_done_with_text(Widget w, XEvent *xev, String *parms,
                           Cardinal *num_parms) 
{
  int    n = 0;
  Arg    wargs[10];             /* Used to get widget resources */
  char  *txt;
  StringInfo *stri;

  n = 0;
  XtSetArg(wargs[n], XtNstring,    &txt);                  n++;
  XtGetValues(w, wargs, n);

  /*
   * Find the correct ScrollInfo structure.
   */
  for(stri=string_widgets; stri; stri=stri->next)
   {
     for(; stri && stri->str_widget != w; stri=stri->next)
       ;

     if (stri)      /* didn't find it. */
       break;

     if (XtDisplay(stri->str_widget) == XtDisplay(w))  /* did find it */
       break;
   }
  if (stri == NULL)
    return;

  if (stri->func)
    stri->func(w, txt, stri->user_data);    /* call the user's function */
}


static XtActionsRec actions_table[] =
{
  { "set-okay",       SetOkay },
  { "done_with_text", libsx_done_with_text }
};

static String fallback_resources[] =
{
  "*Dialog*label.resizable: True",
  "*Dialog*Text.resizable: True",
  "*Dialog.Text.translations: #override <Key>Return: set-okay()\\n\
                              <Key>Linefeed: set-okay()",
  NULL
};

static char *app_name="";

/*
 *
 * This is the function that gets everything started.
 *
 */
static int
_OpenDisplay(int argc, char **argv, Display *dpy, Arg *wargs, int arg_cnt)
{
  static char *dummy_argv[] = { "Untitled", NULL };
  static int   dummy_argc  = 1;
  static int already_called = FALSE;

  if (already_called)   /* must have been called a second time */
    return FALSE;
  already_called = TRUE;

  if (argv == NULL)
    {
      argv = dummy_argv;
      argc = dummy_argc;
    }
  

  /*
   * First create the window state.
   */
  lsx_curwin = (WindowState *)calloc(sizeof(WindowState), 1);
  if (lsx_curwin == NULL)
    return FALSE;

  /* open the display stuff */
  if (dpy == NULL)
    lsx_curwin->toplevel = XtAppInitialize(&lsx_app_con, argv[0], NULL, 0,
                                           &argc, argv, fallback_resources,
                                           wargs, arg_cnt);
  else
    lsx_curwin->toplevel = XtAppCreateShell (argv[0], argv[0],
                                             applicationShellWidgetClass,
                                             dpy, wargs, arg_cnt);
  

  if (lsx_curwin->toplevel == NULL)
   {
     free(lsx_curwin);
     return FALSE;
   }

  app_name  = argv[0];   /* save this for later */


  XtAppAddActions(lsx_app_con, actions_table, XtNumber(actions_table));

  lsx_curwin->form_widget = XtCreateManagedWidget("form", formWidgetClass,
                                                  lsx_curwin->toplevel,NULL,0);

  if (lsx_curwin->form_widget == NULL)
   {
     XtDestroyWidget(lsx_curwin->toplevel);
     free(lsx_curwin);
     lsx_curwin = &empty_window;
     return FALSE;
   }
  lsx_curwin->toplevel_form = lsx_curwin->form_widget;
  

  lsx_curwin->next = lsx_windows;    /* link it in to the list */
  lsx_windows = lsx_curwin;

  /* save these values for later */
  lsx_curwin->display = (Display *)XtDisplay(lsx_curwin->toplevel); 
  lsx_curwin->screen  = DefaultScreen(lsx_curwin->display);
  orig_window  = lsx_curwin;
  base_display = lsx_curwin->display;

  return argc;
} /* end of OpenDisplay() */


int OpenDisplay(int argc, char **argv)
{
  return _OpenDisplay(argc, argv, NULL, NULL, 0);
}

typedef struct {
    String name;
    int flag;
} DialogButton;
#define Yes    16
#define No     32
#define Empty  0
#define Okay   1
#define Abort  2
#define Cancel 4
#define Retry  8


static int selected;
#if 0
static DialogButton dialog_buttons[] = {
    {"Yes", Yes},
    {"No", No},
    {"Okay", Okay},
    {"Abort", Abort},
    {"Cancel", Cancel},
    {"Retry", Retry},
};
#endif

static void
SetSelected(Widget w, XtPointer client_data, XtPointer call_data)
{
  selected = (int)client_data;
}

/*
 * Can't make this static because we need to be able to access the
 * name over in display.c to properly set the resources we want.
 */
void SetOkay(Widget w, XEvent* x, String*s, Cardinal*c)
{
  SetSelected(w, (void *)(Okay | Yes), 0 );
}

Widget MakeWindow(char *window_name, char *display_name, int exclusive)
{
  WindowState *win=NULL;
  Display *d=NULL;
  Arg wargs[20];
  int n=0;
  Visual *vis;
  Colormap cmap;
  char *argv[5];
  int   argc;

  if (lsx_curwin->display == NULL)   /* no other windows open yet... */
    return NULL;

  win = (WindowState *)calloc(sizeof(WindowState), 1);
  if (win == NULL)
    return NULL;
  

  /*
   * Setup a phony argv/argc to appease XtOpenDisplay().
   */
  if (window_name)
    argv[0] = window_name;
  else
    argv[0] = app_name;
  argv[1] = NULL;
  argc = 1;

  if (display_name != NULL)
    d = XtOpenDisplay(lsx_app_con, display_name, app_name, app_name, NULL, 0,
                      &argc, argv);
  else
    d = base_display;
  if (d == NULL)
   {
     free(win);
     return NULL;
   }

  win->display  = d;
  

  cmap = DefaultColormap(d, DefaultScreen(d));
  vis  = DefaultVisual(d, DefaultScreen(d));
  

  n=0;
  XtSetArg(wargs[n], XtNtitle,    app_name);      n++;
  XtSetArg(wargs[n], XtNiconName, app_name);      n++;
  XtSetArg(wargs[n], XtNcolormap, cmap);          n++; 
  XtSetArg(wargs[n], XtNvisual,   vis);           n++; 
  
  if (exclusive == FALSE)
   {
     win->toplevel = XtAppCreateShell(argv[0], app_name,
                                      topLevelShellWidgetClass, d, wargs, n);
   }
  else
   {
     win->toplevel = XtCreatePopupShell(argv[0], transientShellWidgetClass,
                                        lsx_curwin->toplevel, NULL, 0);
   }

  if (win->toplevel == NULL)
   {
     if (d != base_display)
       XtCloseDisplay(d);
     free(win);
     return NULL;
   }


  win->form_widget = XtCreateManagedWidget("form", formWidgetClass,
                                           win->toplevel, NULL, 0);
  if (win->form_widget == NULL)
   {
     XtDestroyWidget(win->toplevel);
     if (d != base_display)
       XtCloseDisplay(d);
     free(win);
     return NULL;
   }
  win->toplevel_form = win->form_widget;
  

  win->screen = DefaultScreen(win->display);

  /*
   * Now link in the new window into the window list and make it
   * the current window.
   */
  win->next   = lsx_windows;
  lsx_windows = win;
  lsx_curwin  = win;

  return win->toplevel;    /* return a handle to the user */
}

static Window    window;        /* only used below by the drawing functions. */
static GC        drawgc;
static Display  *display=NULL;
static DrawInfo *cur_di=NULL;   /* current drawing area info structure */

void SetDrawArea(Widget w)
{
  DrawInfo *di;

  if (lsx_curwin->toplevel == NULL || w == NULL)
    return;

  if ((di=libsx_find_draw_info(w)) == NULL)  /* w isn't really a draw area */
    return;

  window  = (Window)XtWindow(w);
  drawgc  = di->drawgc;
  display = XtDisplay(w);
  cur_di  = di;

  lsx_curwin->last_draw_widget = w;
}

void FreeFont(XFont f)
{
  if (lsx_curwin->display && f)
    XFreeFont(lsx_curwin->display, f);
}

static DrawInfo *draw_info_head = NULL;
/*
 * This function searches through our list of drawing area info structures
 * and returns the one that matches the specified widget.  We have to be
 * careful and make sure that the DrawInfo structure we match is for the
 * right display.
 */
DrawInfo *libsx_find_draw_info(Widget w)
{
  DrawInfo *di;
  
  if (w == NULL)
    return NULL;
  
  for(di=draw_info_head;  di; di=di->next)
   {
     for(; di && di->widget != w; di=di->next)
       ;

     if (di == NULL)       /* we didn't find it */
       break;
     if (XtDisplay(di->widget) == XtDisplay(w)) /* then we've found it */
       break;
   }

  return di;
}

int WHITE  = 0,                  /* Global indicies into the color map */
    BLACK  = 0,
    RED    = 0, 
    GREEN  = 0, 
    BLUE   = 0,
    YELLOW = 0;

static void get_color(Colormap cmap, char *name, int *var)
{
  XColor exact, pixel_color;

  if (XAllocNamedColor(lsx_curwin->display, cmap, name, &exact, &pixel_color))
   {
     *var = pixel_color.pixel;
     lsx_curwin->named_colors[lsx_curwin->color_index++] = pixel_color.pixel;
   }
}


static void
GetStandardColors(void)
{
  Colormap mycmap;
  
  if (lsx_curwin->display == NULL || lsx_curwin->has_standard_colors)
    return;

  if (lsx_curwin->cmap == None)
    mycmap = DefaultColormap(lsx_curwin->display,
                             DefaultScreen(lsx_curwin->display));
  else
    mycmap = lsx_curwin->cmap;

  get_color(mycmap, "black", &BLACK);
  get_color(mycmap, "white", &WHITE);
  get_color(mycmap, "red",   &RED);
  get_color(mycmap, "green", &GREEN);
  get_color(mycmap, "blue",  &BLUE);
  get_color(mycmap, "yellow",&YELLOW);

  lsx_curwin->has_standard_colors = TRUE;
}

void MainLoop(void)
{
  if (lsx_curwin->toplevel == NULL)
    return;

  /* in case the user forgot to map the display, do it for them */
  if (lsx_curwin->window_shown == FALSE) 
   {
     ShowDisplay();
     GetStandardColors();
   }

#if 0
  if (XtIsTransientShell(lsx_curwin->toplevel)) /* handle popups differently */
   {
     WindowState *curwin = lsx_curwin;

     while (curwin->window_shown == TRUE)  /* while window is still open */
      {
        XEvent event;
        
        XtAppNextEvent(lsx_app_con, &event);
        XtDispatchEvent(&event);
      }

     /*
      * Ok, at this point the popup was just closed, so now CloseWindow()
      * stored some info for us in the global variable new_curwin (which
      * we use to change lsx_curwin (after free'ing what lsx_curwin used
      * to point to)..
      */
     free(lsx_curwin);
     lsx_curwin = new_curwin;
     new_curwin = NULL;
     
     return;
   }
  else
    XtAppMainLoop(lsx_app_con); 
#else
	{
		WindowState *curwin = lsx_curwin;

		while (curwin->window_shown == TRUE) {
			XEvent event;
			XtAppNextEvent(lsx_app_con, &event);
printf("got...\n");
			XtDispatchEvent(&event);
		}
	}
#endif
}

#define	TXTBUFSIZ	65536
static char txtbuf[TXTBUFSIZ];
static char *curbuf = txtbuf;

void
xputstr(char *txt)
{
	int n, len;
	Arg wargs[2];
	Widget source;

	len = strlen(txt);
	if ((curbuf+len) > &txtbuf[TXTBUFSIZ-1]) {
		errx(1, "repos");
	}

	bcopy(txt, curbuf, len);
	curbuf += len;

	source = XawTextGetSource(main_widget);

	n = 0;
	XtSetArg(wargs[n], XtNstring, txtbuf);  n++;
	XtSetValues(source, wargs, n);
	while (XtAppPending(lsx_app_con) != 0) {
		XEvent event;
		XtAppNextEvent(lsx_app_con, &event);
		XtDispatchEvent(&event);
	}
}

void
x_mainloop()
{
	while (1) {
		XEvent event;

		XtAppNextEvent(lsx_app_con, &event);
		XtDispatchEvent(&event);
	}
}

/*
 * String Entry Widget Creation stuff.
 */
Widget MakeStringEntry(char *txt, int size, StringCB func, void *data)
{
  static int already_done = FALSE;
  static XtTranslations trans;
  int    n = 0;
  Arg    wargs[10];             /* Used to set widget resources */
  Widget str_entry;
  StringInfo *stri;

  if (lsx_curwin->toplevel == NULL && OpenDisplay(0, NULL) == 0)
    return NULL;


  if (already_done == FALSE)
   {
     already_done = TRUE;
     trans = XtParseTranslationTable("#override\n\
                                      <Key>Return: done_with_text()\n\
                                      <Key>Linefeed: done_with_text()\n\
                                      Ctrl<Key>M: done_with_text()\n\
                                      Ctrl<Key>J: done_with_text()\n");
   }

  stri = (StringInfo *)malloc(sizeof(*stri));
  if (stri == NULL)
    return NULL;

  stri->func      = func;
  stri->user_data = data;


  n = 0;
  XtSetArg(wargs[n], XtNeditType,     XawtextEdit);           n++;
  XtSetArg(wargs[n], XtNwrap,         XawtextWrapNever);      n++;
  XtSetArg(wargs[n], XtNresize,       XawtextResizeWidth);    n++;
  XtSetArg(wargs[n], XtNtranslations, trans);                 n++;
  XtSetArg(wargs[n], XtNwidth,        size);                  n++;
  if (txt)
   {
     XtSetArg(wargs[n], XtNstring,    txt);                   n++;
     XtSetArg(wargs[n], XtNinsertPosition,  strlen(txt));     n++;
   }


  str_entry = XtCreateManagedWidget("string", asciiTextWidgetClass,
                                    lsx_curwin->form_widget,wargs,n);

  if (str_entry)  /* only if we got a real widget do we bother */
   {
     stri->str_widget = str_entry;
     stri->next = string_widgets;
     string_widgets = stri;
   }
  else
    free(stri);

  return str_entry;
}                    /* end of MakeStringEntry() */

