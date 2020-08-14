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
    line* lines;
} line_buffer;

/* Globals */
line_buffer buffer;
long history_count = 0;

/* Methods */
void buffer_grow() {
    buffer.size *= 2;
    line* new_lines = realloc(buffer.lines, sizeof(line) * buffer.size);
    if (new_lines == NULL) exit(1);
    buffer.lines = new_lines;
}

void cmd_change(long n1, long n2) {
    if (n2 > buffer.size) buffer_grow();
    if (n2 >= buffer.length) buffer.length = n2 + 1;
    for (long i = n1; i <= n2; i++) {
        fgets(buffer.lines[i], MAX_LINE_LENGTH, stdin);
        buffer.lines[i][strcspn(buffer.lines[i], "\n")] = '\0';
    }
    scanf(".\n");
}

void cmd_delete(long n1, long n2) {
    if (n1 >= buffer.length) return;
    if (n2 + 1 < buffer.length) {
        memmove(buffer.lines[n1], buffer.lines[n2 + 1], sizeof(line) * (buffer.length - n2 - 1));
        buffer.length -= (n2 - n1 + 1);
    } else {
        buffer.length = n1 + 1;
    }
}

void cmd_print(long n1, long n2) {
    for (long i = n1; i <= n2; i++) {
        if (i < buffer.length) {
            printf("%s\n", buffer.lines[i]);
        } else {
            printf(".\n");
        }
    }
}

void cmd_undo(long n) {
    fprintf(stderr, "cmd_undo %ld\n", n);
    // TODO
}

void cmd_redo(long n) {
    fprintf(stderr, "cmd_redo %ld\n", n);
    // TODO
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
    if (input[0] == 'q') return 1;
    char* c;
    long n1 = strtol(input, &c, 10) - 1;
    if (*c == 'u') history_count += n1;
    else if (*c == 'r') history_count -= n1;
    else {
        if (history_count != 0) history_move();
        long n2 = strtol(c + 1, &c, 10) - 1;
        if (*c == 'c') cmd_change(n1, n2);
        else if (*c == 'd') cmd_delete(n1, n2);
        else if (*c == 'p') cmd_print(n1, n2);
    }
    return 0;
}

void cleanup() {
    free(buffer.lines);
}

int main() {
    // Initialize line buffer
    buffer = (line_buffer) { .size = INITIAL_BUFSIZE, .length = 0, .lines = malloc(sizeof(line) * INITIAL_BUFSIZE) };

    int quit;
    do {
        quit = parse_command();
    } while (!quit);

    cleanup();
    return 0;
}
