#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define NUM_SIZE 10000000  // Number of JSON strings to parse

// JSON Value Types
typedef enum {
    JSON_NULL,
    JSON_BOOL,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT
} JsonValueType;

// Forward declarations
typedef struct JsonValue JsonValue;
typedef struct JsonArray JsonArray;
typedef struct JsonObjectEntry JsonObjectEntry;
typedef struct JsonObject JsonObject;

// JSON Value Structure
struct JsonValue {
    JsonValueType type;
    union {
        double number_value;
        char *string_value;
        int bool_value;
        JsonArray *array_value;
        JsonObject *object_value;
    };
};

// JSON Array Structure
struct JsonArray {
    size_t size;
    JsonValue **items;
};

// JSON Object Entry Structure
struct JsonObjectEntry {
    char *key;
    JsonValue *value;
};

// JSON Object Structure
struct JsonObject {
    size_t size;
    JsonObjectEntry **entries;
};

// Function prototypes for JSON parsing
JsonValue* parse_json(const char *json_str);
void free_json_value(JsonValue *value);

// Internal parser state
typedef struct {
    const char *src;
    size_t pos;
} ParserState;

// Function prototypes for internal parsing functions
static void skip_whitespace(ParserState *state);
static int match_char(ParserState *state, char expected);
static char peek_char(ParserState *state);
static char next_char(ParserState *state);
static JsonValue* parse_value(ParserState *state);
static JsonValue* parse_null(ParserState *state);
static JsonValue* parse_bool(ParserState *state);
static JsonValue* parse_number(ParserState *state);
static JsonValue* parse_string(ParserState *state);
static JsonValue* parse_array(ParserState *state);
static JsonValue* parse_object(ParserState *state);

// Skip whitespace characters
static void skip_whitespace(ParserState *state) {
    while (isspace(state->src[state->pos])) {
        state->pos++;
    }
}

// Match a specific character
static int match_char(ParserState *state, char expected) {
    skip_whitespace(state);
    if (state->src[state->pos] == expected) {
        state->pos++;
        return 1;
    }
    return 0;
}

// Peek at the current character without consuming it
static char peek_char(ParserState *state) {
    skip_whitespace(state);
    return state->src[state->pos];
}

// Consume and return the next character
static char next_char(ParserState *state) {
    skip_whitespace(state);
    return state->src[state->pos++];
}

// Parse a JSON value
static JsonValue* parse_value(ParserState *state) {
    char c = peek_char(state);
    if (c == 'n') {
        return parse_null(state);
    } else if (c == 't' || c == 'f') {
        return parse_bool(state);
    } else if (c == '-' || isdigit(c)) {
        return parse_number(state);
    } else if (c == '"') {
        return parse_string(state);
    } else if (c == '[') {
        return parse_array(state);
    } else if (c == '{') {
        return parse_object(state);
    } else {
        return NULL;
    }
}

// Parse a JSON null
static JsonValue* parse_null(ParserState *state) {
    if (strncmp(&state->src[state->pos], "null", 4) == 0) {
        state->pos += 4;
        JsonValue *value = (JsonValue *)malloc(sizeof(JsonValue));
        value->type = JSON_NULL;
        return value;
    }
    return NULL;
}

// Parse a JSON boolean
static JsonValue* parse_bool(ParserState *state) {
    if (strncmp(&state->src[state->pos], "true", 4) == 0) {
        state->pos += 4;
        JsonValue *value = (JsonValue *)malloc(sizeof(JsonValue));
        value->type = JSON_BOOL;
        value->bool_value = 1;
        return value;
    } else if (strncmp(&state->src[state->pos], "false", 5) == 0) {
        state->pos += 5;
        JsonValue *value = (JsonValue *)malloc(sizeof(JsonValue));
        value->type = JSON_BOOL;
        value->bool_value = 0;
        return value;
    }
    return NULL;
}

// Parse a JSON number
static JsonValue* parse_number(ParserState *state) {
    char *end;
    double number = strtod(&state->src[state->pos], &end);
    if (end == &state->src[state->pos]) {
        return NULL;
    }
    state->pos = end - state->src;
    JsonValue *value = (JsonValue *)malloc(sizeof(JsonValue));
    value->type = JSON_NUMBER;
    value->number_value = number;
    return value;
}

// Parse a JSON string
static JsonValue* parse_string(ParserState *state) {
    if (next_char(state) != '"') {
        return NULL;
    }
    size_t start = state->pos;
    while (state->src[state->pos] != '"' && state->src[state->pos] != '\0') {
        if (state->src[state->pos] == '\\') {
            state->pos++;  // Skip escaped character
        }
        state->pos++;
    }
    if (state->src[state->pos] != '"') {
        return NULL;
    }
    size_t length = state->pos - start;
    char *str = (char *)malloc(length + 1);
    memcpy(str, &state->src[start], length);
    str[length] = '\0';
    state->pos++;  // Skip closing quote
    JsonValue *value = (JsonValue *)malloc(sizeof(JsonValue));
    value->type = JSON_STRING;
    value->string_value = str;
    return value;
}

// Parse a JSON array
static JsonValue* parse_array(ParserState *state) {
    if (!match_char(state, '[')) {
        return NULL;
    }
    JsonArray *array = (JsonArray *)malloc(sizeof(JsonArray));
    array->size = 0;
    array->items = NULL;
    JsonValue *value = (JsonValue *)malloc(sizeof(JsonValue));
    value->type = JSON_ARRAY;
    value->array_value = array;

    if (peek_char(state) == ']') {
        state->pos++;
        return value;
    }

    while (1) {
        JsonValue *item = parse_value(state);
        if (item == NULL) {
            free_json_value(value);
            return NULL;
        }
        array->items = (JsonValue **)realloc(array->items, (array->size + 1) * sizeof(JsonValue *));
        array->items[array->size++] = item;

        if (match_char(state, ']')) {
            break;
        }
        if (!match_char(state, ',')) {
            free_json_value(value);
            return NULL;
        }
    }
    return value;
}

// Parse a JSON object
static JsonValue* parse_object(ParserState *state) {
    if (!match_char(state, '{')) {
        return NULL;
    }
    JsonObject *object = (JsonObject *)malloc(sizeof(JsonObject));
    object->size = 0;
    object->entries = NULL;
    JsonValue *value = (JsonValue *)malloc(sizeof(JsonValue));
    value->type = JSON_OBJECT;
    value->object_value = object;

    if (peek_char(state) == '}') {
        state->pos++;
        return value;
    }

    while (1) {
        // Parse key
        JsonValue *key = parse_string(state);
        if (key == NULL || key->type != JSON_STRING) {
            free_json_value(value);
            return NULL;
        }

        if (!match_char(state, ':')) {
            free_json_value(value);
            free_json_value(key);
            return NULL;
        }

        // Parse value
        JsonValue *val = parse_value(state);
        if (val == NULL) {
            free_json_value(value);
            free_json_value(key);
            return NULL;
        }

        JsonObjectEntry *entry = (JsonObjectEntry *)malloc(sizeof(JsonObjectEntry));
        entry->key = key->string_value;
        entry->value = val;
        free(key);  // Key string is now owned by entry

        object->entries = (JsonObjectEntry **)realloc(object->entries, (object->size + 1) * sizeof(JsonObjectEntry *));
        object->entries[object->size++] = entry;

        if (match_char(state, '}')) {
            break;
        }
        if (!match_char(state, ',')) {
            free_json_value(value);
            return NULL;
        }
    }
    return value;
}

// Parse a JSON string
JsonValue* parse_json(const char *json_str) {
    ParserState state = { json_str, 0 };
    JsonValue *value = parse_value(&state);
    skip_whitespace(&state);
    if (value == NULL || state.src[state.pos] != '\0') {
        free_json_value(value);
        return NULL;
    }
    return value;
}

// Free a JSON value
void free_json_value(JsonValue *value) {
    if (value == NULL) {
        return;
    }
    switch (value->type) {
        case JSON_STRING:
            free(value->string_value);
            break;
        case JSON_ARRAY:
            for (size_t i = 0; i < value->array_value->size; i++) {
                free_json_value(value->array_value->items[i]);
            }
            free(value->array_value->items);
            free(value->array_value);
            break;
        case JSON_OBJECT:
            for (size_t i = 0; i < value->object_value->size; i++) {
                free(value->object_value->entries[i]->key);
                free_json_value(value->object_value->entries[i]->value);
                free(value->object_value->entries[i]);
            }
            free(value->object_value->entries);
            free(value->object_value);
            break;
        default:
            break;
    }
    free(value);
}

int main() {
    // Set the seed to 42 for reproducibility
    srand(42);

    // Allocate an array of JSON strings to parse
    char **json_strings = (char **)malloc(NUM_SIZE * sizeof(char *));
    if (json_strings == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    // Generate NUM_SIZE JSON strings
    for (size_t i = 0; i < NUM_SIZE; i++) {
        // For simplicity, generate JSON strings of the form {"number": <random_number>}
        int random_number = rand() % 1000;
        json_strings[i] = (char *)malloc(50 * sizeof(char));
        if (json_strings[i] == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            return 1;
        }
        snprintf(json_strings[i], 50, "{\"number\": %d}", random_number);
    }

    // Benchmark JSON parsing
    clock_t start_time = clock();
    for (size_t i = 0; i < NUM_SIZE; i++) {
        JsonValue *value = parse_json(json_strings[i]);
        // For the purpose of the benchmark, we can ignore the parsed value
        free_json_value(value);
    }
    clock_t end_time = clock();
    double total_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    // Free the JSON strings
    for (size_t i = 0; i < NUM_SIZE; i++) {
        free(json_strings[i]);
    }
    free(json_strings);

    // Print the total time elapsed
    printf("Time Elapsed: %f\n", total_time);

    return 0;
}
