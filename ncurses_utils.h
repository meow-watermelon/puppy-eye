#ifndef NCURSES_UTILS_H
#define NCURSES_UTILS_H

#include <ncurses.h>

extern void construct_window_layout(WINDOW *input_window);
extern void print_network_interface_delimiter(WINDOW *input_window, int row_number);

#endif /* NCURSES_UTILS_H */
