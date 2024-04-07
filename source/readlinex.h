#ifndef __LINENOISE_H
#define __LINENOISE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h> /* For size_t. */

extern char *readlinexEditMore;

/* The readlinexState structure represents the state during line editing.
 * We pass this state to functions implementing specific editing
 * functionalities. */
struct readlinexState {
	int in_completion;  /* The user pressed TAB and we are now in completion
                         * mode, so input is handled by completeLine(). */
	size_t completion_idx; /* Index of next completion to propose. */
	int ifd;            /* Terminal stdin file descriptor. */
	int ofd;            /* Terminal stdout file descriptor. */
	char *buf;          /* Edited line buffer. */
	size_t buflen;      /* Edited line buffer size. */
	const char *prompt; /* Prompt to display. */
	size_t plen;        /* Prompt length. */
	size_t pos;         /* Current cursor position. */
	size_t oldpos;      /* Previous refresh cursor position. */
	size_t len;         /* Current edited line length. */
	size_t cols;        /* Number of columns in terminal. */
	size_t oldrows;     /* Rows used by last refrehsed line (multiline mode) */
	int history_index;  /* The history index we are currently editing. */
};

typedef struct readlinexCompletions {
	size_t len;
	char **cvec;
} readlinexCompletions;

/* Non blocking API. */
int
readlinexEditStart(struct readlinexState *l, int stdin_fd, int stdout_fd, char *buf, size_t buflen, const char *prompt);
char *readlinexEditFeed(struct readlinexState *l);
void readlinexEditStop(struct readlinexState *l);
void readlinexHide(struct readlinexState *l);
void readlinexShow(struct readlinexState *l);

/* Blocking API. */
char *readlinex(const char *prompt);
void readlinexFree(void *ptr);

/* Completion API. */
typedef void(readlinexCompletionCallback)(const char *, readlinexCompletions *);
typedef char *(readlinexHintsCallback)(const char *, int *color, int *bold);
typedef void(readlinexFreeHintsCallback)(void *);
void readlinexSetCompletionCallback(readlinexCompletionCallback *);
void readlinexSetHintsCallback(readlinexHintsCallback *);
void readlinexSetFreeHintsCallback(readlinexFreeHintsCallback *);
void readlinexAddCompletion(readlinexCompletions *, const char *);

/* History API. */
int readlinexHistoryAdd(const char *line);
int readlinexHistorySetMaxLen(int len);
int readlinexHistorySave(const char *filename);
int readlinexHistoryLoad(const char *filename);

/* Other utilities. */
void readlinexClearScreen(void);
void readlinexSetMultiLine(int ml);
void readlinexPrintKeyCodes(void);
void readlinexMaskModeEnable(void);
void readlinexMaskModeDisable(void);

#ifdef __cplusplus
}
#endif

#endif /* __LINENOISE_H */
