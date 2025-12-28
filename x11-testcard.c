/*
 * x11-scale-area-test.c
 *
 * Copyright(C) 2025   MT
 *
 * Display a 'Test Card' in a re-sizeable window using X11.
 * 
 * This  program is free software: you can redistribute it and/or modify it
 * under  the terms of the GNU General Public License as published  by  the
 * Free  Software Foundation, either version 3 of the License, or (at  your
 * option) any later version.
 *
 * This  program  is distributed in the hope that it will  be  useful,  but
 * WITHOUT   ANY   WARRANTY;   without even   the   implied   warranty   of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You  should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * 03 Oct 25  0.1.0001  - Initial version - MT
 * 05 Oct 25            - Added command line options - MT
 * 28 Oct 25  0.2       - Parse geometry using XParseGeometry() - MT
 * 27 Dec 25            - Disabling  the  background  pixmap  prevents  the 
 *                        window manager from updating the background  when 
 *                        an  expose  event occurs, which stops the  window 
 *                        flickering when it is redrawn - MT
 * 
 */

#define  NAME           "x11 Scale Area Test" 
#define  VERSION        "0.1"
#define  BUILD          "0001"
#define  DATE           "23 Sep 25"
#define  AUTHOR         "MT"

#define  WIDTH          640  /* 200 */
#define  HEIGHT         480  /* 450 */

#define  COLOUR_DEPTH   24

#define  DEBUG

#include <errno.h>      /* errno */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>     /* vargs(), etc */ 
#include <string.h>

#include <ctype.h>

#include <errno.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>  /* XA_ATOM, etc */

#include "x11-scale-area.h"
#include "gcc-debug.h"

const char *h_err_display = "Cannot connect to X server '%s'\n";
const char *h_err_display_properties = "Unable to get display properties\n";
const char *h_err_display_colour = "Requires a %d-bit colour display\n";

void v_version() { /* Display version information */
   fprintf(stdout, "%s: Version %s ", NAME, VERSION);
   if (__DATE__[4] == ' ') fprintf(stdout, "(0"); else fprintf(stdout, "(%c", __DATE__[4]);
   fprintf(stdout, "%c %c%c%c %s %s)", __DATE__[5], __DATE__[0], __DATE__[1], __DATE__[2], &__DATE__[9], __TIME__ );
   fprintf(stdout,"\n");
   fprintf(stdout, "Copyright(C) %s %s\n", __DATE__ +7, AUTHOR);
   fprintf(stdout, "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.\n");
   fprintf(stdout, "This is free software: you are free to change and redistribute it.\n");
   fprintf(stdout, "There is NO WARRANTY, to the extent permitted by law.\n");
   exit(0);
}
 
void v_about() { /* Display help text */
   fprintf(stdout, "Usage: %s [OPTION]... [\n", NAME);
   fprintf(stdout, "Concatenate FILE(s)to standard output.\n\n");
   fprintf(stdout, "      --geometry +x+y      specify initial window position\n");
   fprintf(stdout, "  -?, --help               display this help and exit\n");
   fprintf(stdout, "      --version            output version information and exit\n\n");
   exit(0);
}

void v_error(int i_errno, const char *s_format, ...)  /* Print formatted error message and exit returning errno */
{
   va_list t_args;
   if (!(i_errno)) i_errno = -1;  /* If errno not set return -1 */
   va_start(t_args, s_format);
   fprintf(stderr, "%s: ", NAME);
   vfprintf(stderr, s_format, t_args);
   fprintf(stderr, "\n");
   va_end(t_args);
   exit(i_errno);
}

void v_warning(const char *s_format, ...)  /* Print formatted warning message */
{
   va_list t_args;
   va_start(t_args, s_format);
   fprintf(stderr, "%s: ", NAME);
   vfprintf(stderr, s_format, t_args);
   fprintf(stderr, "\n");
   va_end(t_args);
}

/* Draw a test card image onto a Pixmap */
void v_draw_test_card(Display *x_display, Pixmap x_pixmap, GC x_context, int i_screen, int i_width, int i_height)
{
   Colormap x_colormap;
   XColor x_color;
   Pixmap x_stipple;
   const char *c_colours[] = {"Red", "Orange", "Yellow", "Green", "Medium Blue", "Blue Violet", "Violet", "Black"};
   const char *c_greys[] = {"grey0", "grey6", "grey12", "grey18", "grey25", "grey31", "grey37", "grey43", "grey50", 
                            "grey56", "grey62", "grey68", "grey75", "grey81", "grey87", "grey100"};
   const char *c_fill = "LightSkyBlue";
   const unsigned char c_pattern[] = {
      0x11, 0x11, 0xb8, 0xb8, 0x7c, 0x7c, 0x3a, 0x3a, 0x11, 0x11, 0xa3, 0xa3,
      0xc7, 0xc7, 0x8b, 0x8b, 0x11, 0x11, 0xb8, 0xb8, 0x7c, 0x7c, 0x3a, 0x3a,
      0x11, 0x11, 0xa3, 0xa3, 0xc7, 0xc7, 0x8b, 0x8b};
   unsigned long i_foreground;  /* Foreground colour (black) */
   unsigned long i_background;  /* Background colour (white) */
   unsigned long i_fill;
   unsigned long i_colour;
   int i_vertical_lines = 5;
   int i_horizontal_lines = 4;
   int i_bar_width;
   int i_colours;
   int i_count, i_position;

   /* Get the default colormap for the screen */
   x_colormap = DefaultColormap(x_display, i_screen);

   i_foreground = BlackPixel(x_display, i_screen);
   i_background = WhitePixel(x_display, i_screen);

   if (!XParseColor(x_display, x_colormap, c_greys[8], &x_color) || !XAllocColor(x_display, x_colormap, &x_color)) 
      i_colour = i_foreground;
   else 
      i_colour = x_color.pixel;
               
   /* raw stippled pattern on background */
   XSetForeground(x_display, x_context, i_colour);  /* Set foreground and background colours */
   XSetBackground(x_display, x_context, i_background);
   x_stipple = XCreateBitmapFromData(x_display, RootWindow(x_display, i_screen), (char *)c_pattern, 16, 16);  /* Create stipple pixmap */
   if (x_stipple != None) 
   {
      XSetStipple(x_display, x_context, x_stipple);  /* Set stipple into graphics context */
      XSetFillStyle(x_display, x_context, FillOpaqueStippled);  /* Use opaque stipple so both foreground and background are drawn */
      XFillRectangle(x_display, x_pixmap, x_context, 0, 0, (unsigned int)i_width, (unsigned int)i_height);  /* Fill background with stipple */
      XSetFillStyle(x_display, x_context, FillSolid); /* Reset fill style back to solid for later drawing */
      XFreePixmap(x_display, x_stipple);  /* Free stipple pixmap */
   }
   else
      XFillRectangle(x_display, x_pixmap, x_context, 0, 0, (unsigned int)i_width, (unsigned int)i_height);  /* Fill background with solid colour */

   /* Draw colour bars at top */
   i_colours = sizeof(c_colours) / sizeof(c_colours[0]);
  
   i_bar_width = i_width / i_colours;
   for (i_count = 0; i_count < i_colours; ++i_count) {
      if (!XParseColor(x_display, x_colormap, c_colours[i_count], &x_color) || !XAllocColor(x_display, x_colormap, &x_color)) 
         i_colour = i_foreground;
      else 
         i_colour = x_color.pixel;
      XSetForeground(x_display, x_context, i_colour);
      XFillRectangle(x_display, x_pixmap, x_context, i_count * i_bar_width, 0, (unsigned int)i_bar_width, (unsigned int)(i_height / 5));
   }
   
   /* Draw grey bars at bottom */
   i_colours = sizeof(c_greys) / sizeof(c_greys[0]);
  
   i_bar_width = i_width / i_colours;
   for (i_count = 0; i_count < i_colours; ++i_count) {
      if (!XParseColor(x_display, x_colormap, c_greys[i_count], &x_color) || !XAllocColor(x_display, x_colormap, &x_color)) 
         i_colour = i_foreground;
      else 
         i_colour = x_color.pixel;
      XSetForeground(x_display, x_context, i_colour);
      XFillRectangle(x_display, x_pixmap, x_context, i_count * i_bar_width, i_height - (i_height / 5), (unsigned int)i_bar_width, (unsigned int)(i_height / 5));
   }

   /* Draw horizontal and vertical lines */
   XSetForeground(x_display, x_context, i_foreground);
   for (i_count = 1; i_count <= i_vertical_lines; ++i_count)  /* Vertical lines */
   {
      i_position = (i_count * i_width) / (i_vertical_lines + 1);
      XDrawLine(x_display, x_pixmap, x_context, i_position, i_height / 5, i_position, i_height - i_height / 5);
   }

   for (i_count = 0; i_count < i_horizontal_lines; ++i_count)  /* Horizontal lines */
   {
      i_position = (i_height / 5) + (i_count * (i_height - i_height / 5)) / i_horizontal_lines;
      XDrawLine(x_display, x_pixmap, x_context, 0, i_position, i_width, i_position);
   }

   /* Draw circle in the middle of the test card */
   int i_x_center = i_width / 2;
   int i_y_center = i_height / 2;
   int i_radius = (i_width < i_height ? i_width : i_height) / 4;

   if (!XParseColor(x_display, x_colormap, c_fill, &x_color) || !XAllocColor(x_display, x_colormap, &x_color)) 
      i_fill = i_background;
   else 
      i_fill = x_color.pixel;
      
   XSetForeground(x_display, x_context, i_fill);
   XFillArc(x_display, x_pixmap, x_context,
          i_x_center - i_radius, i_y_center - i_radius,
          (unsigned int)(2 * i_radius), (unsigned int)(2 * i_radius),
          0, 360 * 64);
   XSetForeground(x_display, x_context, i_foreground);
   XDrawArc(x_display, x_pixmap, x_context,
          i_x_center - i_radius, i_y_center - i_radius,
          (unsigned int)(2 * i_radius), (unsigned int)(2 * i_radius),
          0, 360 * 64);

}


int main(int argc, char *argv[])
{
   Display *x_display;                 /* Pointer to display structure */
   Window x_window;                    /* Application window structure */
   GC x_context;                       /* Graphics context */
   Pixmap x_image;                     /* Test image */
   XEvent x_event;
   XSizeHints h_size_hint;
   Atom wm_delete;
   XRectangle o_image_size;
   XRectangle o_screen_size;

   char *s_title = NAME;               /* Windows title */
   char *s_display_name = "";          /* Use the default display */

   unsigned int i_width;
   unsigned int i_height;
   unsigned int i_window_width;        /* Window's width */
   unsigned int i_window_height;       /* Window's height */
   unsigned int i_window_border = 4;   /* Window's border width */
   unsigned int i_colour_depth;        /* Window's colour depth */
   int i_window_left, i_window_top;    /* Window's position */
   int i_screen;                       /* Default screen number */

   int i_count, i_index;
   char b_abort = 0;

   if (!(x_display = XOpenDisplay(s_display_name))) v_error (errno, h_err_display, s_display_name);  /* Open the default display */

   i_screen = DefaultScreen(x_display);  /* Get the default screen for our X server */
   o_screen_size.width = DisplayWidth(x_display, i_screen);
   o_screen_size.height = DisplayHeight(x_display, i_screen);

   /* Default windows position */
   i_window_width = WIDTH;  /* Default Window width in pixels */
   i_window_height = HEIGHT;  /* Window height in pixels */
   i_window_left = (o_screen_size.width -i_window_width) / 2 ;  /* Centre window on screen - ignored by most window managers but useful in kiosk mode */
   i_window_top = (o_screen_size.height - i_window_height) / 2;

   debug(printf("(%dx%d%+d%+d)\n", i_window_width, i_window_height, i_window_left, i_window_top));   

   for (i_count = 1; i_count < argc && !b_abort; i_count++) 
   {
      if (argv[i_count][0] == '-') 
      {
         i_index = 1;
         while (argv[i_count][i_index] != 0) 
         {
            switch (argv[i_count][i_index]) 
            {
            case '?': v_about(); break;
            case '-':
               i_index = strlen(argv[i_count]);
               if (i_index == 2) 
               {
                  b_abort = 1;
               } 
               else if (!strncmp(argv[i_count], "--version", i_index))
               {
                  v_version();
               } else if (!strncmp(argv[i_count], "--help", i_index))
               {
                  v_about();
               }

               else if (!strncmp(argv[i_count], "--geometry=", 11)) /* Just check the first 11 characters match */
               {
                  char *c_geometry = argv[i_count] + 11;
                  char *c_char;

                  i_window_left = 0;
                  i_window_top = 0;
                  for (c_char = c_geometry; *c_char; c_char++) /* Convert to lowercase before parsing */
                     *c_char = (char)tolower((unsigned char)*c_char);
                  if (XParseGeometry(c_geometry,  &i_window_left, &i_window_top, &i_window_width, &i_window_height))
                  {
                     if (i_count + 1 < argc)  /* Remove the parameter from the arguments */
                        for (int i_offset = i_count + 1; i_offset < argc - 1; i_offset++)
                           argv[i_offset] = argv[i_offset + 1];
                     argc--;
                  } 
                  else 
                     v_error(-1, "invalid geometry: %s (expected: WxH[+X+Y])\n", c_geometry);
               }

               else if (!strncmp(argv[i_count], "--geometry", i_index))
               {
                  if (i_count + 1 < argc) 
                  {
                     char *c_geometry = argv[i_count + 1];
                     char *c_char;

                     i_window_left = 0;
                     i_window_top = 0;
                     for (c_char = c_geometry; *c_char; c_char++) /* Convert to lowercase before parsing */
                        *c_char = (char)tolower((unsigned char)*c_char);
                     if (XParseGeometry(c_geometry,  &i_window_left, &i_window_top, &i_window_width, &i_window_height))
                     {
                        if (i_count + 2 < argc)  /* Remove the parameter from the arguments */
                           for (int i_offset = i_count + 1; i_offset < argc - 1; i_offset++)
                              argv[i_offset] = argv[i_offset + 1];
                        argc--;
                     } 
                     else 
                        v_error(-1, "invalid geometry: %s (expected: WxH[+X+Y])\n", c_geometry);
                  }
                  else
                     v_error(-1, "missing argument%s\nTry '%s --help' for more information.\n", argv[i_count], NAME);
               }

               else if (!strncmp(argv[i_count], "--help", i_index)) 
                  v_about();
               else 
                  v_error(-1, "invalid option %s\nTry '%s --help' for more information.\n", argv[i_count], NAME);

               i_index--;
               break;
            default:
               v_error(-1, "unknown option -- %c\nTry '%s --help' for more information.\n", argv[i_count][i_index], NAME);
            }
            i_index++;
         }
         if (argv[i_count][1] != 0) /* Shift arguments */
            for (i_index = i_count; i_index < argc - 1; i_index++) argv[i_index] = argv[i_index + 1];
               argc--; i_count--;
      }
   }

   debug(printf("(%dx%d%+d%+d)\n", i_window_width, i_window_height, i_window_left, i_window_top));   

   x_window = XCreateSimpleWindow(x_display, /* Create the application window, as a child of the root window */
      XDefaultRootWindow(x_display),
      i_window_left, i_window_top, /* Windows position */
      i_window_width, i_window_height, /* Window size */
      i_window_border, /* Border width - ignored ? */
      BlackPixel(x_display, i_screen), /* Border colour - ignored ? */
      BlackPixel(x_display, i_screen)); /* Background colour */

   /* Define the minimum size (client area, excluding borders) */
   h_size_hint.min_width  = i_window_width;
   h_size_hint.min_height = i_window_height;
   
   /* Define the maximum size */
   h_size_hint.max_width  = i_window_width * 1.5;
   h_size_hint.max_height = i_window_height * 1.5;

   /* Use a fixed aspect ratio */
   h_size_hint.min_aspect.x = i_window_width;
   h_size_hint.min_aspect.y = i_window_height;
   h_size_hint.max_aspect.x = i_window_width;
   h_size_hint.max_aspect.y = i_window_height;

   h_size_hint.flags = PMinSize | PMaxSize | PAspect;
   /** XSetWMNormalHints(x_display, x_window, &h_size_hint); /* Set by XSetStandardProperties */
   
   /* Get window geometry (necessary to get colour depth) */
   if (XGetGeometry(x_display, x_window, &RootWindow(x_display, i_screen), &i_window_left, &i_window_top, &i_window_width, &i_window_height, &i_window_border,  &i_colour_depth) == False)
      v_error(errno, h_err_display_properties);
   
   wm_delete = XInternAtom(x_display, "WM_DELETE_WINDOW", False);
   XSetWMProtocols(x_display, x_window, &wm_delete, DefaultDepth (x_display, i_screen));

   /* Set window title and size */
   XSetStandardProperties(x_display, x_window, s_title, s_title, None, argv, argc, &h_size_hint);  /* Set the window title and icon */
   
   /* Declare the atoms we want to allow as window actions */
   Atom allowed_actions[3];
   allowed_actions[0] = XInternAtom(x_display, "_NET_WM_ACTION_RESIZE", False);  /* allow resizing */
   allowed_actions[1] = XInternAtom(x_display, "_NET_WM_ACTION_MOVE", False);    /* allow moving */
   allowed_actions[2] = XInternAtom(x_display, "_NET_WM_ACTION_CLOSE", False);   /* allow closing */

   /* Get the atom for the property _NET_WM_ALLOWED_ACTIONS */
   Atom net_wm_allowed_actions = XInternAtom(x_display, "_NET_WM_ALLOWED_ACTIONS", False);

   XChangeProperty(x_display, x_window,
                   net_wm_allowed_actions, XA_ATOM, 32,
                   PropModeReplace,
                   (unsigned char *)allowed_actions,
                   3);


   /* Disable background to prevent the window manager from clearing the window before it is redrawn */
   XSetWindowBackgroundPixmap(x_display, x_window, None);

   /* Select input events and map (show) the window */
   XSelectInput(x_display, x_window, ExposureMask | KeyPressMask | StructureNotifyMask);
   XMapWindow(x_display, x_window);

   x_context = XCreateGC(x_display, x_window, 0, NULL);

   if (i_colour_depth != COLOUR_DEPTH) v_error(errno, h_err_display_colour, COLOUR_DEPTH);  /* Check colour depth */

   o_image_size.width = i_window_width;
   o_image_size.height = i_window_height;
   x_image = XCreatePixmap(x_display, x_window, o_image_size.width, o_image_size.height, DefaultDepth(x_display, i_screen)); /* Create a source pixmap */
   v_draw_test_card(x_display, x_image, x_context, i_screen, o_image_size.width, o_image_size.height);

   XSetForeground(x_display, x_context, BlackPixel(x_display, i_screen));
   
   while (True) {
      XNextEvent(x_display, &x_event);

      if (x_event.type == ConfigureNotify) /* Window was resized */
      {
         i_window_width = x_event.xconfigure.width;
         i_window_height = x_event.xconfigure.height;
      }

      if (x_event.type == Expose) 
      {
         if (i_window_width > h_size_hint.max_width) i_width = h_size_hint.max_width; else i_width =  i_window_width;
         if (i_window_height > h_size_hint.max_height) i_height = h_size_hint.max_height; else i_height =  i_window_height;
         i_width =  i_window_width; i_height =  i_window_height;
         debug(printf("%d x %d (%d x %d) (%d%%)\n", h_size_hint.min_width, h_size_hint.min_height, i_window_width, i_window_height, (int)((float) i_window_height / h_size_hint.min_height * 100.0)));

         XScaleArea(x_display, x_image, x_window, x_context, 0, 0, o_image_size.width, o_image_size.height, (i_window_width - i_width) / 2, (i_window_height - i_height) / 2, i_width, i_height);

         //XFillRectangle(x_display, x_window, x_context, 0, 0, i_window_width, i_window_height);  /* Fill window to clear it */
         //XCopyArea(x_display, x_image, x_window, x_context, 0, 0, o_image_size.width, o_image_size.height, (i_window_width - o_image_size.width) / 2, (i_window_height - o_image_size.height) / 2);
      }

      if (x_event.type == ClientMessage) 
         if ((Atom) x_event.xclient.data.l[0] == wm_delete) break;

      if (x_event.type == KeyPress) {
         break; /* exit on any key press */
      }
   }

   XFreePixmap(x_display, x_image);
   XFreeGC(x_display, x_context);
   XDestroyWindow(x_display, x_window);
   XCloseDisplay(x_display);
   return 0;
}
