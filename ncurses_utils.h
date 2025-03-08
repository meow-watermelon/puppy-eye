#ifndef NCURSES_UTILS_H
#define NCURSES_UTILS_H

#include <ncurses.h>

extern void construct_window_layout(WINDOW *input_window, int interface_count);
extern void print_delimiter(WINDOW *input_window, int row_number, int *column_positions, size_t column_size);

#endif /* NCURSES_UTILS_H */
