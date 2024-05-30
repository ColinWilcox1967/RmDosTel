#define MAX_MERGE_FILES 10

struct type_struct
{
    char title[11];
    int offset;
    char highlight_char;
};

struct merge_entry
{
    char filename[67];
    int  file_type;
    int  used;
};

