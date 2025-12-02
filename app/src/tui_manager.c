#include "tui_manager.h"
#include <ncurses.h>
#include <string.h>
#include <stdarg.h>
#include <pthread.h>

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

// State for Redrawing
static int current_mode = 0; // 0=Standard, 1=Presentation
static int anim_frame = 0;

static char ear_status[64] = "Idle";
static char ear_history[MAX_LOG_LINES][256];
static int ear_head = 0;
static int ear_count = 0;

// static char ear_text[256] = ""; // Removed in favor of history
static char brain_status[64] = "Idle";
static char brain_text[256] = "";
static char mouth_status[64] = "Idle";
static char mouth_text[256] = "";

static pthread_mutex_t tui_mutex = PTHREAD_MUTEX_INITIALIZER;

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
    attroff(A_BOLD | COLOR_PAIR(1));
}

// Helper to draw wrapped text
void draw_text_wrapped(int y, int x, int h, int w, const char* text, int color_pair) {
    int cur_y = 0;
    int cur_x = 0;
    int len = strlen(text);
    
    attron(color_pair);
    for (int i = 0; i < len; i++) {
        if (cur_y >= h) break; // Out of space
        
        mvaddch(y + cur_y, x + cur_x, text[i]);
        cur_x++;
        
        if (cur_x >= w) {
            cur_x = 0;
            cur_y++;
        }
    }
    attroff(color_pair);
}

void render_logs() {
    if (current_mode == 1) return; // No logs in presentation mode

    int x = col_split + 1;
    int y_start = 4; 
    int w = screen_w - col_split - 2;
    
    // Clear log area
    for(int i=0; i<LOG_DISPLAY_H; i++) {
        mvhline(y_start + i, x, ' ', w);
    }

    // Draw recent logs
    int drawn = 0;
    int idx = (log_head - 1 + MAX_LOG_LINES) % MAX_LOG_LINES;
    
    while(drawn < log_count && drawn < LOG_DISPLAY_H) {
        mvprintw(y_start + LOG_DISPLAY_H - 1 - drawn, x, "%.*s", w, log_history[idx]);
        
        idx = (idx - 1 + MAX_LOG_LINES) % MAX_LOG_LINES;
        drawn++;
    }
}

// --- Animation Helpers ---
const char* get_wave_anim(int active) {
    if (!active) return "          ";
    static const char* waves[] = {
        ". . . . . ",
        " . . . . .",
        ". . . . . ",
        " . . . . ."
    };
    return waves[anim_frame % 4];
}

// --- Render Modes ---

void render_standard() {
    erase();
    
    // Header
    attron(COLOR_PAIR(4));
    for(int y=0; y<HEIGHT_HEADER; y++) mvhline(y, 0, ' ', screen_w);
    mvprintw(1, screen_w/2 - 15, "ENSC 351: DISTRIBUTED HUB (DEBUG)");
    attroff(COLOR_PAIR(4));

    // Boxes
    draw_box(3, 0, HEIGHT_PANEL, col_split, " BOARD 1: THE EAR ");
    draw_box(3 + HEIGHT_PANEL, 0, HEIGHT_PANEL, col_split, " BOARD 2: THE BRAIN ");
    draw_box(3 + HEIGHT_PANEL * 2, 0, HEIGHT_PANEL, col_split, " BOARD 3: THE MOUTH ");
    draw_box(3, col_split, HEIGHT_PANEL * 3, screen_w - col_split, " NETWORK CONSOLE ");

    // Content - Ear (History)
    attron(COLOR_PAIR(3));
    mvprintw(4, 2, "Status: %s", ear_status);
    attroff(COLOR_PAIR(3));
    
    // Draw Ear History
    int ear_y_start = 6;
    int ear_h = HEIGHT_PANEL - 3; // Available height
    int drawn = 0;
    int idx = (ear_head - 1 + MAX_LOG_LINES) % MAX_LOG_LINES;
    
    while(drawn < ear_count && drawn < ear_h) {
        mvprintw(ear_y_start + drawn, 2, "%.*s", col_split - 4, ear_history[idx]);
        idx = (idx - 1 + MAX_LOG_LINES) % MAX_LOG_LINES;
        drawn++;
    }

    // Content - Brain
    attron(COLOR_PAIR(3));
    mvprintw(4 + HEIGHT_PANEL, 2, "Status: %s", brain_status);
    attroff(COLOR_PAIR(3));
    attron(COLOR_PAIR(2));
    draw_text_wrapped(6 + HEIGHT_PANEL, 2, HEIGHT_PANEL - 3, col_split - 4, brain_text, COLOR_PAIR(2));
    attroff(COLOR_PAIR(2));

    // Content - Mouth
    attron(COLOR_PAIR(3));
    mvprintw(4 + HEIGHT_PANEL * 2, 2, "Status: %s", mouth_status);
    attroff(COLOR_PAIR(3));
    draw_text_wrapped(6 + HEIGHT_PANEL * 2, 2, HEIGHT_PANEL - 3, col_split - 4, mouth_text, 0);

    render_logs();
}

void render_presentation() {
    erase();
    
    // Header
    attron(COLOR_PAIR(4) | A_BOLD);
    for(int y=0; y<HEIGHT_HEADER; y++) mvhline(y, 0, ' ', screen_w);
    mvprintw(1, screen_w/2 - 20, "   ENSC 351: LIVE TRANSLATION DEMO   ");
    attroff(COLOR_PAIR(4) | A_BOLD);

    int box_w = screen_w / 3 - 2;
    int box_h = screen_h - 6;
    int y_start = 4;

    // 1. EAR
    draw_box(y_start, 1, box_h, box_w, " THE EAR ");
    attron(A_BOLD | COLOR_PAIR(3));
    mvprintw(y_start + 2, 3, "%s", ear_status);
    attroff(A_BOLD | COLOR_PAIR(3));
    // Draw Ear History (Presentation)
    int ear_h_avail = box_h - 6;
    int count_to_show = (ear_count < ear_h_avail) ? ear_count : ear_h_avail;
    
    // Start from the oldest visible item
    // ear_head points to the next empty slot. 
    // The newest item is at (ear_head - 1).
    // The oldest item is at (ear_head - ear_count).
    // We want to start at (ear_head - count_to_show).
    int start_idx = (ear_head - count_to_show + MAX_LOG_LINES) % MAX_LOG_LINES;
    
    for(int i=0; i<count_to_show; i++) {
        int idx = (start_idx + i) % MAX_LOG_LINES;
        mvprintw(y_start + 4 + i, 3, "%.*s", box_w - 4, ear_history[idx]);
    }
    if (strstr(ear_status, "Listening") || strstr(ear_status, "Streaming")) {
        attron(COLOR_PAIR(2));
        mvprintw(y_start + 6, 3, "Active: [%s]", get_wave_anim(1));
        attroff(COLOR_PAIR(2));
    }

    // Arrow 1
    mvprintw(y_start + box_h/2, box_w + 1, "-->");

    // 2. BRAIN
    draw_box(y_start, box_w + 5, box_h, box_w, " THE BRAIN ");
    attron(A_BOLD | COLOR_PAIR(3));
    mvprintw(y_start + 2, box_w + 7, "%s", brain_status);
    attroff(A_BOLD | COLOR_PAIR(3));
    attron(COLOR_PAIR(2) | A_BOLD);
    draw_text_wrapped(y_start + 4, box_w + 7, box_h - 6, box_w - 4, brain_text, COLOR_PAIR(2) | A_BOLD);
    attroff(COLOR_PAIR(2) | A_BOLD);

    // Arrow 2
    mvprintw(y_start + box_h/2, (box_w * 2) + 5, "-->");

    // 3. MOUTH
    draw_box(y_start, (box_w * 2) + 9, box_h, box_w, " THE MOUTH ");
    attron(A_BOLD | COLOR_PAIR(3));
    mvprintw(y_start + 2, (box_w * 2) + 11, "%s", mouth_status);
    attroff(A_BOLD | COLOR_PAIR(3));
    draw_text_wrapped(y_start + 4, (box_w * 2) + 11, box_h - 6, box_w - 4, mouth_text, 0);
    if (strstr(mouth_status, "Speaking")) {
        attron(COLOR_PAIR(2));
        mvprintw(y_start + 6, (box_w * 2) + 11, "Audio: [%s]", get_wave_anim(1));
        attroff(COLOR_PAIR(2));
    }
}

void render_all() {
    if (current_mode == 0) render_standard();
    else render_presentation();
    refresh();
}

void TUI_log(const char* fmt, ...) {
    pthread_mutex_lock(&tui_mutex);
    char buffer[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    // Add to circular buffer
    strncpy(log_history[log_head], buffer, 128);
    log_head = (log_head + 1) % MAX_LOG_LINES;
    if (log_count < MAX_LOG_LINES) log_count++;

    render_all();
    pthread_mutex_unlock(&tui_mutex);
}

void TUI_init(void) {
    pthread_mutex_lock(&tui_mutex);
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

    render_all();
    pthread_mutex_unlock(&tui_mutex);
}

void TUI_update_ear(const char* status, const char* text) {
    pthread_mutex_lock(&tui_mutex);
    strncpy(ear_status, status, 63);
    
    // Add to history if text is not empty and different from last (simple dedup)
    if (strlen(text) > 0) {
        strncpy(ear_history[ear_head], text, 255);
        ear_head = (ear_head + 1) % MAX_LOG_LINES;
        if (ear_count < MAX_LOG_LINES) ear_count++;
    }
    
    render_all();
    pthread_mutex_unlock(&tui_mutex);
}

void TUI_update_brain(const char* status, const char* text) {
    pthread_mutex_lock(&tui_mutex);
    strncpy(brain_status, status, 63);
    strncpy(brain_text, text, 255);
    render_all();
    pthread_mutex_unlock(&tui_mutex);
}

void TUI_update_mouth(const char* status, const char* text) {
    pthread_mutex_lock(&tui_mutex);
    strncpy(mouth_status, status, 63);
    strncpy(mouth_text, text, 255);
    render_all();
    pthread_mutex_unlock(&tui_mutex);
}

// --- New Control Functions ---

void TUI_set_mode(int mode) {
    pthread_mutex_lock(&tui_mutex);
    current_mode = mode;
    render_all();
    pthread_mutex_unlock(&tui_mutex);
}

void TUI_process_input(int ch) {
    if (ch == 'v' || ch == 'V') {
        TUI_set_mode(!current_mode); // Toggle
    }
}

void TUI_animate(void) {
    pthread_mutex_lock(&tui_mutex);
    anim_frame++;
    // Only redraw if we are in presentation mode or have active animations
    // For now, just redraw to keep it simple and responsive
    render_all();
    pthread_mutex_unlock(&tui_mutex);
}

void TUI_update_progress(float percent) {
    pthread_mutex_lock(&tui_mutex);
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
    pthread_mutex_unlock(&tui_mutex);
}

void TUI_close(void) {
    pthread_mutex_lock(&tui_mutex);
    endwin();
    pthread_mutex_unlock(&tui_mutex);
}