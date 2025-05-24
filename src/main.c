#include <stdio.h>
#include <stdlib.h> // Untuk free()
#include <cjson/CJSON.h> // Header untuk library cJSON

int main(int argc, char** argv) {
    printf("Halo dari C dengan cJSON!\n\n");

    cJSON *root = NULL;
    cJSON *name = NULL;
    cJSON *age = NULL;
    cJSON *isStudent = NULL;
    char *json_string = NULL;

    // Membuat objek JSON root
    root = cJSON_CreateObject();
    if (root == NULL) {
        fprintf(stderr, "Gagal membuat objek JSON root.\n");
        return 1;
    }

    // Menambahkan item ke objek JSON
    name = cJSON_AddStringToObject(root, "nama", "Budi Santoso");
    if (name == NULL) {
        fprintf(stderr, "Gagal menambahkan 'nama'.\n");
        cJSON_Delete(root);
        return 1;
    }

    age = cJSON_AddNumberToObject(root, "usia", 25);
    if (age == NULL) {
        fprintf(stderr, "Gagal menambahkan 'usia'.\n");
        cJSON_Delete(root);
        return 1;
    }

    isStudent = cJSON_AddTrueToObject(root, "mahasiswa");
    if (isStudent == NULL) {
        fprintf(stderr, "Gagal menambahkan 'mahasiswa'.\n");
        cJSON_Delete(root);
        return 1;
    }

    // Mengubah objek JSON menjadi string yang diformat
    json_string = cJSON_Print(root);
    if (json_string == NULL) {
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