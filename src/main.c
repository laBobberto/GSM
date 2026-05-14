#include "parser.h"
#include "evaluator.h"
#include "table.h"
#include <stdio.h>
#include <stdlib.h>

void print_table(Table* table) {
    if (!table) return;

    printf(",");
    for (int i = 0; i < table->cols; i++) {
        printf("%s%s", table->col_names[i], (i == table->cols - 1) ? "" : ",");
    }
    printf("\n");

    for (int i = 0; i < table->rows; i++) {
        printf("%d,", table->row_indices[i]);
        for (int j = 0; j < table->cols; j++) {
            if (table->cells[i][j].state == CELL_VALUE) {
                printf("%d", table->cells[i][j].value);
            } else if (table->cells[i][j].state == CELL_ERROR) {
                printf("#ERR");
            } else {
                printf("%s", table->cells[i][j].raw_text);
            }
            if (j < table->cols - 1) printf(",");
        }
        printf("\n");
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename.csv>\n", argv[0]);
        return 1;
    }

    Table* table = parse_csv(argv[1]);
    if (!table) {
        fprintf(stderr, "Error: Could not parse file %s\n", argv[1]);
        return 1;
    }

    evaluate_table(table);

    print_table(table);

    free_table(table);

    return 0;
}
