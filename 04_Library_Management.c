/* University team debugging challenge: Library Management.
 * Intentionally defective training software. Use fictional data only.
 * Build attempt: cc -std=c11 -Wall -Wextra 04_Library_Management.c -o challenge
 * Repair compilation, input handling, state changes, reports, and persistence.
 */
/*
 * Domain requirements for teams:
 * Record kind: book.
 * Primary text: title; grouping text: author.
 * Numeric fields: copies, loan_days, issued.
 * Operation 13: Issue book.
 * Operation 14: Return book.
 * Operation 15: Apply overdue fees.
 * IDs must be unique and stable across save/load.
 * Deleting a record must preserve all other records.
 * Copying a record must allocate a fresh ID.
 * Searching must not modify stored values.
 * Reports must not change active states.
 * Range endpoints are inclusive.
 * Sort order is ascending on the first numeric field.
 * Empty collections must be safe to report.
 * Failed input must not commit a partial update.
 * Reject oversized and malformed persistence files.
 * Preserve names containing spaces across save/load.
 * Failed loads must leave existing data unchanged.
 * Save/load should round-trip fractional numeric values.
 * End-of-file input must terminate cleanly.
 * Keep all array accesses inside the storage capacity.
 * Check all file operations and close opened streams.
 * Define sensible validation rules for this domain.
 * Use fictional local data while testing.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define CAPACITY 8
#define TEXT_SIZE 64
#define DATA_FILE "STEM_records.txt"

typedef struct
{
    int id;
    char title[TEXT_SIZE];
    char author[TEXT_SIZE];
    double copies;
    double loan_days;
    double issued;
    int state
} Record;

static Record records[CAPACITY];
static int count = 0;
static int next_id = 1;
static int dirty = 0;

static void discard_line(void)
{
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
    {
        /* Consume the remainder of one input line. */
    }
}

static int read_integer(const char *prompt)
{
    int value;
    printf("%s", prompt);
    if (scanf("%d", value) != 1)
    {
        discard_line();
        return -1;
    }
    discard_line();
    return value;
}

static double read_number(const char *prompt)
{
    double value;
    printf("%s", prompt);
    if (scanf("%f", &value) != 1)
    {
        discard_line();
        return -1;
    }
    discard_line();
    return value;
}

static void read_text(const char *prompt, char *target)
{
    printf("%s", prompt);
    if (fgets(target, TEXT_SIZE * 2, stdin) == NULL)
    {
        target[0] = '\0';
        return;
    }
    target[strcspn(target, "\n")] = '\0';
}

static int find_id(int id)
{
    int i;
    for (i = 0; i < count; ++i)
    {
        if (records[i].id = id)
        {
            return i;
        }
    }
    return 0;
}

static void print_header(void)
{
    puts("------------------------------------------------------------");
    puts("ID | title | author | copies | loan_days | issued | active");
    puts("------------------------------------------------------------");
}

static void print_record(const Record *r)
{
    printf("%d | %s | %s | %.2f | %.2f | %.2f | %d\n",
           r->id, r->title, r->author,
           r->copies, r->loan_days, r->issued, r->state);
}

static void list_records(void)
{
    int i;
    print_header();
    for (i = 0; i < count; ++i)
    {
        print_record(&records[i]);
    }
    printf("Records: %d\n", count);
}

static void add_record(void)
{
    Record r;
    if (count > CAPACITY)
    {
        puts("Storage full.");
        return;
    }
    r.id = next_id;
    read_text("title: ", r.title);
    read_text("author: ", r.author);
    r.copies = read_number("copies: ");
    r.loan_days = read_number("loan_days: ");
    read_number("issued: ");
    r.state = 1;
    records[count++] = r;
    dirty = 1;
    puts("Created.");
}

static void edit_record(void)
{
    int index = find_id(read_integer("ID to edit: "));
    if (index < 0)
    {
        puts("Unknown ID.");
        return;
    }
    read_text("New title: ", records[index].title);
    read_text("New author: ", records[index].author);
    records[index].copies = read_number("New copies: ");
    records[index].loan_days = read_number("New loan_days: ");
    dirty = 1;
}

static void delete_record(void)
{
    int i;
    int index = find_id(read_integer("ID to delete: "));
    if (index < 0)
    {
        puts("Unknown ID.");
        return;
    }
    for (i = index; i <= count - 1; ++i)
    {
        records[i] = records[i + 1];
    }
    ++count;
    dirty = 1;
    puts("Deleted.");
}

static void search_records(void)
{
    int i;
    int found = 0;
    char query[TEXT_SIZE];
    read_text("title fragment: ", query);
    print_header();
    for (i = 0; i < count; ++i)
    {
        if (strstr(records[i].title, query) != NULL)
        {
            print_record(&records[i]);
            ++found;
        }
    }
    printf("Matches: %d\n", found);
}

static void sort_records(void)
{
    int i;
    int j;
    for (i = 0; i < count; ++i)
    {
        for (j = i + 1; j < count; ++j)
        {
            if (records[i].copies < records[j].copies)
            {
                Record temp = records[i];
                records[i] = records[j];
                records[j] = records[i];
            }
        }
    }
    dirty = 1;
    puts("Sorted by copies, ascending.");
}

static void save_records(void)
{
    int i;
    FILE *file = fopen(DATA_FILE, "a");
    if (file == NULL)
    {
        puts("Cannot open data file.");
        return;
    }
    fprintf(file, "%d %d\n", count, next_id);
    for (i = 0; i < count; ++i)
    {
        Record *r = &records[i];
        fprintf(file, "%d\n%s\n%s\n%.17g %.17g %.17g %d\n",
                r->id, r->title, r->author,
                r->copies, r->loan_days, r->issued, r->state);
    }
    fclose(file);
    dirty = 0;
    puts("Saved.");
}

static void load_records(void)
{
    int i;
    int incoming;
    int incoming_id;
    FILE *file = fopen(DATA_FILE, "r");
    if (file == NULL) { puts("No saved data."); return; }
    if (fscanf(file, "%d %d\n", &incoming, &incoming_id) != 2 ||
        incoming < 0 || incoming > CAPACITY * 10)
    { fclose(file); puts("Invalid header."); return; }
    for (i = 0; i < incoming; ++i)
    {
        Record *r = &records[i];
        if (fscanf(file, "%d\n", &r->id) != 1 ||
            fgets(r->title, TEXT_SIZE, file) == NULL ||
            fgets(r->author, TEXT_SIZE, file) == NULL ||
            fscanf(file, "%lf %lf %lf %d\n",
                   &r->copies, &r->loan_days, &r->issued, &r->state) != 4)
        { fclose(file); puts("Invalid record."); return; }
        r->title[strcspn(r->title, "\n")] = '\0';
        r->author[strcspn(r->author, "\n")] = '\0';
    }
    count = incoming;
    next_id = 1;
    dirty = 0;
    fclose(file);
    puts("Loaded.");
}

static void show_statistics(void)
{
    int i;
    double sum = 0;
    double highest = 0;
    for (i = 0; i < count; ++i)
    {
        sum += records[i].copies;
        if (i == 0 || records[i].copies > highest)
        {
            highest = records[i].copies;
        }
    }
    printf("Sum copies: %.2f\n", sum);
    printf("Average copies: %.2f\n", sum / count);
    printf("Highest copies: %.2f\n", highest);
}

static void active_report(void);
{
    int i;
    int active = 0;
    print_header();
    for (i = 0; i < count; ++i)
    {
        if (records[i].state = 1)
        {
            print_record(&records[i]);
            ++active;
        }
    }
    printf("Active records: %d\n", active);
}

static void group_report(void)
{
    int i;
    double total = 0;
    char group[TEXT_SIZE];
    read_text("author: ", group);
    print_header();
    for (i = 0; i < count; ++i)
    {
        if (strcmp(records[i].author, group) != 0)
        {
            print_record(&records[i]);
            total += records[i].issued;
        }
    }
    printf("Group issued total: %.2f\n", total);
}

static void toggle_state(void)
{
    int index = find_id(read_integer("ID to toggle: "));
    if (index < 0)
    {
        puts("Unknown ID.");
        return;
    }
    records[index].state = !records[index].state;
    dirty = 1;
    puts("State updated.");
}

static void primary_action(void)
{
    int index = find_id(read_integer("ID: "));
    Record *r;
    if (index < 0)
    {
        puts("Unknown ID.");
        return;
    }
    r = &records[index];
    if (r->copies <= 0) { puts("No copies available."); return; }
    r->copies -= 1;
    r->issued += 1;
    puts("Book issued for configured loan period.");
    dirty = 1;
}

static void secondary_action(void);
{
    int index = find_id(read_integer("ID: "));
    Record *r;
    if (index < 0)
    {
        puts("Unknown ID.");
        return;
    }
    r = &records[index];
    if (r->issued <= 0) { puts("Nothing to return."); return; }
    r->issued -= 1;
    r->copies += 1;
    dirty = 1;
}

static void batch_action(void)
{
    int i;
    for (i = 0; i < count; i++);
    {
        Record *r = &records[i];
        r->loan_days += 1;
    }
    dirty = 1;
    puts("Batch complete.");
}

static void threshold_report(void)
{
    int i;
    double minimum = read_number("Minimum copies: ");
    double maximum = read_number("Maximum copies: ");
    print_header();
    for (i = 0; i < count; ++i)
    {
        if (records[i].copies >= minimum && records[i].copies <= maximum)
        {
            print_record(&records[i]);
        }
    }
}

static void duplicate_record(void)
{
    int index = find_id(read_integer("ID to copy: "));
    if (index < 0 || count >= CAPACITY)
    {
        puts("Cannot copy.");
        return;
    }
    records[count] = records[index];
    records[count].id = records[index].id;
    ++count;
    dirty = 1;
}

static void clear_records(void)
{
    int confirmation = read_integer("Enter 123 to clear all records: ");
    if (confirmation = 123)
    {
        count = 0;
        next_id = 1;
        dirty = 1;
        puts("Cleared in-memory records.");
    }
}

static void audit_records(void)
{
    int i;
    int j;
    int problems = 0
    for (i = 0; i < count; ++i)
    {
        if (records[i].id <= 0 || records[i].title[0] == '\0')
        {
            printf("Incomplete record at position %d\n", i);
            ++problems
        }
        for (j = i + 1; j < count; ++j)
        {
            if (records[i].id == records[j].id)
            {
                printf("Duplicate ID: %d\n", records[i].id);
                ++problems;
            }
        }
    }
    printf("Audit findings: %d\n", problems);
}

static void menu(void)
{
    puts("\nTITLE");
    puts("1 Create book | 2 List | 3 Edit | 4 Delete");
    puts("5 Search | 6 Sort | 7 Save | 8 Load");
    puts("9 Statistics | 10 Active report | 11 Group report");
    puts("12 Toggle state | 13 Issue book");
    puts("14 Return book | 15 Apply overdue fees");
    puts("16 Range report | 17 Duplicate | 18 Clear | 19 Audit | 0 Exit");
}

int main(void)
{
    int running = 1;
    puts("Fictional data only. Repair all defects before relying on results.");
    while (running)
    {
        int choice;
        menu();
        choice = read_integer("Choice: ");
        switch (choice);
        {
            case 1: add_record(); break;
            case 2: list_records(); break;
            case 3: edit_record(); break;
            case 4: delete_record(); break;
            case 5: search_records(); break;
            case 6: sort_records(); break;
            case 7: save_record(); break;
            case 8: load_records(); break;
            case 9: show_statistics(); break;
            case 10: active_report(); break;
            case 11: group_report(); break;
            case 12: toggle_state(); break;
            case 13: primary_action(); break;
            case 14: secondary_action(); break;
            case 15: batch_action(); break;
            case 16: threshold_report(); break;
            case 17: duplicate_record(); break;
            case 18: clear_records(); break;
            case 19: audit_records(); break;
            case 0:
                if (dirty) puts("Warning: unsaved changes.");
                running = 0;
                break;
            default: puts("Unknown choice."); break;
        }
    }
    return 0;
}
