#ifndef TUI_MANAGER_H
#define TUI_MANAGER_H

void TUI_init(void);
void TUI_close(void);

// Update specific sections (Left Column)
void TUI_update_ear(const char* status, const char* text);
void TUI_update_brain(const char* status, const char* text);
void TUI_update_mouth(const char* status, const char* text);

// Log to the Network Console (Right Column)
// Supports formatting like printf: TUI_log("Sent %d bytes", bytes);
void TUI_log(const char* fmt, ...);

// Update progress (Bottom)
// Update progress (Bottom)
void TUI_update_progress(float percent);

// Interactive & Visuals
void TUI_set_mode(int mode); // 0 = Standard, 1 = Presentation
void TUI_process_input(int ch);
void TUI_animate(void);      // Call periodically for animations

#endif