
/*  Copyright (C) 2016, Inria
    Author: Dumitru Potop-Butucaru.

    This file is part of RPi653. 

    RPi653 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3 of the License.

    RPi653 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <librpi/debug.h>
#include <librpi/indent.h>

// Depth
#define INDENT_MAX_DEPTH 10
static unsigned char indent_depth = 0 ;

void indent_set_depth(unsigned char depth) {
  if(depth > INDENT_MAX_DEPTH) {
    // Debug message and set it to 0
    debug_printf("indent_set_depth: error: required depth %d "
		 "> max depth %d. Reset to 0.\n",
		 depth,
		 INDENT_MAX_DEPTH) ;
    indent_depth = 0 ;
  }
  indent_depth = depth ;
}
void indent_increment_depth() {
  if(indent_depth < INDENT_MAX_DEPTH) {
    indent_depth ++ ;
  }
}
void indent_decrement_depth() {
  if(indent_depth > 0) {
    indent_depth -- ;
  }
}
unsigned char indent_get_depth() {
  return indent_depth ;
}
