/*
 * edU - Prova finale di Algoritmi e Principi dell'Informatica (A.A. 2019/20)
 * Christian Grasso
 */

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#define MAX_CMD_LENGTH    16
#define MAX_LINE_LENGTH   1024
#define INITIAL_TEXT_SIZE 512

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

    struct history_entry* prev;
} history_entry;

/* Globals */
line_buffer text;
int history_count = 0;
int history_undo_size = 0;
int history_redo_size = 0;
history_entry* history_undo = NULL;
history_entry* history_redo = NULL;

/* Methods */
void buffer_grow() {
    text.size *= 2;
    line* new_lines = realloc(text.lines, sizeof(line) * text.size);
    if (new_lines == NULL) exit(16);
    text.lines = new_lines;
}

void history_swap(history_entry* entry) {
    for (int i = entry->n1; i <= entry->n2; i++) {
        line tmp = entry->buffer[i - entry->n1];
        entry->buffer[i - entry->n1] = text.lines[i];
        text.lines[i] = tmp;
    }
}

void history_undo_clear() {
    while (history_undo != NULL) {
        history_entry* cur = history_undo;
        history_undo = history_undo->prev;
        free(cur);
    }
    history_undo_size = 0;
}

void history_redo_clear() {
    while (history_redo != NULL) {
        history_entry* cur = history_redo;
        history_redo = history_redo->prev;
        free(cur);
    }
    history_redo_size = 0;
}

void history_push(enum history_action action, int n1, int n2) {
    history_redo_clear();
    int copy_length = n2 - n1 + 1;
    line* buffer = calloc(copy_length, sizeof(line));
    if (n1 + copy_length > text.length) copy_length = text.length - n1;
    if (copy_length > 0) {
        memcpy(buffer, &text.lines[n1], sizeof(line) * copy_length);
    }
    history_entry* prev = history_undo;
    history_undo = malloc(sizeof(history_entry));
    *history_undo = (history_entry) {
        .action = action,
        .n1 = n1,
        .n2 = n2,
        .buffer = buffer,
        .prev_length = text.length,
        .prev = prev
    };
    history_undo_size++;
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
        text.lines[i] = malloc(MAX_LINE_LENGTH + 2);
        fgets(text.lines[i], MAX_LINE_LENGTH + 2, stdin);
        text.lines[i] = realloc(text.lines[i], strlen(text.lines[i]) + 1);
    }
    scanf(".\n");
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
            fputs(".\n", stdout);
        }
    }
}

void cmd_undo(int n) {
    history_entry* cur;
    while ((cur = history_undo) != NULL && n-- > 0) {
        if (cur->action == CHANGE) {
            history_swap(cur);
        } else {
            int buf_length = cur->n2 - cur->n1 + 1;
            if (cur->n1 < text.length) {
                memmove(
                    &text.lines[cur->n2 + 1],
                    &text.lines[cur->n1],
                    sizeof(line) * (text.length - cur->n1)
                );
            }
            memcpy(&text.lines[cur->n1], cur->buffer, sizeof(line) * buf_length);
        }
        int curr_len = text.length;
        text.length = cur->prev_length;
        cur->prev_length = curr_len;
        history_undo = history_undo->prev;
        cur->prev = history_redo;
        history_redo = cur;
    }
}

void cmd_redo(int n) {
    history_entry* cur;
    while ((cur = history_redo) != NULL && n-- > 0) {
        int curr_len = text.length;
        if (cur->action == CHANGE) {
            history_swap(cur);
        } else {
            text_delete(cur->n1, cur->n2);
        }
        text.length = cur->prev_length;
        cur->prev_length = curr_len;
        history_redo = history_redo->prev;
        cur->prev = history_undo;
        history_undo = cur;
    }
}

void history_move() {
    #ifdef DEBUG
        fprintf(stdout, ">>> history_move: %d\n", history_count);
    #endif
    if (history_count > 0) {
        cmd_undo(history_count);
    } else {
        cmd_redo(-history_count);
    }
    history_undo_size -= history_count;
    history_redo_size += history_count;
    history_count = 0;
}

int parse_command() {
    // Assumption: input is not malformed
    char input[MAX_CMD_LENGTH];
    fgets(input, MAX_CMD_LENGTH, stdin);
    #ifdef DEBUG
        fputs(input, stdout);
    #endif
    if (input[0] == 'q') return 1;
    char* c;
    int n1 = (int) strtol(input, &c, 10);
    if (*c == 'u') {
        history_count += n1;
        if (history_count > history_undo_size) history_count = history_undo_size;
    } else if (*c == 'r') {
        history_count -= n1;
        if (-history_count > history_redo_size) history_count = -history_redo_size;
    } else {
        if (history_count != 0) history_move();
        n1 -= 1;
        int n2 = (int) strtol(c + 1, &c, 10) - 1;
        if (*c == 'c') cmd_change(n1, n2);

        else if (*c == 'd') cmd_delete(n1, n2);
        else if (*c == 'p') cmd_print(n1, n2);
    }
    return 0;
}

void cleanup() {
    history_undo_clear();
    history_redo_clear();
    for (int i = 0; i < text.length; i++) {
        free(text.lines[i]);
    }
    free(text.lines);
}

int main() {
    text = (line_buffer) { .size = INITIAL_TEXT_SIZE, .length = 0, .lines = calloc(INITIAL_TEXT_SIZE, sizeof(line)) };

    int quit;
    do {
        quit = parse_command();
    } while (!quit);

    cleanup();
    return 0;
}
