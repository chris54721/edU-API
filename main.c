/*
 * edU - Prova finale di Algoritmi e Principi dell'Informatica (A.A. 2019/20)
 * Christian Grasso
 */

#include <stdlib.h>
#include <stdio.h>

#define MAX_CMD_LENGTH  32
#define MAX_LINE_LENGTH 1024
#define INITIAL_BUFSIZE 1024

/* Types */
typedef char line[MAX_LINE_LENGTH + 1];

typedef struct {
    int size;
    int length;
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

void buffer_read() {
    if (buffer.length >= buffer.size) buffer_grow();
    scanf("%s", buffer.lines[buffer.length]);
    buffer.length++;
}

void cmd_change(long n1, long n2) {
    fprintf(stderr, "cmd_change %ld %ld\n", n1, n2);
    // TODO
}

void cmd_delete(long n1, long n2) {
    fprintf(stderr, "cmd_delete %ld %ld\n", n1, n2);
    // TODO
}

void cmd_print(long n1, long n2) {
    fprintf(stderr, "cmd_print %ld %ld\n", n1, n2);
    // TODO
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
    long n1 = strtol(input, &c, 10);
    if (*c == 'u') history_count += n1;
    else if (*c == 'r') history_count -= n1;
    else {
        if (history_count != 0) history_move();
        long n2 = strtol(c + 1, &c, 10);
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
