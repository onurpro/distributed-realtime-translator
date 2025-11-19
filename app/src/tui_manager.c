#include "tui_manager.h"
#include <ncurses.h>
#include <string.h>

#define HEIGHT_HEADER 3
#define HEIGHT_PANEL 6

static int screen_w, screen_h;

void draw_box_title(int y, int h, const char* title) {
    mvhline(y, 0, 0, screen_w);
    mvhline(y + h, 0, 0, screen_w);
    mvvline(y, 0, 0, h);
    mvvline(y, screen_w - 1, 0, h);
    mvaddch(y, 0, ACS_ULCORNER);
    mvaddch(y, screen_w - 1, ACS_URCORNER);
    mvaddch(y + h, 0, ACS_LLCORNER);
    mvaddch(y + h, screen_w - 1, ACS_LRCORNER);
    
    attron(A_BOLD | COLOR_PAIR(1));
    mvprintw(y, 2, " %s ", title);
    attroff(A_BOLD | COLOR_PAIR(1));
}

void TUI_init(void) {
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    getmaxyx(stdscr, screen_h, screen_w);
    start_color();

    init_pair(1, COLOR_CYAN, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_WHITE, COLOR_BLUE);

    attron(COLOR_PAIR(4));
    for(int y=0; y<HEIGHT_HEADER; y++) mvhline(y, 0, ' ', screen_w);
    mvprintw(1, screen_w/2 - 15, "ENSC 351: DISTRIBUTED TRANSLATOR");
    attroff(COLOR_PAIR(4));

    draw_box_title(3, HEIGHT_PANEL, " BOARD 1: THE EAR ");
    draw_box_title(3 + HEIGHT_PANEL, HEIGHT_PANEL, " BOARD 2: THE BRAIN ");
    draw_box_title(3 + HEIGHT_PANEL * 2, HEIGHT_PANEL, " BOARD 3: THE MOUTH ");
    refresh();
}

void TUI_update_ear(const char* status, const char* text) {
    int y = 3;
    for(int i=1; i<HEIGHT_PANEL-1; i++) mvhline(y+i, 1, ' ', screen_w-2);
    attron(COLOR_PAIR(3));
    mvprintw(y+1, 2, "Status: %s", status);
    attroff(COLOR_PAIR(3));
    mvprintw(y+3, 2, "Input: %.60s...", text);
    refresh();
}

void TUI_update_brain(const char* status, const char* text) {
    int y = 3 + HEIGHT_PANEL;
    for(int i=1; i<HEIGHT_PANEL-1; i++) mvhline(y+i, 1, ' ', screen_w-2);
    attron(COLOR_PAIR(3));
    mvprintw(y+1, 2, "Status: %s", status);
    attroff(COLOR_PAIR(3));
    attron(COLOR_PAIR(2));
    mvprintw(y+3, 2, "French: %.60s...", text);
    attroff(COLOR_PAIR(2));
    refresh();
}

void TUI_update_mouth(const char* status, const char* text) {
    int y = 3 + HEIGHT_PANEL * 2;
    for(int i=1; i<HEIGHT_PANEL-1; i++) mvhline(y+i, 1, ' ', screen_w-2);
    attron(COLOR_PAIR(3));
    mvprintw(y+1, 2, "Status: %s", status);
    attroff(COLOR_PAIR(3));
    mvprintw(y+3, 2, "Speaking: %.60s...", text);
    refresh();
}

void TUI_update_progress(float percent) {
    int bar_width = screen_w - 4;
    int filled = (int)(bar_width * percent);
    int y = screen_h - 2;
    mvprintw(y-1, 2, "Progress: %.1f%%", percent * 100);
    mvaddch(y, 1, '[');
    for(int i=0; i<bar_width; i++) {
        if (i < filled) attron(A_REVERSE);
        addch(' ');
        if (i < filled) attroff(A_REVERSE);
    }
    addch(']');
    refresh();
}

void TUI_close(void) {
    endwin();
}