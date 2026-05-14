#ifndef TABLE_H
#define TABLE_H

typedef enum {
    CELL_RAW,
    CELL_VALUE,
    CELL_ERROR,
    CELL_EVALUATING
} CellState;

typedef struct {
    char* raw_text;
    int value;
    CellState state;
} Cell;

typedef struct {
    char* name;
    int original_index;
} ColIndex;

typedef struct {
    int row_val;
    int original_index;
} RowIndex;

typedef struct {
    int rows;
    int cols;
    char** col_names;
    int* row_indices;
    Cell** cells;
    ColIndex* col_map;
    RowIndex* row_map;
} Table;

Table* create_table(int rows, int cols);
void free_table(Table* table);

#endif
