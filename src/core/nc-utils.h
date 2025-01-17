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


/**
 * Start up the notcurses context.
 *
 * @return The notcurses handle.
 */
struct notcurses *ncu_start(void);


/**
 * Clean up the notcurses context.
 *
 * @param nc The notcurses handle.
 */
void ncu_end(struct notcurses *nc);


/**
 * Draw a line in the outermost cells of a plane.
 *
 * @param ncp The plane to draw on.
 * @param gclusters Six grapheme clusters to use, ul, ur, ll, lr, hl, vl
 *        eg NCBOXLIGHT / "┌┐└┘─│".
 *
 */
void ncutil_perimiter(struct ncplane *ncp, const char *gclusters);


/**
 * Fill a plane with a single character.
 *
 * @param ncp The plane to fill.
 * @param ch The character to fill with.
 */
void ncutil_fill(struct ncplane *ncp, char ch);


/**
 * Create a grid of cells inside a plane.
 *
 * @param ncp The parent plane containing the grid.
 * @param grid The grid of cells to draw.
 * @param rows The number of rows in the grid.
 * @param cols The number of columns in the grid.
 * @param width The width of each grid cell.
 * @param height The height of each grid cell.
 * @return 0 on success, non-zero on failure.
 */
int ncutil_grid(struct ncplane *ncp, struct ncplane **grid, int rows, int cols, int width, int height);


#endif //NC_UTILS_H
