#include <stdio.h>
#include <stdlib.h>      // Untuk free()
#include <string.h> // For strcmp and strlen
#include <cjson/CJSON.h> // Header untuk library cJSON
#include "hello_world.h"
#include "hash_map.h"

int example_of_map();
int example_of_json();

int main(int argc, char **argv)
{
    hello_world();
    example_of_map();
    example_of_json();

    return 0;
}

int example_of_map()
{
    printf("--- String Key HashMap Example ---\n");

    // Create a map with string keys and int values
    // We provide generic_free for both keys and values because we'll dynamically allocate them.
    map_t *string_map = map_create(0, hash_string, compare_string, generic_free, generic_free);
    if (string_map == NULL)
    {
        return 1; // Exit on map creation failure
    }

    // Insert elements
    char *key1 = strdup("apple");
    int *val1 = (int *)malloc(sizeof(int));
    *val1 = 10;
    map_insert(string_map, key1, val1);

    char *key2 = strdup("banana");
    int *val2 = (int *)malloc(sizeof(int));
    *val2 = 20;
    map_insert(string_map, key2, val2);

    char *key3 = strdup("cherry");
    int *val3 = (int *)malloc(sizeof(int));
    *val3 = 30;
    map_insert(string_map, key3, val3);

    char *key4 = strdup("date");
    int *val4 = (int *)malloc(sizeof(int));
    *val4 = 40;
    map_insert(string_map, key4, val4);

    char *key5 = strdup("elderberry");
    int *val5 = (int *)malloc(sizeof(int));
    *val5 = 50;
    map_insert(string_map, key5, val5);

    // Test retrieval
    printf("Map size: %zu\n", map_size(string_map));
    int *retrieved_val = (int *)map_get(string_map, "banana");
    if (retrieved_val)
    {
        printf("Value for 'banana': %d\n", *retrieved_val);
    }
    else
    {
        printf("'banana' not found.\n");
    }

    retrieved_val = (int *)map_get(string_map, "grape");
    if (retrieved_val)
    {
        printf("Value for 'grape': %d\n", *retrieved_val);
    }
    else
    {
        printf("'grape' not found.\n");
    }

    // Test update
    printf("\nUpdating 'apple' value...\n");
    char *key1_update = strdup("apple"); // New key string, same content
    int *val1_update = (int *)malloc(sizeof(int));
    *val1_update = 100;
    map_insert(string_map, key1_update, val1_update); // This will free old key1 and val1, then use key1_update and val1_update

    retrieved_val = (int *)map_get(string_map, "apple");
    if (retrieved_val)
    {
        printf("New value for 'apple': %d\n", *retrieved_val);
    }

    printf("Map size after update: %zu\n", map_size(string_map)); // Size should remain the same

    // Test deletion
    printf("\nDeleting 'cherry'...\n");
    map_result_t delete_res = map_delete(string_map, "cherry");
    if (delete_res == MAP_SUCCESS)
    {
        printf("'cherry' deleted successfully.\n");
    }
    else if (delete_res == MAP_KEY_NOT_FOUND)
    {
        printf("'cherry' not found for deletion.\n");
    }
    printf("Map size after deletion: %zu\n", map_size(string_map));

    retrieved_val = (int *)map_get(string_map, "cherry");
    if (retrieved_val)
    {
        printf("Value for 'cherry' (after delete): %d\n", *retrieved_val);
    }
    else
    {
        printf("'cherry' not found (after delete).\n");
    }

    // Iterate and print all elements
    printf("\nAll elements in map:\n");
    map_iterate(string_map, print_string_map_element, NULL);

    // Destroy the map, freeing all remaining allocated keys and values
    map_destroy(string_map);
    printf("\nString map destroyed.\n");

    printf("\n--- Integer Key HashMap Example ---\n");

    // Create a map with int keys and string values
    // We'll allocate keys and values on the stack for this example, so no free_func
    map_t *int_map = map_create(0, hash_int, compare_int, NULL, NULL);
    if (int_map == NULL)
    {
        return 1;
    }

    int i_key1 = 101;
    char *i_val1 = "Value for 101";
    map_insert(int_map, &i_key1, i_val1);

    int i_key2 = 202;
    char *i_val2 = "Value for 202";
    map_insert(int_map, &i_key2, i_val2);

    int i_key3 = 303;
    char *i_val3 = "Value for 303";
    map_insert(int_map, &i_key3, i_val3);

    printf("Int map size: %zu\n", map_size(int_map));
    char *retrieved_str = (char *)map_get(int_map, &i_key2);
    if (retrieved_str)
    {
        printf("Value for key 202: \"%s\"\n", retrieved_str);
    }
    else
    {
        printf("Key 202 not found.\n");
    }

    // Destroy the map (no custom free functions needed here as data is on stack)
    map_destroy(int_map);
    printf("Integer map destroyed.\n");
    return 0;
}

int example_of_json()
{
    printf("Halo dari C dengan cJSON!\n\n");

    cJSON *root = NULL;
    cJSON *name = NULL;
    cJSON *age = NULL;
    cJSON *isStudent = NULL;
    char *json_string = NULL;

    // Membuat objek JSON root
    root = cJSON_CreateObject();
    if (root == NULL)
    {
        fprintf(stderr, "Gagal membuat objek JSON root.\n");
        return 1;
    }

    // Menambahkan item ke objek JSON
    name = cJSON_AddStringToObject(root, "nama", "Budi Santoso");
    if (name == NULL)
    {
        fprintf(stderr, "Gagal menambahkan 'nama'.\n");
        cJSON_Delete(root);
        return 1;
    }

    age = cJSON_AddNumberToObject(root, "usia", 25);
    if (age == NULL)
    {
        fprintf(stderr, "Gagal menambahkan 'usia'.\n");
        cJSON_Delete(root);
        return 1;
    }

    isStudent = cJSON_AddTrueToObject(root, "mahasiswa");
    if (isStudent == NULL)
    {
        fprintf(stderr, "Gagal menambahkan 'mahasiswa'.\n");
        cJSON_Delete(root);
        return 1;
    }

    // Mengubah objek JSON menjadi string yang diformat
    json_string = cJSON_Print(root);
    if (json_string == NULL)
    {
        fprintf(stderr, "Gagal mencetak JSON ke string.\n");
        cJSON_Delete(root);
        return 1;
    }

    printf("Objek JSON yang dibuat:\n%s\n", json_string);

    // Membebaskan memori
    cJSON_Delete(root); // Ini akan membebaskan semua item yang ditambahkan ke root juga
    free(json_string);  // cJSON_Print mengalokasikan memori yang perlu di-free

    return 0;
}