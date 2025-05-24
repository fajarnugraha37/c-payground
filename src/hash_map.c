#include "hash_map.h" 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// This is the full definition of the node, which is an internal detail.
typedef struct map_node_t {
    void *key;
    void *value;
    struct map_node_t *next;
} map_node_t;

// This is the full definition of the map, hidden from users of the header.
struct map_t {
    map_node_t **buckets;      // Array of pointers to linked list heads (buckets)
    size_t capacity;           // Current number of buckets
    size_t size;               // Current number of key-value pairs stored
    hash_func_t hash_func;     // Function to hash keys
    compare_func_t compare_func; // Function to compare keys
    free_func_t key_free_func; // Optional: Function to free key memory
    free_func_t value_free_func; // Optional: Function to free value memory
};

#define INITIAL_CAPACITY 16    // Default initial number of buckets
#define LOAD_FACTOR_THRESHOLD 0.75 // Resize when size/capacity exceeds this
#define RESIZE_FACTOR 2        // Factor by which capacity grows

/**
 * @brief Creates a new map node.
 * @param key Pointer to the key.
 * @param value Pointer to the value.
 * @return A pointer to the newly created node, or NULL on allocation error.
 */
static map_node_t *_map_node_create(void *key, void *value) {
    map_node_t *node = (map_node_t *)malloc(sizeof(map_node_t));
    if (node == NULL) {
        fprintf(stderr, "MAP_ALLOCATION_ERROR: Failed to allocate map node.\n");
        return NULL;
    }
    node->key = key;
    node->value = value;
    node->next = NULL;
    return node;
}

/**
 * @brief Destroys a map node, optionally freeing key and value memory.
 * @param node Pointer to the node to destroy.
 * @param key_free_func Optional function to free key memory.
 * @param value_free_func Optional function to free value memory.
 */
static void _map_node_destroy(map_node_t *node, free_func_t key_free_func, free_func_t value_free_func) {
    if (node == NULL) return;

    if (key_free_func && node->key) {
        key_free_func(node->key);
    }
    if (value_free_func && node->value) {
        value_free_func(node->value);
    }
    free(node);
}

/**
 * @brief Calculates the bucket index for a given key.
 * @param map Pointer to the map.
 * @param key Pointer to the key.
 * @return The calculated bucket index.
 */
static size_t _map_get_bucket_index(const map_t *map, const void *key) {
    // Ensure the hash function is provided
    if (map->hash_func == NULL) {
        fprintf(stderr, "MAP_FAILURE: Hash function not set.\n");
        return 0; // Or handle as an error
    }
    return map->hash_func(key) % map->capacity;
}

/**
 * @brief Resizes the hash map to a new capacity.
 * This involves creating a new, larger array of buckets and rehashing all existing elements.
 * @param map Pointer to the map to resize.
 * @param new_capacity The desired new capacity for the map.
 * @return MAP_SUCCESS on success, MAP_ALLOCATION_ERROR on failure.
 */
static map_result_t _map_resize(map_t *map, size_t new_capacity) {
    if (new_capacity < map->size) {
        fprintf(stderr, "MAP_FAILURE: New capacity is smaller than current size.\n");
        return MAP_FAILURE; // Cannot shrink below current size
    }

    map_node_t **new_buckets = (map_node_t **)calloc(new_capacity, sizeof(map_node_t *));
    if (new_buckets == NULL) {
        fprintf(stderr, "MAP_ALLOCATION_ERROR: Failed to allocate new buckets for resize.\n");
        return MAP_ALLOCATION_ERROR;
    }

    // Store old capacity and buckets
    size_t old_capacity = map->capacity;
    map_node_t **old_buckets = map->buckets;

    // Temporarily update map's capacity and buckets to use the new ones
    // This is crucial for _map_get_bucket_index to work correctly during rehashing
    map->capacity = new_capacity;
    map->buckets = new_buckets;
    map->size = 0; // Reset size, it will be recalculated during reinsertion

    // Rehash all existing nodes into the new buckets
    for (size_t i = 0; i < old_capacity; ++i) {
        map_node_t *current = old_buckets[i];
        while (current != NULL) {
            map_node_t *next_node = current->next; // Save next node before modifying current

            // Calculate new index for the current node
            size_t new_index = _map_get_bucket_index(map, current->key);

            // Add the current node to the head of the linked list in the new bucket
            current->next = map->buckets[new_index];
            map->buckets[new_index] = current;
            map->size++; // Increment size for each reinserted element

            current = next_node;
        }
    }

    // Free the old buckets array (but not the nodes themselves, they've been moved)
    free(old_buckets);

    return MAP_SUCCESS;
}

/**
 * @brief Creates and initializes a new hash map.
 * @param initial_capacity The initial number of buckets. If 0, uses INITIAL_CAPACITY.
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
    free_func_t value_free_func) {
    if (hash_func == NULL || compare_func == NULL) {
        fprintf(stderr, "MAP_FAILURE: Hash function and compare function cannot be NULL.\n");
        return NULL;
    }

    map_t *map = (map_t *)malloc(sizeof(map_t));
    if (map == NULL) {
        fprintf(stderr, "MAP_ALLOCATION_ERROR: Failed to allocate map structure.\n");
        return NULL;
    }

    map->capacity = (initial_capacity > 0) ? initial_capacity : INITIAL_CAPACITY;
    map->size = 0;
    map->hash_func = hash_func;
    map->compare_func = compare_func;
    map->key_free_func = key_free_func;
    map->value_free_func = value_free_func;

    map->buckets = (map_node_t **)calloc(map->capacity, sizeof(map_node_t *));
    if (map->buckets == NULL) {
        fprintf(stderr, "MAP_ALLOCATION_ERROR: Failed to allocate map buckets.\n");
        free(map);
        return NULL;
    }

    return map;
}

/**
 * @brief Destroys the hash map and frees all associated memory.
 * Iterates through all buckets and frees each node and its associated key/value data
 * if free functions are provided.
 * @param map Pointer to the map to destroy.
 */
void map_destroy(map_t *map) {
    if (map == NULL) return;

    for (size_t i = 0; i < map->capacity; ++i) {
        map_node_t *current = map->buckets[i];
        while (current != NULL) {
            map_node_t *next_node = current->next;
            _map_node_destroy(current, map->key_free_func, map->value_free_func);
            current = next_node;
        }
    }
    free(map->buckets);
    free(map);
}

/**
 * @brief Inserts a key-value pair into the map. If the key already exists, its value is updated.
 * Handles automatic resizing if the load factor threshold is exceeded.
 * @param map Pointer to the map.
 * @param key Pointer to the key.
 * @param value Pointer to the value.
 * @return MAP_SUCCESS on success, MAP_ALLOCATION_ERROR on resize failure.
 */
map_result_t map_insert(map_t *map, void *key, void *value) {
    if (map == NULL || key == NULL) {
        return MAP_FAILURE;
    }

    // Check if resize is needed
    if ((double)(map->size + 1) / map->capacity > LOAD_FACTOR_THRESHOLD) {
        map_result_t res = _map_resize(map, map->capacity * RESIZE_FACTOR);
        if (res != MAP_SUCCESS) {
            return res; // Failed to resize
        }
    }

    size_t index = _map_get_bucket_index(map, key);
    map_node_t *current = map->buckets[index];

    // Check if key already exists (update value)
    while (current != NULL) {
        if (map->compare_func(current->key, key) == 0) {
            // Key found, free old value if free_func is set, then update
            if (map->value_free_func && current->value) {
                map->value_free_func(current->value);
            }
            // Free old key if it was dynamically allocated and different from new key
            // This handles cases where a new key with same content is provided
            if (map->key_free_func && current->key != key) {
                map->key_free_func(current->key);
            }
            current->key = key; // Update key pointer (might be the same)
            current->value = value;
            return MAP_SUCCESS;
        }
        current = current->next;
    }

    // Key not found, insert new node at the head of the linked list
    map_node_t *new_node = _map_node_create(key, value);
    if (new_node == NULL) {
        return MAP_ALLOCATION_ERROR;
    }

    new_node->next = map->buckets[index];
    map->buckets[index] = new_node;
    map->size++;

    return MAP_SUCCESS;
}

/**
 * @brief Retrieves the value associated with a given key.
 * @param map Pointer to the map.
 * @param key Pointer to the key to search for.
 * @return A pointer to the value if found, or NULL if the key is not found.
 */
void *map_get(const map_t *map, const void *key) {
    if (map == NULL || key == NULL) {
        return NULL;
    }

    size_t index = _map_get_bucket_index(map, key);
    map_node_t *current = map->buckets[index];

    while (current != NULL) {
        if (map->compare_func(current->key, key) == 0) {
            return current->value; // Key found
        }
        current = current->next;
    }

    return NULL; // Key not found
}

/**
 * @brief Checks if a key exists in the map.
 * @param map Pointer to the map.
 * @param key Pointer to the key to check.
 * @return 1 if the key exists, 0 otherwise.
 */
int map_contains(const map_t *map, const void *key) {
    return map_get(map, key) != NULL;
}

/**
 * @brief Deletes a key-value pair from the map.
 * Optionally frees the key and value memory if free functions are set.
 * @param map Pointer to the map.
 * @param key Pointer to the key to delete.
 * @return MAP_SUCCESS on successful deletion, MAP_KEY_NOT_FOUND if key not found, MAP_FAILURE on invalid input.
 */
map_result_t map_delete(map_t *map, const void *key) {
    if (map == NULL || key == NULL) {
        return MAP_FAILURE;
    }

    size_t index = _map_get_bucket_index(map, key);
    map_node_t *current = map->buckets[index];
    map_node_t *prev = NULL;

    while (current != NULL) {
        if (map->compare_func(current->key, key) == 0) {
            // Key found, remove node from list
            if (prev == NULL) { // Node is the head of the list
                map->buckets[index] = current->next;
            } else {
                prev->next = current->next;
            }
            _map_node_destroy(current, map->key_free_func, map->value_free_func);
            map->size--;
            return MAP_SUCCESS;
        }
        prev = current;
        current = current->next;
    }

    return MAP_KEY_NOT_FOUND; // Key not found
}

/**
 * @brief Returns the number of key-value pairs in the map.
 * @param map Pointer to the map.
 * @return The current size of the map.
 */
size_t map_size(const map_t *map) {
    return (map != NULL) ? map->size : 0;
}

/**
 * @brief Iterates over all key-value pairs in the map, calling a callback function for each.
 * @param map Pointer to the map.
 * @param callback_func The function to call for each key-value pair.
 * Iteration stops if this function returns a non-zero value.
 * @param user_data An opaque pointer to user-defined data passed to the callback.
 * @return MAP_SUCCESS if iteration completed, MAP_FAILURE if callback stopped it or on invalid input.
 */
map_result_t map_iterate(const map_t *map, map_iter_func_t callback_func, void *user_data) {
    if (map == NULL || callback_func == NULL) {
        return MAP_FAILURE;
    }

    for (size_t i = 0; i < map->capacity; ++i) {
        map_node_t *current = map->buckets[i];
        while (current != NULL) {
            if (callback_func(current->key, current->value, user_data) != 0) {
                return MAP_FAILURE; // Callback requested to stop iteration
            }
            current = current->next;
        }
    }
    return MAP_SUCCESS;
}

/**
 * @brief Simple hash function for C-style strings (djb2 algorithm).
 * @param key Pointer to the string key.
 * @return The hash value.
 */
unsigned long hash_string(const void *key) {
    const char *str = (const char *)key;
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash;
}

/**
 * @brief Comparison function for C-style strings.
 * @param key1 Pointer to the first string.
 * @param key2 Pointer to the second string.
 * @return 0 if strings are equal, non-zero otherwise.
 */
int compare_string(const void *key1, const void *key2) {
    return strcmp((const char *)key1, (const char *)key2);
}

/**
 * @brief Hash function for integers.
 * @param key Pointer to an integer key.
 * @return The hash value.
 */
unsigned long hash_int(const void *key) {
    // For simple integers, the integer itself can often serve as a good hash,
    // assuming it's distributed well. Otherwise, more complex algorithms can be used.
    return (unsigned long)(*(const int *)key);
}

/**
 * @brief Comparison function for integers.
 * @param key1 Pointer to the first integer.
 * @param key2 Pointer to the second integer.
 * @return 0 if integers are equal, non-zero otherwise.
 */
int compare_int(const void *key1, const void *key2) {
    return (*(const int *)key1 - *(const int *)key2);
}

/**
 * @brief Generic free function for dynamically allocated data.
 * @param data Pointer to the data to free.
 */
void generic_free(void *data) {
    free(data);
}

// Callback function for map_iterate
int print_string_map_element(const void *key, void *value, void *user_data) {
    printf("  Key: \"%s\", Value: %d\n", (const char *)key, *(int *)value);
    return 0; // Continue iteration
}
