#include <stdio.h>

#include <notcurses/notcurses.h>

#include "core/test.h"


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


int main(int argc, char const *argv[]) {
  fprintf(stderr, "filename: %s\n", argv[0]);

  test_func(2048);

  /* start notcurses */
  notcurses_options opts = {
    .termtype = NULL,
    .loglevel = NCLOGLEVEL_SILENT,
    .margin_l = 0, .margin_r = 0,
    .margin_t = 0, .margin_b = 0,
    .flags = 0 // NCOPTION_SUPPRESS_BANNERS // to suppress info at end
  };
  struct notcurses *nc = notcurses_init(&opts, stdout);

  /* check if images can be loaded */
  int canopen_images = notcurses_canopen_images(nc);
  ncblitter_e blitter = get_blitter(nc);
  ncscale_e scale = get_scale(blitter);

  /* set up the screen */
  int err = notcurses_enter_alternate_screen(nc);
  if (err) {
    fprintf(stderr, "error entering alternate screen\n");
    return 1;
  }

  /* set up the standard plane and get its dimensions */
  struct ncplane *pln_std = notcurses_stdplane(nc);
  unsigned int rows, cols;
  ncplane_dim_yx(pln_std, &rows, &cols);

  /* render the screen */
  notcurses_render(nc);

  // TODO: play the game

  // TODO: destroy any created planes

  notcurses_leave_alternate_screen(nc);
  notcurses_stop(nc);

  return 0;
}
