/**
 * @file tui_manager.h
 * @brief Text User Interface (TUI) Manager using ncurses.
 */

#ifndef TUI_MANAGER_H
#define TUI_MANAGER_H

/**
 * @brief Initialize the TUI.
 *
 * Sets up ncurses, colors, and the initial layout.
 */
void TUI_init(void);

/**
 * @brief Close the TUI and restore the terminal.
 */
void TUI_close(void);

/**
 * @brief Update the "Ear" panel (Board 1 status).
 *
 * @param status The current status string.
 * @param text The text content to display.
 */
void TUI_update_ear(const char *status, const char *text);

/**
 * @brief Update the "Brain" panel (Board 2 status).
 *
 * @param status The current status string.
 * @param text The text content to display.
 */
void TUI_update_brain(const char *status, const char *text);

/**
 * @brief Update the "Mouth" panel (Board 3 status).
 *
 * @param status The current status string.
 * @param text The text content to display.
 */
void TUI_update_mouth(const char *status, const char *text);

/**
 * @brief Log a message to the Network Console panel.
 *
 * @param fmt Format string (printf-style).
 * @param ... Arguments for the format string.
 */
void TUI_log(const char *fmt, ...);

/**
 * @brief Update the global progress bar.
 *
 * @param percent Progress percentage (0.0 to 1.0).
 */
void TUI_update_progress(float percent);

#endif // TUI_MANAGER_H