#define MAX_FOREIGN_FIELDS 99
#define MAX_EXAMPLE_LENGTH 9

#define NUMERIC_FIELD      0
#define TEXT_FIELD         1

struct import_field
{
    int name,
        number,
        password;
} mapped_fields;


struct foreign_format
{
   /* Define as struct to allow for future fields - especially DBF files */

    int size;
    char example[MAX_EXAMPLE_LENGTH+1]; /* limit example length so that it fits on screen */
    int type; 
} foreign_fields[MAX_FOREIGN_FIELDS];
