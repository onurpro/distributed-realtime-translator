#ifndef TUI_MANAGER_H
#define TUI_MANAGER_H

void TUI_init(void);
void TUI_close(void);

// Update specific sections
void TUI_update_ear(const char* status, const char* text);
void TUI_update_brain(const char* status, const char* text);
void TUI_update_mouth(const char* status, const char* text);

// Update progress (0.0 to 1.0)
void TUI_update_progress(float percent);

#endif