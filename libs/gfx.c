/*
A simple graphics library for CSE 20211 by Douglas Thain

This work is licensed under a Creative Commons Attribution 4.0 International
License.  https://creativecommons.org/licenses/by/4.0/

For complete documentation, see:
http://www.nd.edu/~dthain/courses/cse20211/fall2013/gfx
Version 3, 11/07/2012 - Now much faster at changing colors rapidly.
Version 2, 9/23/2011 - Fixes a bug that could result in jerky animation.
*/

#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "gfx.h"

/*
gfx_open creates several X11 objects, and stores them in globals
for use by the other functions in the library.
*/

static Display *gfx_display = 0;
static Window gfx_window;
static GC gfx_gc;
static Colormap gfx_colormap;
static int gfx_fast_color_mode = 0;

static const char *fontSizes[] = {
	[SMALL] = "6x10",
	[MEDIUM] = "9x15",
	[LARGE] = "12x24"
};

static XFontSet fontSets[3];

/* These values are saved by gfx_wait then retrieved later by gfx_xpos and
 * gfx_ypos. */

static int saved_xpos = 0;
static int saved_ypos = 0;

/* Open a new graphics window. */

void gfx_open(int width, int height, const char *title) {
  gfx_display = XOpenDisplay(0);
  if (!gfx_display) {
    fprintf(stderr, "gfx_open: unable to open the graphics window.\n");
    exit(1);
  }

  Visual *visual = DefaultVisual(gfx_display, 0);
  if (visual && visual->class == TrueColor) {
    gfx_fast_color_mode = 1;
  } else {
    gfx_fast_color_mode = 0;
  }

  int blackColor = BlackPixel(gfx_display, DefaultScreen(gfx_display));
  int whiteColor = WhitePixel(gfx_display, DefaultScreen(gfx_display));

  gfx_window =
      // XCreateWindow(gfx_display, DefaultRootWindow(gfx_display), 0, 0, 
      //               width, height, 0, 0, 0, visual, CWBitGravity, &(XSetWindowAttributes){.bit_gravity = CenterGravity});
      XCreateSimpleWindow(gfx_display, DefaultRootWindow(gfx_display), 0, 0,
                          width, height, 0, blackColor, blackColor);

  XSetWindowAttributes attr;
  attr.backing_store = Always;
  attr.bit_gravity = CenterGravity;
  

  // XChangeWindowAttributes(gfx_display, gfx_window, CWBackingStore, &attr);
  XChangeWindowAttributes(gfx_display, gfx_window, CWBackingStore | CWBitGravity, &attr);

  XStoreName(gfx_display, gfx_window, title);

  XSelectInput(gfx_display, gfx_window,
               StructureNotifyMask | KeyPressMask | ButtonPressMask);

  XMapWindow(gfx_display, gfx_window);

  gfx_gc = XCreateGC(gfx_display, gfx_window, 0, 0);

  gfx_colormap = DefaultColormap(gfx_display, 0);

  XSetForeground(gfx_display, gfx_gc, whiteColor);

  char **missing_charset_list;
  int missing_charset_count = 0;
  char *default_string = "?";
  
  fontSets[SMALL] = XCreateFontSet(gfx_display, "6x10", &missing_charset_list, &missing_charset_count, &default_string);
  fontSets[MEDIUM] = XCreateFontSet(gfx_display, "9x15", &missing_charset_list, &missing_charset_count, &default_string);
  fontSets[LARGE] = XCreateFontSet(gfx_display, "12x24", &missing_charset_list, &missing_charset_count, &default_string);

  // Wait for the MapNotify event

  for (;;) {
    XEvent e;
    XNextEvent(gfx_display, &e);
    if (e.type == MapNotify)
      break;
  }
}

void gfx_text2(int x, int y, const char *text, Color bg, Color fg) {
  int colorBg = XAllocColor(gfx_display, DefaultColormap(gfx_display, DefaultScreen(gfx_display)), &(XColor){.red = bg.r, .green = bg.g, .blue = bg.b});
  int colorFg = XAllocColor(gfx_display, DefaultColormap(gfx_display, DefaultScreen(gfx_display)), &(XColor){.red = fg.r, .green = fg.g, .blue = fg.b});
  XSetBackground(gfx_display, gfx_gc, colorBg);
  XSetForeground(gfx_display, gfx_gc, colorFg);
  XDrawString(gfx_display, gfx_window, gfx_gc, x, y, text, strlen(text));
}

int utf8toXChar2b(XChar2b *output_r, int outsize, const char *input, int inlen){
	int j, k;
	for(j =0, k=0; j < inlen && k < outsize; j ++){
		unsigned char c = input[j];
		if (c < 128)  {
			output_r[k].byte1 = 0;
			output_r[k].byte2 = c; 
			k++;
		} else if (c < 0xC0) {
			/* we're inside a character we don't know  */
			continue;
		} else switch(c&0xF0){
		case 0xC0: case 0xD0: /* two bytes 5+6 = 11 bits */
			if (inlen < j+1){ return k; }
			output_r[k].byte1 = (c&0x1C) >> 2;
			j++;
			output_r[k].byte2 = ((c&0x3) << 6) + (input[j]&0x3F);
			k++;
			break;
		case 0xE0: /* three bytes 4+6+6 = 16 bits */ 
			if (inlen < j+2){ return k; }
			j++;
			output_r[k].byte1 = ((c&0xF) << 4) + ((input[j]&0x3C) >> 2);
			c = input[j];
			j++;
			output_r[k].byte2 = ((c&0x3) << 6) + (input[j]&0x3F);
			k++;
			break;
		case 0xFF:
			/* the character uses more than 16 bits */
			continue;
		}
	}
	return k;
}

void gfx_text_utf8(int x, int y, const char *text, FontSize fontSize) {
  // const char *fontset_name = fontSets[fontSize];
  // XFontSet fontset; 

  
  XFontSet fontset = fontSets[fontSize];
  if (fontset == NULL) {
      fprintf(stderr, "Invalid fontset\n");
      return;
  }
  // printf("locale: %s (suports? %d)\n", XLocaleOfFontSet(fontset), XSupportsLocale());
  XmbDrawString(gfx_display, gfx_window, fontset, gfx_gc, x, y, text, strlen(text));

  // XFontSet fontSet = XCreateFontSet(gfx_display, fontSizes[fontSize], NULL, NULL, NULL);
  // Xutf8DrawString(gfx_display, gfx_window, fontSet, gfx_gc, x, y, text, strlen(text));
}

void gfx_set_background(int r, int g, int b) {
  XColor color;
  color.pixel = 0;
  color.red = r << 8;
  color.green = g << 8;
  color.blue = b << 8;
  XAllocColor(gfx_display, gfx_colormap, &color);
  XSetWindowAttributes attr;
  attr.background_pixel = color.pixel;
  XChangeWindowAttributes(gfx_display, gfx_window, CWBackPixel, &attr);
}

/* Draw a single point at (x,y) */

void gfx_point(int x, int y) {
  XDrawPoint(gfx_display, gfx_window, gfx_gc, x, y);
}

/* Draw a line from (x1,y1) to (x2,y2) */

void gfx_line(int x1, int y1, int x2, int y2) {
  XDrawLine(gfx_display, gfx_window, gfx_gc, x1, y1, x2, y2);
}

void gfx_circle(int x, int y, int r) {
  XDrawArc(gfx_display, gfx_window, gfx_gc, x, y, r, r, 0, 360 * 64);
}

void gfx_rectangle(int x, int y, int w, int h) {
  XDrawRectangle(gfx_display, gfx_window, gfx_gc, x, y, w, h);
}

void gfx_fill_rectangle(int x, int y, int w, int h) {
  XFillRectangle(gfx_display, gfx_window, gfx_gc, x, y, w, h);
}

void gfx_fill_circle(int x, int y, int r) {
  XFillArc(gfx_display, gfx_window, gfx_gc, x, y, r, r, 0, 360 * 64);
}

void gfx_fill_triangle(int x1, int y1, int x2, int y2, int x3, int y3) {
  XPoint points[3];
  points[0].x = x1;
  points[0].y = y1;
  points[1].x = x2;
  points[1].y = y2;
  points[2].x = x3;
  points[2].y = y3;
  XFillPolygon(gfx_display, gfx_window, gfx_gc, points, 3, Convex, CoordModeOrigin);
}

void gfx_triangle(int x1, int y1, int x2, int y2, int x3, int y3) {
  XPoint points[3];
  points[0].x = x1;
  points[0].y = y1;
  points[1].x = x2;
  points[1].y = y2;
  points[2].x = x3;
  points[2].y = y3;
  XDrawLines(gfx_display, gfx_window, gfx_gc, points, 3, CoordModeOrigin);
}

void gfx_fill_polygon(int *x, int *y, int n) {
  XPoint points[n];
  for (int i = 0; i < n; i++) {
    points[i].x = x[i];
    points[i].y = y[i];
  }
  XFillPolygon(gfx_display, gfx_window, gfx_gc, points, n, Convex, CoordModeOrigin);
}

void gfx_polygon(int *x, int *y, int n) {
  XPoint points[n];
  for (int i = 0; i < n; i++) {
    points[i].x = x[i];
    points[i].y = y[i];
  }
  XDrawLines(gfx_display, gfx_window, gfx_gc, points, n, CoordModeOrigin);
}

void gfx_fill_arc(int x, int y, int w, int h, int angle1, int angle2) {
  XFillArc(gfx_display, gfx_window, gfx_gc, x, y, w, h, angle1, angle2);
}

void gfx_arc(int x, int y, int w, int h, int angle1, int angle2) {
  XDrawArc(gfx_display, gfx_window, gfx_gc, x, y, w, h, angle1, angle2);
}

void gfx_fill_ellipse(int x, int y, int w, int h) {
  XFillArc(gfx_display, gfx_window, gfx_gc, x, y, w, h, 0, 360 * 64);
}

void gfx_ellipse(int x, int y, int w, int h) {
  XDrawArc(gfx_display, gfx_window, gfx_gc, x, y, w, h, 0, 360 * 64);
}

void gfx_fill_pie(int x, int y, int w, int h, int angle1, int angle2) {
  XFillArc(gfx_display, gfx_window, gfx_gc, x, y, w, h, angle1, angle2);
}

void gfx_pie(int x, int y, int w, int h, int angle1, int angle2) {
  XDrawArc(gfx_display, gfx_window, gfx_gc, x, y, w, h, angle1, angle2);
}

/* Change the current drawing color. */

void gfx_color(int r, int g, int b) {
  XColor color;

  if (gfx_fast_color_mode) {
    /* If this is a truecolor display, we can just pick the color directly. */
    color.pixel = ((b & 0xff) | ((g & 0xff) << 8) | ((r & 0xff) << 16));
  } else {
    /* Otherwise, we have to allocate it from the colormap of the display. */
    color.pixel = 0;
    color.red = r << 8;
    color.green = g << 8;
    color.blue = b << 8;
    XAllocColor(gfx_display, gfx_colormap, &color);
  }

  XSetForeground(gfx_display, gfx_gc, color.pixel);
}

void gfx_bgcolor(int r, int g, int b) {
  XColor color;

  if (gfx_fast_color_mode) {
    /* If this is a truecolor display, we can just pick the color directly. */
    color.pixel = ((b & 0xff) | ((g & 0xff) << 8) | ((r & 0xff) << 16));
  } else {
    /* Otherwise, we have to allocate it from the colormap of the display. */
    color.pixel = 0;
    color.red = r << 8;
    color.green = g << 8;
    color.blue = b << 8;
    XAllocColor(gfx_display, gfx_colormap, &color);
  }
  XSetBackground(gfx_display, gfx_gc, color.pixel);
}

/* Clear the graphics window to the background color. */

void gfx_clear() { XClearWindow(gfx_display, gfx_window); }

/* Change the current background color. */

void gfx_clear_color(int r, int g, int b) {
  XColor color;
  color.pixel = 0;
  color.red = r << 8;
  color.green = g << 8;
  color.blue = b << 8;
  XAllocColor(gfx_display, gfx_colormap, &color);

  XSetWindowAttributes attr;
  attr.background_pixel = color.pixel;
  XChangeWindowAttributes(gfx_display, gfx_window, CWBackPixel, &attr);
}

int gfx_event_waiting() {
  XEvent event;

  gfx_flush();

  while (1) {
    if (XCheckMaskEvent(gfx_display, -1, &event)) {
      if (event.type == KeyPress) {
        XPutBackEvent(gfx_display, &event);
        return 1;
      } else if (event.type == ButtonPress) {
        XPutBackEvent(gfx_display, &event);
        return 1;
      } else {
        return 0;
      }
    } else {
      return 0;
    }
  }
}

/* Wait for the user to press a key or mouse button. */

char gfx_wait() {
  XEvent event;


  while (1) {
    gfx_flush();
    XNextEvent(gfx_display, &event);
    if (event.type == KeyPress) {
      saved_xpos = event.xkey.x;
      saved_ypos = event.xkey.y;
      return XLookupKeysym(&event.xkey, 0);
    } else if (event.type == ButtonPress) {
      saved_xpos = event.xkey.x;
      saved_ypos = event.xkey.y;
      return event.xbutton.button;
    }
  }
}

void gfx_iddle(int (*function)(char), char stop_key) {
  XEvent event;

  char key = '\0';
  while (1) {
    if(function(key) != 0) {
      break;
    }
    gfx_flush();
    // XNextEvent(gfx_display, &event);
    if(XCheckMaskEvent(gfx_display, -1, &event)) {
      if (event.type == KeyPress) {
        saved_xpos = event.xkey.x;
        saved_ypos = event.xkey.y;
        key = XLookupKeysym(&event.xkey, 0);
      }
      if(key == stop_key) {
        break;
      }
    }
  }
}

/* Return the X and Y coordinates of the last event. */

int gfx_xpos() { return saved_xpos; }

int gfx_ypos() { return saved_ypos; }

/* Flush all previous output to the window. */

void gfx_flush() { XFlush(gfx_display); }

void gfx_text(int x, int y, const char *text, FontSize fontSize) {
  XSetFont(gfx_display, gfx_gc, XLoadFont(gfx_display, fontSizes[fontSize]));
  XDrawString(gfx_display, gfx_window, gfx_gc, x, y, text, strlen(text));
  // XDrawImageString(gfx_display, gfx_window, gfx_gc, x, y, text, strlen(text));
}

int gfx_xsize() {
  XWindowAttributes window_attributes_return;
  XGetWindowAttributes(gfx_display, gfx_window, &window_attributes_return);
  return window_attributes_return.width;
}

int gfx_ysize() {
  XWindowAttributes window_attributes_return;
  XGetWindowAttributes(gfx_display, gfx_window, &window_attributes_return);
  return window_attributes_return.height;
}