#ifndef HASH_MAP_H
#define HASH_MAP_H

#include <stddef.h>

// Enum for map operation results
typedef enum {
    MAP_SUCCESS = 0,
    MAP_FAILURE,
    MAP_KEY_NOT_FOUND,
    MAP_ALLOCATION_ERROR
} map_result_t;

// Opaque pointer to the map structure
// This hides the internal details of map_t from users of the header.
typedef struct map_t map_t;

// Function pointer for hashing keys
// Takes a const void* key and returns an unsigned long hash value.
typedef unsigned long (*hash_func_t)(const void *key);

// Function pointer for comparing keys
// Returns 0 if keys are equal, non-zero otherwise.
typedef int (*compare_func_t)(const void *key1, const void *key2);

// Function pointer for freeing key/value memory
// Takes a void* pointer to the data to be freed.
typedef void (*free_func_t)(void *data);

// Function pointer for iterating over map elements
// Takes the key, value, and a user-defined data pointer.
// Returns 0 to continue iteration, non-zero to stop.
typedef int (*map_iter_func_t)(const void *key, void *value, void *user_data);

/**
 * @brief Creates and initializes a new hash map.
 * @param initial_capacity The initial number of buckets. If 0, uses a default.
 * @param hash_func Function to hash keys. MUST NOT be NULL.
 * @param compare_func Function to compare keys. MUST NOT be NULL.
 * @param key_free_func Optional: Function to free key memory when a key is removed or map is destroyed. Can be NULL.
 * @param value_free_func Optional: Function to free value memory when a value is removed or map is destroyed. Can be NULL.
 * @return A pointer to the newly created map, or NULL on error.
 */
map_t *map_create(
    size_t initial_capacity, 
    hash_func_t hash_func, 
    compare_func_t compare_func,
    free_func_t key_free_func, 
    free_func_t value_free_func);

/**
 * @brief Destroys the hash map and frees all associated memory.
 * @param map Pointer to the map to destroy.
 */
void map_destroy(map_t *map);

/**
 * @brief Inserts a key-value pair into the map. If the key already exists, its value is updated.
 * @param map Pointer to the map.
 * @param key Pointer to the key.
 * @param value Pointer to the value.
 * @return MAP_SUCCESS on success, MAP_ALLOCATION_ERROR on resize failure.
 */
map_result_t map_insert(map_t *map, void *key, void *value);

/**
 * @brief Retrieves the value associated with a given key.
 * @param map Pointer to the map.
 * @param key Pointer to the key to search for.
 * @return A pointer to the value if found, or NULL if the key is not found.
 */
void *map_get(const map_t *map, const void *key);

/**
 * @brief Checks if a key exists in the map.
 * @param map Pointer to the map.
 * @param key Pointer to the key to check.
 * @return 1 if the key exists, 0 otherwise.
 */
int map_contains(const map_t *map, const void *key);

/**
 * @brief Deletes a key-value pair from the map.
 * @param map Pointer to the map.
 * @param key Pointer to the key to delete.
 * @return MAP_SUCCESS on successful deletion, MAP_KEY_NOT_FOUND if key not found, MAP_FAILURE on invalid input.
 */
map_result_t map_delete(map_t *map, const void *key);

/**
 * @brief Returns the number of key-value pairs in the map.
 * @param map Pointer to the map.
 * @return The current size of the map.
 */
size_t map_size(const map_t *map);

/**
 * @brief Iterates over all key-value pairs in the map, calling a callback function for each.
 * @param map Pointer to the map.
 * @param callback_func The function to call for each key-value pair.
 * @param user_data An opaque pointer to user-defined data passed to the callback.
 * @return MAP_SUCCESS if iteration completed, MAP_FAILURE if callback stopped it or on invalid input.
 */
map_result_t map_iterate(const map_t *map, map_iter_func_t callback_func, void *user_data);

unsigned long hash_string(const void *key);
int compare_string(const void *key1, const void *key2);
unsigned long hash_int(const void *key);
int compare_int(const void *key1, const void *key2);
void generic_free(void *data);
int print_string_map_element(const void *key, void *value, void *user_data);

#endif // HASH_MAP_H