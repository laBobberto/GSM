#include "evaluator.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

typedef struct {
    int row;
    int col;
} CellRef;

static int compare_cols(const void* a, const void* b) {
    return strcmp(((ColIndex*)a)->name, ((ColIndex*)b)->name);
}

static int compare_rows(const void* a, const void* b) {
    return (((RowIndex*)a)->row_val - ((RowIndex*)b)->row_val);
}

static int find_column_index(Table* table, const char* name) {
    if (!table->col_map) return -1;
    ColIndex key = {(char*)name, 0};
    ColIndex* res = bsearch(&key, table->col_map, table->cols, sizeof(ColIndex), compare_cols);
    return res ? res->original_index : -1;
}

static int find_row_index(Table* table, int index_val) {
    if (!table->row_map) return -1;
    RowIndex key = {index_val, 0};
    RowIndex* res = bsearch(&key, table->row_map, table->rows, sizeof(RowIndex), compare_rows);
    return res ? res->original_index : -1;
}

static int resolve_operand(Table* table, const char* op_str, int* error);

static int evaluate_cell_recursive(Table* table, int r, int c);

static int resolve_operand(Table* table, const char* op_str, int* error) {
    if (isdigit(op_str[0]) || (op_str[0] == '-' && isdigit(op_str[1]))) {
        return atoi(op_str);
    }

    char col_name[256];
    int row_val;
    int i = 0;
    while (op_str[i] && !isdigit(op_str[i])) {
        col_name[i] = op_str[i];
        i++;
    }
    col_name[i] = '\0';
    if (op_str[i] == '\0') {
        *error = 1;
        return 0;
    }
    row_val = atoi(op_str + i);

    int col_idx = find_column_index(table, col_name);
    int row_idx = find_row_index(table, row_val);

    if (col_idx == -1 || row_idx == -1) {
        *error = 1;
        return 0;
    }

    int val = evaluate_cell_recursive(table, row_idx, col_idx);
    if (table->cells[row_idx][col_idx].state == CELL_ERROR) {
        *error = 1;
    }
    return val;
}

static int evaluate_cell_recursive(Table* table, int r, int c) {
    Cell* cell = &table->cells[r][c];

    if (cell->state == CELL_VALUE) return cell->value;
    if (cell->state == CELL_ERROR) return 0;
    if (cell->state == CELL_EVALUATING) {
        cell->state = CELL_ERROR;
        return 0;
    }

    cell->state = CELL_EVALUATING;

    if (cell->raw_text[0] == '=') {
        char* expr = cell->raw_text + 1;
        while (isspace((unsigned char)*expr)) expr++;

        char arg1_str[256], arg2_str[256], op_char;
        
        char* search_ptr = expr;
        if (*search_ptr == '+' || *search_ptr == '-') search_ptr++;
        
        char* op_ptr = strpbrk(search_ptr, "+-*/");
        if (!op_ptr) {
            cell->state = CELL_ERROR;
            return 0;
        }
        op_char = *op_ptr;

        int arg1_len = op_ptr - expr;
        if (arg1_len >= (int)sizeof(arg1_str)) arg1_len = sizeof(arg1_str) - 1;
        strncpy(arg1_str, expr, arg1_len);
        arg1_str[arg1_len] = '\0';
        strncpy(arg2_str, op_ptr + 1, sizeof(arg2_str) - 1);
        arg2_str[sizeof(arg2_str) - 1] = '\0';

        char *a1 = arg1_str, *a2 = arg2_str;
        while(isspace(*a1)) a1++;
        char* end = a1 + strlen(a1) - 1;
        while(end > a1 && isspace(*end)) *end-- = '\0';

        while(isspace(*a2)) a2++;
        end = a2 + strlen(a2) - 1;
        while(end > a2 && isspace(*end)) *end-- = '\0';

        int error = 0;
        int val1 = resolve_operand(table, a1, &error);
        if (error) {
            cell->state = CELL_ERROR;
            return 0;
        }
        int val2 = resolve_operand(table, a2, &error);
        if (error) {
            cell->state = CELL_ERROR;
            return 0;
        }

        switch (op_char) {
            case '+': cell->value = val1 + val2; break;
            case '-': cell->value = val1 - val2; break;
            case '*': cell->value = val1 * val2; break;
            case '/':
                if (val2 == 0) {
                    cell->state = CELL_ERROR;
                    return 0;
                }
                cell->value = val1 / val2;
                break;
            default: cell->state = CELL_ERROR; return 0;
        }
    } else {
        if (isdigit(cell->raw_text[0]) || (cell->raw_text[0] == '-' && isdigit(cell->raw_text[1]))) {
            cell->value = atoi(cell->raw_text);
        } else {
            if (strlen(cell->raw_text) == 0) {
                 cell->value = 0; 
            } else {
                 cell->state = CELL_ERROR;
                 return 0;
            }
        }
    }

    if (cell->state != CELL_ERROR) {
        cell->state = CELL_VALUE;
    }
    return cell->value;
}

void evaluate_table(Table* table) {
    table->col_map = malloc(table->cols * sizeof(ColIndex));
    for (int i = 0; i < table->cols; i++) {
        table->col_map[i].name = table->col_names[i];
        table->col_map[i].original_index = i;
    }
    qsort(table->col_map, table->cols, sizeof(ColIndex), compare_cols);

    table->row_map = malloc(table->rows * sizeof(RowIndex));
    for (int i = 0; i < table->rows; i++) {
        table->row_map[i].row_val = table->row_indices[i];
        table->row_map[i].original_index = i;
    }
    qsort(table->row_map, table->rows, sizeof(RowIndex), compare_rows);

    for (int i = 0; i < table->rows; i++) {
        for (int j = 0; j < table->cols; j++) {
            evaluate_cell_recursive(table, i, j);
        }
    }
}
