#include "table.h"
#include <stdlib.h>
#include <string.h>

Table* create_table(int rows, int cols) {
    Table* table = (Table*)malloc(sizeof(Table));
    if (!table) return NULL;

    table->rows = rows;
    table->cols = cols;
    table->col_names = (char**)calloc(cols, sizeof(char*));
    table->row_indices = (int*)calloc(rows, sizeof(int));
    table->cells = (Cell**)malloc(rows * sizeof(Cell*));

    if (!table->col_names || !table->row_indices || !table->cells) {
        free_table(table);
        return NULL;
    }

    table->col_map = NULL;
    table->row_map = NULL;

    for (int i = 0; i < rows; i++) {
        table->cells[i] = (Cell*)calloc(cols, sizeof(Cell));
        if (!table->cells[i]) {
            free_table(table);
            return NULL;
        }
    }

    return table;
}

void free_table(Table* table) {
    if (!table) return;

    if (table->col_names) {
        for (int i = 0; i < table->cols; i++) {
            free(table->col_names[i]);
        }
        free(table->col_names);
    }

    if (table->row_indices) {
        free(table->row_indices);
    }

    if (table->col_map) free(table->col_map);
    if (table->row_map) free(table->row_map);

    if (table->cells) {
        for (int i = 0; i < table->rows; i++) {
            if (table->cells[i]) {
                for (int j = 0; j < table->cols; j++) {
                    free(table->cells[i][j].raw_text);
                }
                free(table->cells[i]);
            }
        }
        free(table->cells);
    }

    free(table);
}
