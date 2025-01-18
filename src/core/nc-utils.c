//
// Created by Oscar Holroyd on 15/01/2025.
//

#include "nc-utils.h"

#include "logging.h"


ncblitter_e get_blitter(const struct notcurses *nc) {
  if (notcurses_canpixel(nc)) {
    // pixels are the best option
    return NCBLIT_PIXEL;
  }

  return NCBLIT_DEFAULT;
}


ncscale_e get_scale(const ncblitter_e blitter) {
  if (blitter == NCBLIT_PIXEL) {
    return NCSCALE_SCALE_HIRES;
  }

  return NCSCALE_NONE;
}


struct notcurses *ncutil_start(void) {
  const notcurses_options opts = {
    .termtype = NULL,
    .loglevel = NCLOGLEVEL_SILENT,
    .margin_l = 0, .margin_r = 0,
    .margin_t = 0, .margin_b = 0,
    .flags = 0 // NCOPTION_SUPPRESS_BANNERS // to suppress info at end
  };
  struct notcurses *nc = notcurses_init(&opts, stdout);
  LOG("INFO: started notcurses");
  if (!nc) {
    LOG("ERROR: failed to start notcurses");
    return NULL;
  }

  /* set up the screen */
  if (notcurses_enter_alternate_screen(nc)) {
    LOG("ERROR: failed to enter alternate screen");
    notcurses_stop(nc);
    return NULL;
  }

  return nc;
}


void ncutil_end(struct notcurses *nc) {
  notcurses_leave_alternate_screen(nc);
  notcurses_stop(nc);
}


void ncutil_perimiter(struct ncplane *ncp, const char *gclusters) {
  /* define color/style etc */
  uint32_t channels = 0;
  ncchannel_set_rgb8(&channels, 0x00, 0x00, 0x00);
  const uint16_t attr = NCSTYLE_NONE;

  /* define cells */
  nccell ul = NCCELL_TRIVIAL_INITIALIZER, ur = NCCELL_TRIVIAL_INITIALIZER;
  nccell ll = NCCELL_TRIVIAL_INITIALIZER, lr = NCCELL_TRIVIAL_INITIALIZER;
  nccell hl = NCCELL_TRIVIAL_INITIALIZER, vl = NCCELL_TRIVIAL_INITIALIZER;

  const int err = nccells_load_box(ncp, attr, channels, &ul, &ur, &ll, &lr, &hl, &vl, gclusters);
  if (err) {
    // fall back to ASCII
    nccells_ascii_box(ncp, attr, channels, &ul, &ur, &ll, &lr, &hl, &vl);
  }

  /* draw perimeter */
  ncplane_perimeter(ncp, &ul, &ur, &ll, &lr, &hl, &vl, 0);

  /* drop cells */
  nccell_release(ncp, &ul);
  nccell_release(ncp, &ur);
  nccell_release(ncp, &ll);
  nccell_release(ncp, &lr);
  nccell_release(ncp, &hl);
  nccell_release(ncp, &vl);
}


void ncutil_fill(struct ncplane *ncp, const char ch) {
  unsigned int rows, cols;
  ncplane_dim_yx(ncp, &rows, &cols);

  nccell ce = NCCELL_INITIALIZER((uint32_t)ch, ncplane_styles(ncp), ncplane_channels(ncp));

  for (unsigned y = 0; y < rows; y++) {
    for (unsigned x = 0; x < cols; x++) {
      ncplane_putc_yx(ncp, y, x, &ce);
    }
  }

  nccell_release(ncp, &ce);
}


int ncutil_grid(
  struct ncplane *ncp, struct ncplane **grid,
  const int rows, const int cols, const int width, const int height
) {
  /* check that the parent plane is large enough */
  unsigned int p_rows, p_cols;
  ncplane_dim_yx(ncp, &p_rows, &p_cols);
  if (p_rows < rows * height || p_cols < cols * width) {
    LOG("ERROR: parent plane is too small for grid\n");
    return 1;
  }

  /* set all pointers to NULL to start with */
  memset(grid, 0, sizeof(struct ncplane *) * rows * cols);

  /* create the grid */
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      ncplane_options opts = {
        .y = i * height,
        .x = j * width,
        .rows = height,
        .cols = width,
      };
      grid[i * cols + j] = ncplane_create(ncp, &opts);

      if (!grid[i * cols + j]) {
        LOG("ERROR: failed to create grid cell %d %d\n", i, j);

        /* clean up any cells that have been created */
        for (int ii = 0; ii < rows; ii++) {
          for (int jj = 0; jj < cols; jj++) {
            if (grid[ii * cols + jj]) {
              ncplane_destroy(grid[ii * cols + jj]);
              grid[ii * cols + jj] = NULL;
            }
          }
        }
        return 1;
      }
    }
  }

  return 0;
}


const char *ncu_keystr(const uint32_t key) {
  switch (key) {
    case NCKEY_RESIZE: return "resize event";
    case NCKEY_INVALID: return "invalid";

    case NCKEY_ESC: return "escape";
    case NCKEY_TAB: return "tab";
    case NCKEY_LEFT: return "left";
    case NCKEY_UP: return "up";
    case NCKEY_RIGHT: return "right";
    case NCKEY_DOWN: return "down";
    case NCKEY_INS: return "insert";
    case NCKEY_DEL: return "delete";
    case NCKEY_PGDOWN: return "pgdown";
    case NCKEY_PGUP: return "pgup";
    case NCKEY_HOME: return "home";
    case NCKEY_END: return "end";
    case NCKEY_F00: return "F0";
    case NCKEY_F01: return "F1";
    case NCKEY_F02: return "F2";
    case NCKEY_F03: return "F3";
    case NCKEY_F04: return "F4";
    case NCKEY_F05: return "F5";
    case NCKEY_F06: return "F6";
    case NCKEY_F07: return "F7";
    case NCKEY_F08: return "F8";
    case NCKEY_F09: return "F9";
    case NCKEY_F10: return "F10";
    case NCKEY_F11: return "F11";
    case NCKEY_F12: return "F12";
    case NCKEY_F13: return "F13";
    case NCKEY_F14: return "F14";
    case NCKEY_F15: return "F15";
    case NCKEY_F16: return "F16";
    case NCKEY_F17: return "F17";
    case NCKEY_F18: return "F18";
    case NCKEY_F19: return "F19";
    case NCKEY_F20: return "F20";
    case NCKEY_F21: return "F21";
    case NCKEY_F22: return "F22";
    case NCKEY_F23: return "F23";
    case NCKEY_F24: return "F24";
    case NCKEY_F25: return "F25";
    case NCKEY_F26: return "F26";
    case NCKEY_F27: return "F27";
    case NCKEY_F28: return "F28";
    case NCKEY_F29: return "F29";
    case NCKEY_F30: return "F30";
    case NCKEY_F31: return "F31";
    case NCKEY_F32: return "F32";
    case NCKEY_F33: return "F33";
    case NCKEY_F34: return "F34";
    case NCKEY_F35: return "F35";
    case NCKEY_F36: return "F36";
    case NCKEY_F37: return "F37";
    case NCKEY_F38: return "F38";
    case NCKEY_F39: return "F39";
    case NCKEY_F40: return "F40";
    case NCKEY_F41: return "F41";
    case NCKEY_F42: return "F42";
    case NCKEY_F43: return "F43";
    case NCKEY_F44: return "F44";
    case NCKEY_F45: return "F45";
    case NCKEY_F46: return "F46";
    case NCKEY_F47: return "F47";
    case NCKEY_F48: return "F48";
    case NCKEY_F49: return "F49";
    case NCKEY_F50: return "F50";
    case NCKEY_F51: return "F51";
    case NCKEY_F52: return "F52";
    case NCKEY_F53: return "F53";
    case NCKEY_F54: return "F54";
    case NCKEY_F55: return "F55";
    case NCKEY_F56: return "F56";
    case NCKEY_F57: return "F57";
    case NCKEY_F58: return "F58";
    case NCKEY_F59: return "F59";
    case NCKEY_BACKSPACE: return "backspace";
    case NCKEY_CENTER: return "center";
    case NCKEY_ENTER: return "enter";
    case NCKEY_CLS: return "clear";
    case NCKEY_DLEFT: return "down+left";
    case NCKEY_DRIGHT: return "down+right";
    case NCKEY_ULEFT: return "up+left";
    case NCKEY_URIGHT: return "up+right";
    case NCKEY_BEGIN: return "begin";
    case NCKEY_CANCEL: return "cancel";
    case NCKEY_CLOSE: return "close";
    case NCKEY_COMMAND: return "command";
    case NCKEY_COPY: return "copy";
    case NCKEY_EXIT: return "exit";
    case NCKEY_PRINT: return "print";
    case NCKEY_REFRESH: return "refresh";
    case NCKEY_SEPARATOR: return "separator";
    case NCKEY_CAPS_LOCK: return "caps lock";
    case NCKEY_SCROLL_LOCK: return "scroll lock";
    case NCKEY_NUM_LOCK: return "num lock";
    case NCKEY_PRINT_SCREEN: return "print screen";
    case NCKEY_PAUSE: return "pause";
    case NCKEY_MENU: return "menu";
    // media keys, similarly only available through kitty's protocol
    case NCKEY_MEDIA_PLAY: return "play";
    case NCKEY_MEDIA_PAUSE: return "pause";
    case NCKEY_MEDIA_PPAUSE: return "play-pause";
    case NCKEY_MEDIA_REV: return "reverse";
    case NCKEY_MEDIA_STOP: return "stop";
    case NCKEY_MEDIA_FF: return "fast-forward";
    case NCKEY_MEDIA_REWIND: return "rewind";
    case NCKEY_MEDIA_NEXT: return "next track";
    case NCKEY_MEDIA_PREV: return "previous track";
    case NCKEY_MEDIA_RECORD: return "record";
    case NCKEY_MEDIA_LVOL: return "lower volume";
    case NCKEY_MEDIA_RVOL: return "raise volume";
    case NCKEY_MEDIA_MUTE: return "mute";
    case NCKEY_LSHIFT: return "left shift";
    case NCKEY_LCTRL: return "left ctrl";
    case NCKEY_LALT: return "left alt";
    case NCKEY_LSUPER: return "left super";
    case NCKEY_LHYPER: return "left hyper";
    case NCKEY_LMETA: return "left meta";
    case NCKEY_RSHIFT: return "right shift";
    case NCKEY_RCTRL: return "right ctrl";
    case NCKEY_RALT: return "right alt";
    case NCKEY_RSUPER: return "right super";
    case NCKEY_RHYPER: return "right hyper";
    case NCKEY_RMETA: return "right meta";
    case NCKEY_L3SHIFT: return "level 3 shift";
    case NCKEY_L5SHIFT: return "level 5 shift";
    case NCKEY_MOTION: return "mouse (no buttons pressed)";
    case NCKEY_BUTTON1: return "mouse (button 1)";
    case NCKEY_BUTTON2: return "mouse (button 2)";
    case NCKEY_BUTTON3: return "mouse (button 3)";
    case NCKEY_BUTTON4: return "mouse (button 4)";
    case NCKEY_BUTTON5: return "mouse (button 5)";
    case NCKEY_BUTTON6: return "mouse (button 6)";
    case NCKEY_BUTTON7: return "mouse (button 7)";
    case NCKEY_BUTTON8: return "mouse (button 8)";
    case NCKEY_BUTTON9: return "mouse (button 9)";
    case NCKEY_BUTTON10: return "mouse (button 10)";
    case NCKEY_BUTTON11: return "mouse (button 11)";
    default: return "unknown";
  }
}
