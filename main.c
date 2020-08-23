/*
 * edU - Prova finale di Algoritmi e Principi dell'Informatica (A.A. 2019/20)
 * Christian Grasso
 */

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#define INITIAL_TEXT_SIZE    1024
#define INITIAL_HISTORY_SIZE 64

/* Types */
typedef char* line;

typedef struct {
    int size;
    int length;
    line* lines;
} line_buffer;

enum history_action { CHANGE, DELETE };

typedef struct history_entry {
    enum history_action action;
    int n1;
    int n2;
    int prev_length;
    line* buffer;
    int buf_length;

    // struct history_entry* prev;
} history_entry;

typedef struct {
    int size;
    int length;
    int cursor; // index of the first redoable action
    history_entry* entries;
} history_arr;

/* Globals */
line_buffer text;
int history_move_count = 0;
history_arr history;

size_t MAX_LINE_LENGTH = 1026;

/* Methods */
void buffer_grow() {
    text.size *= 2;
    line* new_lines = realloc(text.lines, sizeof(line) * text.size);
    if (new_lines == NULL) exit(1);
    text.lines = new_lines;
}

void history_grow() {
    history.size *= 2;
    history_entry* new_entries = realloc(history.entries, sizeof(struct history_entry) * history.size);
    if (new_entries == NULL) exit(1);
    history.entries = new_entries;
}

void history_swap(history_entry* entry, int is_redo, int is_next_cmd_edit) {
    int extra_len = (entry->n2 - entry->n1 + 1) - entry->buf_length;
    if (is_redo || !is_next_cmd_edit) {
        for (int i = 0; i < entry->buf_length; i++) {
            line tmp = entry->buffer[i];
            entry->buffer[i] = text.lines[entry->n1 + i];
            text.lines[entry->n1 + i] = tmp;
        }
        if (extra_len > 0) {
            line* source = is_redo ? &entry->buffer[entry->buf_length] : &text.lines[entry->n1 + entry->buf_length];
            line* copy_to = is_redo ? &text.lines[entry->n1 + entry->buf_length] : &entry->buffer[entry->buf_length];
            memcpy(copy_to, source, sizeof(line) * extra_len);
        }
    } else {
        // No need to swap, the history buffer will be cleared immediately afterwards
        for (int i = 0; i < entry->buf_length; i++) {
            text.lines[entry->n1 + i] = entry->buffer[i];
        }
    }
}

void history_redo_clear() {
    for (int i = history.cursor; i < history.length; i++) {
        free(history.entries[i].buffer);
    }
    history.length = history.cursor;
}

void history_push(enum history_action action, int n1, int n2) {
    history_redo_clear();
    int block_length = n2 - n1 + 1;
    line* buffer;
    int buf_length;

    if (n1 + block_length > text.length) {
        buf_length = text.length - n1;
    } else {
        buf_length = block_length;
    }
    if (buf_length < 0) buf_length = 0;

    if (action == CHANGE) {
        // Allocate a full-sized buffer (needed for history_swap)
        buffer = malloc(sizeof(line) * block_length);
    } else {
        if (buf_length > 0) {
            buffer = malloc(sizeof(line) * buf_length);
        } else {
            buffer = NULL;
        }
    }

    memcpy(buffer, &text.lines[n1], sizeof(line) * buf_length);

    if (history.cursor + 1 >= history.size) {
        history_grow();
    }
    history.entries[history.cursor] = (history_entry) {
        .action = action,
        .n1 = n1,
        .n2 = n2,
        .prev_length = text.length,
        .buffer = buffer,
        .buf_length = buf_length
    };
    history.cursor++;
    if (history.cursor >= history.length) {
        history.length = history.cursor;
    }
}

void text_delete(int n1, int n2) {
    if (n1 >= text.length) return;
    if (n2 + 1 < text.length) {
        memmove(
                &text.lines[n1],
                &text.lines[n2 + 1],
                sizeof(line) * (text.length - n2 - 1)
        );
        text.length -= (n2 - n1 + 1);
    } else {
        text.length = n1;
    }
}

void cmd_change(int n1, int n2) {
    history_push(CHANGE, n1, n2);
    while (n2 > text.size) buffer_grow();
    if (n2 >= text.length) text.length = n2 + 1;
    for (int i = n1; i <= n2; i++) {
        text.lines[i] = malloc(MAX_LINE_LENGTH);
        ssize_t read = getline(&text.lines[i], &MAX_LINE_LENGTH, stdin);
        text.lines[i] = realloc(text.lines[i], read + 1);
    }
    getchar(); getchar(); // .\n
}

void cmd_delete(int n1, int n2) {
    history_push(DELETE, n1, n2);
    text_delete(n1, n2);
}

void cmd_print(int n1, int n2) {
    for (int i = n1; i <= n2; i++) {
        if (i >= 0 && i < text.length && text.lines[i] != NULL) {
            fputs(text.lines[i], stdout);
        } else {
            putc('.', stdout); putc('\n', stdout);
        }
    }
}

void cmd_undo(int n, int is_next_cmd_edit) {
    for (int i = 0; i < n; i++) {
        if (history.cursor <= 0) break;
        history_entry* cur = &history.entries[--history.cursor];
        int curr_len = text.length;
        if (cur->action == CHANGE) {
            history_swap(cur, 0, is_next_cmd_edit);
        } else {
            if (cur->n1 < text.length) {
                memmove(
                    &text.lines[cur->n2 + 1],
                    &text.lines[cur->n1],
                    sizeof(line) * (text.length - cur->n1)
                );
            }
            if (cur->buffer != NULL) {
                memcpy(&text.lines[cur->n1], cur->buffer, sizeof(line) * cur->buf_length);
            }
        }
        text.length = cur->prev_length;
        cur->prev_length = curr_len;
    }
}

void cmd_redo(int n, int is_next_cmd_edit) {
    for (int i = 0; i < n; i++) {
        if (history.cursor >= history.length) break;
        history_entry* cur = &history.entries[history.cursor++];
        int curr_len = text.length;
        if (cur->action == CHANGE) {
            history_swap(cur, 1, is_next_cmd_edit);
        } else {
            text_delete(cur->n1, cur->n2);
        }
        text.length = cur->prev_length;
        cur->prev_length = curr_len;
    }
}

void history_move(int is_next_cmd_edit) {
    if (history_move_count > 0) {
        cmd_undo(history_move_count, is_next_cmd_edit);
    } else {
        cmd_redo(-history_move_count, is_next_cmd_edit);
    }
    history_move_count = 0;
}

int parse_command() {
    char c = (char) getchar();
    if (c == 'q') return 1;
    int n1 = c - '0';
    while ((c = (char) getchar()) && c >= '0' && c <= '9') {
        n1 = n1 * 10 + c - '0';
    }
    if (c == 'u') {
        history_move_count += n1;
        if (history_move_count > history.cursor) history_move_count = history.cursor;
        getchar(); // \n
    } else if (c == 'r') {
        history_move_count -= n1;
        int max_redoable = history.length - history.cursor;
        if (-history_move_count > max_redoable) history_move_count = -max_redoable;
        getchar(); // \n
    } else {
        n1 -= 1;
        int n2 = getchar() - '0';
        while ((c = (char) getchar()) && c >= '0' && c <= '9') {
            n2 = n2 * 10 + c - '0';
        }
        n2 -= 1;
        getchar(); // \n
        if (history_move_count != 0) history_move(c == 'c' || c == 'd');
        if (c == 'c') cmd_change(n1, n2);
        else if (c == 'd') cmd_delete(n1, n2);
        else if (c == 'p') cmd_print(n1, n2);
    }
    return 0;
}

int main() {
    text = (line_buffer) {
        .size = INITIAL_TEXT_SIZE,
        .length = 0,
        .lines = malloc(sizeof(line) * INITIAL_TEXT_SIZE)
    };

    history = (history_arr) {
        .size = INITIAL_HISTORY_SIZE,
        .length = 0,
        .cursor = 0,
        .entries = malloc(sizeof(history_entry) * INITIAL_HISTORY_SIZE)
    };

    int quit;
    do {
        quit = parse_command();
    } while (!quit);

    return 0;
}
