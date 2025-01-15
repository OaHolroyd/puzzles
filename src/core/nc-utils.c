//
// Created by Oscar Holroyd on 15/01/2025.
//

#include "nc-utils.h"


/**
 * Get the best blitter available for the terminal.
 *
 * @param nc The notcurses context.
 * @return The best blitter.
 */
ncblitter_e get_blitter(struct notcurses *nc) {
  if (notcurses_canpixel(nc)) {
    // pixels are the best option
    return NCBLIT_PIXEL;
  }

  return NCBLIT_DEFAULT;
}


/**
 * Get the correct scaling option for a given blitter.
 *
 * @param blitter The blitter to use.
 * @return The best scaling option.
 */
ncscale_e get_scale(ncblitter_e blitter) {
  if (blitter == NCBLIT_PIXEL) {
    return NCSCALE_SCALE_HIRES;
  }

  return NCSCALE_NONE;
}
