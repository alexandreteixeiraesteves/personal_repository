
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
#ifndef INDENT_H
#define INDENT_H

// Pretty indentation working together with the debug
// printing routines. 
void indent_set_depth(unsigned char depth) ;
// These 2 functions should only be called between printed
// lines (after a \n), not in the middle of a line.
void indent_increment_depth(void) ;
void indent_decrement_depth(void) ;
unsigned char indent_get_depth(void) ;

#endif
