/*
 * edU - Prova finale di Algoritmi e Principi dell'Informatica (A.A. 2019/20)
 * Christian Grasso
 */

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#define MAX_CMD_LENGTH  32
#define MAX_LINE_LENGTH 1024
#define INITIAL_BUFSIZE 128

/* Types */
typedef char line[MAX_LINE_LENGTH + 1];

typedef struct {
    long size;
    long length;
    line** lines;
} line_buffer;

enum history_action { CHANGE, DELETE };

typedef struct history_entry {
    enum history_action action;
    long n1;
    long n2;
    long prev_length;
    line** buffer;

    struct history_entry* prev;
} history_entry;

/* Globals */
line_buffer text;
long history_count = 0;
history_entry* undo_history = NULL;
history_entry* redo_history = NULL;

/* Methods */
void buffer_grow() {
    text.size += 128;
    line** new_lines = realloc(text.lines, sizeof(line*) * text.size);
    if (new_lines == NULL) exit(1);
    text.lines = new_lines;
}

void history_swap(history_entry* entry) {
    for (long i = entry->n1; i <= entry->n2; i++) {
        line* tmp = entry->buffer[i - entry->n1];
        entry->buffer[i - entry->n1] = text.lines[i];
        text.lines[i] = tmp;
    }
}

void history_undo_clear() {
    while (undo_history != NULL) {
        history_entry* cur = undo_history;
        for (int i = 0; i < (cur->n2 - cur->n1 + 1); i++) {
            free(*cur->buffer[i]);
        }
        undo_history = undo_history->prev;
        free(cur);
    }
}

void history_redo_clear() {
    while (redo_history != NULL) {
        history_entry* cur = redo_history;
        for (int i = 0; i < (cur->n2 - cur->n1 + 1); i++) {
            free(*cur->buffer[i]);
        }
        redo_history = redo_history->prev;
        free(cur);
    }
}

void history_push(enum history_action action, long n1, long n2) {
    history_redo_clear();
    long copy_length = n2 - n1 + 1;
    line** buffer = calloc(copy_length, sizeof(line*));
    if (n1 + copy_length > text.length) copy_length = text.length - n1;
    if (copy_length < 0) copy_length = 0;
    memcpy(buffer, &text.lines[n1], sizeof(line*) * copy_length);
    history_entry* prev = undo_history;
    undo_history = malloc(sizeof(history_entry));
    *undo_history = (history_entry) {
        .action = action,
        .n1 = n1,
        .n2 = n2,
        .buffer = buffer,
        .prev_length = text.length,
        .prev = prev
    };
}

void text_delete(long n1, long n2) {
    if (n1 >= text.length) return;
    if (n2 + 1 < text.length) {
        memmove(
                &text.lines[n1],
                &text.lines[n2 + 1],
                sizeof(line*) * (text.length - n2 - 1)
        );
        text.length -= (n2 - n1 + 1);
    } else {
        text.length = n1 + 1;
    }
}

void cmd_change(long n1, long n2) {
    history_push(CHANGE, n1, n2);
    if (n2 > text.size) buffer_grow();
    if (n2 >= text.length) text.length = n2 + 1;
    for (long i = n1; i <= n2; i++) {
        line* l = malloc(sizeof(line));
        fgets(*l, MAX_LINE_LENGTH, stdin);
        (*l)[strcspn(*l, "\n")] = '\0';
        text.lines[i] = l;
    }
    scanf(".\n");
}

void cmd_delete(long n1, long n2) {
    history_push(DELETE, n1, n2);
    text_delete(n1, n2);
}

void cmd_print(long n1, long n2) {
    for (long i = n1; i <= n2; i++) {
        if (i >= 0 && i < text.length && text.lines[i] != NULL) {
            printf("%s\n", *text.lines[i]);
        } else {
            printf(".\n");
        }
    }
}

void cmd_undo(long n) {
    history_entry* cur;
    while ((cur = undo_history) != NULL && n-- > 0) {
        if (cur->action == CHANGE) {
            history_swap(cur);
        } else {
            long buf_length = cur->n2 - cur->n1 + 1;
            if (cur->n1 < text.length) {
                memmove(
                    &text.lines[cur->n2 + 1],
                    &text.lines[cur->n1],
                    sizeof(line*) * (text.length - cur->n1 - 1)
                );
            }
            memcpy(&text.lines[cur->n1], cur->buffer, sizeof(line*) * buf_length);
        }
        text.length = cur->prev_length;
        undo_history = undo_history->prev;
        cur->prev = redo_history;
        redo_history = cur;
    }
}

void cmd_redo(long n) {
    history_entry* cur;
    while ((cur = redo_history) != NULL && n-- > 0) {
        if (cur->action == CHANGE) {
            history_swap(cur);
        } else {
            text_delete(cur->n1, cur->n2);
        }
        redo_history = redo_history->prev;
        cur->prev = undo_history;
        undo_history = cur;
    }
}

void history_move() {
    if (history_count > 0) cmd_undo(history_count);
    else cmd_redo(-history_count);
    history_count = 0;
}

int parse_command() {
    // Assumption: input is not malformed
    char input[MAX_CMD_LENGTH];
    fgets(input, MAX_CMD_LENGTH, stdin);
    #ifdef DEBUG
        fputs(input, stderr);
    #endif
    if (input[0] == 'q') return 1;
    char* c;
    long n1 = strtol(input, &c, 10);
    if (*c == 'u') history_count += n1;
    else if (*c == 'r') history_count -= n1;
    else {
        if (history_count != 0) history_move();
        n1 -= 1;
        long n2 = strtol(c + 1, &c, 10) - 1;
        if (*c == 'c') cmd_change(n1, n2);
        else if (*c == 'd') cmd_delete(n1, n2);
        else if (*c == 'p') cmd_print(n1, n2);
    }
    return 0;
}

void cleanup() {
    history_undo_clear();
    history_redo_clear();
    free(text.lines);
}

int main() {
    text = (line_buffer) { .size = INITIAL_BUFSIZE, .length = 0, .lines = malloc(sizeof(line*) * INITIAL_BUFSIZE) };

    int quit;
    do {
        quit = parse_command();
    } while (!quit);

    cleanup();
    return 0;
}
