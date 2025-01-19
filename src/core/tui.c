//
// Created by Oscar Holroyd on 19/01/2025.
//

#include "tui.h"


#include <string.h>
#include <locale.h>


void tui_start(void) {
  setlocale(LC_ALL, ""); // set locale to allow wide characters
  initscr(); // start curses mode
  cbreak(); // disable line buffering
  noecho(); // don't echo input
  curs_set(0); // hide the cursor
}


void tui_end(void) {
  endwin(); // end curses mode
}


int tui_start_color(void) {
  if (!has_colors()) {
    return -1;
  }

  start_color();
  return COLOR_PAIRS;
}


void tui_grid(WINDOW **grid, int rows, int cols, int height, int width, int offsety, int offsetx) {
  /* set all pointers to NULL to start with */
  memset(grid, 0, sizeof(WINDOW *) * rows * cols);

  /* create the grid */
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      grid[i * cols + j] = win_create(height, width, offsety + i * height, offsetx + j * width);
    }
  }
}


WINDOW *win_create(int height, int width, int starty, int startx) {
  WINDOW *win = newwin(height, width, starty, startx);
  wrefresh(win); // TODO: remove this?
  return win;
}


void win_destroy(WINDOW *win) {
  // catch NULL pointers
  if (win) {
    wborder(win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    wrefresh(win);
    delwin(win);
  }
}


void win_border(WINDOW *win, BoxStyle style, const attr_t attrs) {
  // set the border characters
  const wchar_t *ctl, *ctr, *cbl, *cbr, *ct, *cb, *cl, *cr;
  switch (style) {
    case BOXROUND:
      ctl = L"╭";
      ctr = L"╮";
      cbl = L"╰";
      cbr = L"╯";
      ct = L"─";
      cb = L"─";
      cl = L"│";
      cr = L"│";
      break;
    case BOXHEAVY:
      ctl = L"┏";
      ctr = L"┓";
      cbl = L"┗";
      cbr = L"┛";
      ct = L"━";
      cb = L"━";
      cl = L"┃";
      cr = L"┃";
      break;
    case BOXLIGHT:
    default:
      ctl = L"┌";
      ctr = L"┐";
      cbl = L"└";
      cbr = L"┘";
      ct = L"─";
      cb = L"─";
      cl = L"│";
      cr = L"│";
  }

  // transfer the characters to the cchar_t structs
  cchar_t tl, tr, bl, br, t, b, l, r;
  setcchar(&tl, ctl, attrs, 0, NULL);
  setcchar(&tr, ctr, attrs, 0, NULL);
  setcchar(&bl, cbl, attrs, 0, NULL);
  setcchar(&br, cbr, attrs, 0, NULL);
  setcchar(&t, ct, attrs, 0, NULL);
  setcchar(&b, cb, attrs, 0, NULL);
  setcchar(&l, cl, attrs, 0, NULL);
  setcchar(&r, cr, attrs, 0, NULL);

  // draw the border
  wborder_set(win, &l, &r, &t, &b, &tl, &tr, &bl, &br);
}


void init_hex_color(short index, uint32_t hex) {
  /* convert hex to 1000 */
  short r = (hex >> 16) * 1000 / 255;
  short g = ((hex >> 8) & 0xFF) * 1000 / 255;
  short b = (hex & 0xFF) * 1000 / 255;

  init_color(index, r, g, b);
}


const char *nc_keystr(int key) {
  switch (key) {
    case KEY_DOWN: return "KEY_UP"; /* down-arrow key */
    case KEY_UP: return "KEY_UP"; /* up-arrow key */
    case KEY_LEFT: return "KEY_LEFT"; /* left-arrow key */
    case KEY_RIGHT: return "KEY_RIGHT"; /* right-arrow key */
    case KEY_HOME: return "KEY_HOME"; /* home key */
    case KEY_BACKSPACE: return "KEY_BACKSPACE"; /* backspace key */
    case KEY_F0: return "KEY_F0"; /* Function keys.  Space for 64 */
    case KEY_DL: return "KEY_DL"; /* delete-line key */
    case KEY_IL: return "KEY_IL"; /* insert-line key */
    case KEY_DC: return "KEY_DC"; /* delete-character key */
    case KEY_IC: return "KEY_IC"; /* insert-character key */
    case KEY_EIC: return "KEY_EIC"; /* sent by rmir or smir in insert mode */
    case KEY_CLEAR: return "KEY_CLEAR"; /* clear-screen or erase key */
    case KEY_EOS: return "KEY_EOS"; /* clear-to-end-of-screen key */
    case KEY_EOL: return "KEY_EOL"; /* clear-to-end-of-line key */
    case KEY_SF: return "KEY_SF"; /* scroll-forward key */
    case KEY_SR: return "KEY_SR"; /* scroll-backward key */
    case KEY_NPAGE: return "KEY_NPAGE"; /* next-page key */
    case KEY_PPAGE: return "KEY_PPAGE"; /* previous-page key */
    case KEY_STAB: return "KEY_STAB"; /* set-tab key */
    case KEY_CTAB: return "KEY_CTAB"; /* clear-tab key */
    case KEY_CATAB: return "KEY_CATAB"; /* clear-all-tabs key */
    case KEY_ENTER: return "KEY_ENTER"; /* enter/send key */
    case KEY_PRINT: return "KEY_PRINT"; /* print key */
    case KEY_LL: return "KEY_LL"; /* lower-left key (home down) */
    case KEY_A1: return "KEY_A1"; /* upper left of keypad */
    case KEY_A3: return "KEY_A3"; /* upper right of keypad */
    case KEY_B2: return "KEY_B2"; /* center of keypad */
    case KEY_C1: return "KEY_C1"; /* lower left of keypad */
    case KEY_C3: return "KEY_C3"; /* lower right of keypad */
    case KEY_BTAB: return "KEY_BTAB"; /* back-tab key */
    case KEY_BEG: return "KEY_BEG"; /* begin key */
    case KEY_CANCEL: return "KEY_CANCEL"; /* cancel key */
    case KEY_CLOSE: return "KEY_CLOSE"; /* close key */
    case KEY_COMMAND: return "KEY_COMMAND"; /* command key */
    case KEY_COPY: return "KEY_COPY"; /* copy key */
    case KEY_CREATE: return "KEY_CREATE"; /* create key */
    case KEY_END: return "KEY_END"; /* end key */
    case KEY_EXIT: return "KEY_EXIT"; /* exit key */
    case KEY_FIND: return "KEY_FIND"; /* find key */
    case KEY_HELP: return "KEY_HELP"; /* help key */
    case KEY_MARK: return "KEY_MARK"; /* mark key */
    case KEY_MESSAGE: return "KEY_MESSAGE"; /* message key */
    case KEY_MOVE: return "KEY_MOVE"; /* move key */
    case KEY_NEXT: return "KEY_NEXT"; /* next key */
    case KEY_OPEN: return "KEY_OPEN"; /* open key */
    case KEY_OPTIONS: return "KEY_OPTIONS"; /* options key */
    case KEY_PREVIOUS: return "KEY_PREVIOUS"; /* previous key */
    case KEY_REDO: return "KEY_REDO"; /* redo key */
    case KEY_REFERENCE: return "KEY_REFERENCE"; /* reference key */
    case KEY_REFRESH: return "KEY_REFRESH"; /* refresh key */
    case KEY_REPLACE: return "KEY_REPLACE"; /* replace key */
    case KEY_RESTART: return "KEY_RESTART"; /* restart key */
    case KEY_RESUME: return "KEY_RESUME"; /* resume key */
    case KEY_SAVE: return "KEY_SAVE"; /* save key */
    case KEY_SBEG: return "KEY_SBEG"; /* shifted begin key */
    case KEY_SCANCEL: return "KEY_SCANCEL"; /* shifted cancel key */
    case KEY_SCOMMAND: return "KEY_SCOMMAND"; /* shifted command key */
    case KEY_SCOPY: return "KEY_SCOPY"; /* shifted copy key */
    case KEY_SCREATE: return "KEY_SCREATE"; /* shifted create key */
    case KEY_SDC: return "KEY_SDC"; /* shifted delete-character key */
    case KEY_SDL: return "KEY_SDL"; /* shifted delete-line key */
    case KEY_SELECT: return "KEY_SELECT"; /* select key */
    case KEY_SEND: return "KEY_SEND"; /* shifted end key */
    case KEY_SEOL: return "KEY_SEOL"; /* shifted clear-to-end-of-line key */
    case KEY_SEXIT: return "KEY_SEXIT"; /* shifted exit key */
    case KEY_SFIND: return "KEY_SFIND"; /* shifted find key */
    case KEY_SHELP: return "KEY_SHELP"; /* shifted help key */
    case KEY_SHOME: return "KEY_SHOME"; /* shifted home key */
    case KEY_SIC: return "KEY_SIC"; /* shifted insert-character key */
    case KEY_SLEFT: return "KEY_SLEFT"; /* shifted left-arrow key */
    case KEY_SMESSAGE: return "KEY_SMESSAGE"; /* shifted message key */
    case KEY_SMOVE: return "KEY_SMOVE"; /* shifted move key */
    case KEY_SNEXT: return "KEY_SNEXT"; /* shifted next key */
    case KEY_SOPTIONS: return "KEY_SOPTIONS"; /* shifted options key */
    case KEY_SPREVIOUS: return "KEY_SPREVIOUS"; /* shifted previous key */
    case KEY_SPRINT: return "KEY_SPRINT"; /* shifted print key */
    case KEY_SREDO: return "KEY_SREDO"; /* shifted redo key */
    case KEY_SREPLACE: return "KEY_SREPLACE"; /* shifted replace key */
    case KEY_SRIGHT: return "KEY_SRIGHT"; /* shifted right-arrow key */
    case KEY_SRSUME: return "KEY_SRSUME"; /* shifted resume key */
    case KEY_SSAVE: return "KEY_SSAVE"; /* shifted save key */
    case KEY_SSUSPEND: return "KEY_SSUSPEND"; /* shifted suspend key */
    case KEY_SUNDO: return "KEY_SUNDO"; /* shifted undo key */
    case KEY_SUSPEND: return "KEY_SUSPEND"; /* suspend key */
    case KEY_UNDO: return "KEY_UNDO"; /* undo key */
    case KEY_MOUSE: return "KEY_MOUSE"; /* Mouse event has occurred */
    case KEY_RESIZE: return "KEY_RESIZE"; /* Terminal resize event */
    case KEY_EVENT: return "KEY_EVENT"; /* We were interrupted by an event */
    default: return "unknown";
  }
}
