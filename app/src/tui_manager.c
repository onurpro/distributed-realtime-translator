#include "tui_manager.h"
#include <ncurses.h>
#include <string.h>
#include <stdarg.h>

#define HEIGHT_HEADER 3
#define HEIGHT_PANEL 6
#define MAX_LOG_LINES 100  // Store up to 100 lines in memory
#define LOG_DISPLAY_H 18   // How many lines fit visually

static int screen_w, screen_h;
static int col_split;      // X coordinate where the screen splits

// Circular Buffer for Logs
static char log_history[MAX_LOG_LINES][128];
static int log_head = 0;
static int log_count = 0;

// Helper to draw boxes
void draw_box(int y, int x, int h, int w, const char* title) {
    // Clear background
    for(int i=1; i<h-1; i++) {
        mvhline(y+i, x+1, ' ', w-2);
    }
    
    // Draw Border
    mvhline(y, x, 0, w);
    mvhline(y + h - 1, x, 0, w);
    mvvline(y, x, 0, h);
    mvvline(y, x + w - 1, 0, h);
    
    // Corners
    mvaddch(y, x, ACS_ULCORNER);
    mvaddch(y, x + w - 1, ACS_URCORNER);
    mvaddch(y + h - 1, x, ACS_LLCORNER);
    mvaddch(y + h - 1, x + w - 1, ACS_LRCORNER);

    // Title
    attron(A_BOLD | COLOR_PAIR(1));
    mvprintw(y, x + 2, " %s ", title);
    attroff(A_BOLD | COLOR_PAIR(1));
}

void render_logs() {
    int x = col_split + 1;
    int y_start = 4; 
    int w = screen_w - col_split - 2;
    
    // Clear log area
    for(int i=0; i<LOG_DISPLAY_H; i++) {
        mvhline(y_start + i, x, ' ', w);
    }

    // Draw recent logs
    // We work backwards from the newest log
    int drawn = 0;
    int idx = (log_head - 1 + MAX_LOG_LINES) % MAX_LOG_LINES;
    
    while(drawn < log_count && drawn < LOG_DISPLAY_H) {
        mvprintw(y_start + LOG_DISPLAY_H - 1 - drawn, x, "%.*s", w, log_history[idx]);
        
        idx = (idx - 1 + MAX_LOG_LINES) % MAX_LOG_LINES;
        drawn++;
    }
}

void TUI_log(const char* fmt, ...) {
    char buffer[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    // Add to circular buffer
    strncpy(log_history[log_head], buffer, 128);
    log_head = (log_head + 1) % MAX_LOG_LINES;
    if (log_count < MAX_LOG_LINES) log_count++;

    render_logs();
    refresh();
}

void TUI_init(void) {
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    getmaxyx(stdscr, screen_h, screen_w);
    start_color();
    
    col_split = screen_w / 2; // Split screen in half

    init_pair(1, COLOR_CYAN, COLOR_BLACK);   // Borders
    init_pair(2, COLOR_GREEN, COLOR_BLACK);  // Output
    init_pair(3, COLOR_YELLOW, COLOR_BLACK); // Labels
    init_pair(4, COLOR_WHITE, COLOR_BLUE);   // Header
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK);// Network Logs

    // Header
    attron(COLOR_PAIR(4));
    for(int y=0; y<HEIGHT_HEADER; y++) mvhline(y, 0, ' ', screen_w);
    mvprintw(1, screen_w/2 - 15, "ENSC 351: DISTRIBUTED HUB");
    attroff(COLOR_PAIR(4));

    // Draw Static Boxes
    draw_box(3, 0, HEIGHT_PANEL, col_split, " BOARD 1: THE EAR ");
    draw_box(3 + HEIGHT_PANEL, 0, HEIGHT_PANEL, col_split, " BOARD 2: THE BRAIN ");
    draw_box(3 + HEIGHT_PANEL * 2, 0, HEIGHT_PANEL, col_split, " BOARD 3: THE MOUTH ");
    
    // Draw Log Box
    draw_box(3, col_split, HEIGHT_PANEL * 3, screen_w - col_split, " NETWORK CONSOLE ");

    refresh();
}

void TUI_update_ear(const char* status, const char* text) {
    int y = 3;
    int w = col_split - 2;
    
    // Clear content
    mvhline(y+1, 2, ' ', w);
    mvhline(y+3, 2, ' ', w);

    attron(COLOR_PAIR(3));
    mvprintw(y+1, 2, "Status: %s", status);
    attroff(COLOR_PAIR(3));
    mvprintw(y+3, 2, "Input: %.50s...", text);
    refresh();
}

void TUI_update_brain(const char* status, const char* text) {
    int y = 3 + HEIGHT_PANEL;
    int w = col_split - 2;

    mvhline(y+1, 2, ' ', w);
    mvhline(y+3, 2, ' ', w);

    attron(COLOR_PAIR(3));
    mvprintw(y+1, 2, "Status: %s", status);
    attroff(COLOR_PAIR(3));
    
    attron(COLOR_PAIR(2));
    mvprintw(y+3, 2, "French: %.50s...", text);
    attroff(COLOR_PAIR(2));
    refresh();
}

void TUI_update_mouth(const char* status, const char* text) {
    int y = 3 + HEIGHT_PANEL * 2;
    int w = col_split - 2;
    
    mvhline(y+1, 2, ' ', w);
    mvhline(y+3, 2, ' ', w);

    attron(COLOR_PAIR(3));
    mvprintw(y+1, 2, "Status: %s", status);
    attroff(COLOR_PAIR(3));
    
    mvprintw(y+3, 2, "Speaking: %.50s...", text);
    refresh();
}

void TUI_update_progress(float percent) {
    int bar_width = screen_w - 4;
    int filled = (int)(bar_width * percent);
    int y = screen_h - 2;
    
    mvprintw(y-1, 2, "Total Progress: %.1f%%", percent * 100);
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