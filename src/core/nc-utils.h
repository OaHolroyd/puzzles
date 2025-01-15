//
// Created by Oscar Holroyd on 15/01/2025.
//

#ifndef NC_UTILS_H
#define NC_UTILS_H

#include <notcurses/notcurses.h>


/**
 * Get the best blitter available for the terminal.
 *
 * @param nc The notcurses context.
 * @return The best blitter.
 */
ncblitter_e get_blitter(struct notcurses *nc);


/**
 * Get the correct scaling option for a given blitter.
 *
 * @param blitter The blitter to use.
 * @return The best scaling option.
 */
ncscale_e get_scale(ncblitter_e blitter);


#endif //NC_UTILS_H
