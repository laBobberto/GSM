#include "parser.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static char* trim_whitespace(char* str) {
    char* end;
    while(isspace((unsigned char)*str)) str++;
    if(*str == 0) return str;
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

static char* duplicate_string(const char* s) {
    size_t len = strlen(s);
    char* d = malloc(len + 1);
    if (d) strcpy(d, s);
    return d;
}

static char* get_next_field(char** line_ptr) {
    if (*line_ptr == NULL) return NULL;
    char* start = *line_ptr;
    char* comma = strchr(start, ',');
    if (comma) {
        *comma = '\0';
        *line_ptr = comma + 1;
    } else {
        // Handle newline at the end
        char* newline = strpbrk(start, "\r\n");
        if (newline) *newline = '\0';
        *line_ptr = NULL;
    }
    return start;
}

Table* parse_csv(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return NULL;

    char line[8192];
    if (!fgets(line, sizeof(line), file)) {
        fclose(file);
        return NULL;
    }

    int cols = 0;
    char* line_ptr = line;
    char* field;
    
    field = get_next_field(&line_ptr);
    
    int col_capacity = 10;
    char** col_names = malloc(col_capacity * sizeof(char*));

    while ((field = get_next_field(&line_ptr)) != NULL) {
        if (cols >= col_capacity) {
            col_capacity *= 2;
            col_names = realloc(col_names, col_capacity * sizeof(char*));
        }
        col_names[cols++] = duplicate_string(trim_whitespace(field));
    }

    if (cols == 0) {
        free(col_names);
        fclose(file);
        return NULL;
    }

    int row_capacity = 10;
    int rows = 0;
    int* row_indices = malloc(row_capacity * sizeof(int));
    Cell** cells = malloc(row_capacity * sizeof(Cell*));

    while (fgets(line, sizeof(line), file)) {
        line_ptr = line;
        field = get_next_field(&line_ptr);
        if (!field) continue;

        if (rows >= row_capacity) {
            row_capacity *= 2;
            row_indices = realloc(row_indices, row_capacity * sizeof(int));
            cells = realloc(cells, row_capacity * sizeof(Cell*));
        }

        row_indices[rows] = atoi(trim_whitespace(field));
        cells[rows] = calloc(cols, sizeof(Cell));

        for (int i = 0; i < cols; i++) {
            field = get_next_field(&line_ptr);
            if (field) {
                cells[rows][i].raw_text = duplicate_string(trim_whitespace(field));
            } else {
                cells[rows][i].raw_text = duplicate_string("");
            }
            cells[rows][i].state = CELL_RAW;
        }
        rows++;
    }

    fclose(file);

    Table* table = malloc(sizeof(Table));
    table->rows = rows;
    table->cols = cols;
    table->col_names = col_names;
    table->row_indices = row_indices;
    table->cells = cells;

    return table;
}
