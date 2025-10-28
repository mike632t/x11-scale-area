/*
 * x11-font.h - RPN (Reverse Polish) calculator simulator.
 *
 * Copyright(C) 2013   MT
 *
 * Font definations and data structures.
 *
 * This  program is free software: you can redistribute it and/or modify  it
 * under  the  terms of the GNU General Public License as published  by  the
 * Free  Software  Foundation, either version 3 of the License, or (at  your
 * option) any later version.
 *
 * This  program  is  distributed in the hope that it will  be  useful,  but
 * WITHOUT   ANY   WARRANTY;   without  even   the   implied   warranty   of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
 * Public License for more details.
 *
 * You  should have received a copy of the GNU General Public License  along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * 09 Mar 14         - Initial version - MT
 * 10 Dec 18         - Defined DEC fonts if compiling on VMS - MT
 * 24 Feb 24         - Defined an array of alternative fonts for each style
 *                     of text required - MT
 * 24 Feb 24         - Moved font definitions to "x11-fonts.c" - MT
 *                   - Defined an array of alternative fonts for each style
 *                     of  text required and select the most suitable fonts
 *                     from these alternatives to reduce the dependency  on
 *                     a single set of fonts - MT
 * 22 Apr 24         - Tidied up function prototypes - MT
 *
 *
 */

XFontStruct *h_normal_font;
XFontStruct *h_small_font;
XFontStruct *h_large_font;

extern const char *s_normal_fonts[];
extern const char *s_small_fonts[];
extern const char *s_large_fonts[];

XFontStruct *h_get_font(Display *x_display, const char** s_fonts);
