#include <mysql.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

struct knn_data {
    double *values;
    uint64_t count;
    uint64_t capacity;
    uint64_t k;
    double threshold;
};

EXPORT bool outlier_knn_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
    if (args->arg_count != 3) {
        strcpy(message, "outlier_knn() requires exactly three arguments: value, k, threshold");
        return 1;
    }

    if (args->arg_type[0] != REAL_RESULT && args->arg_type[0] != INT_RESULT) {
        strcpy(message, "The first argument must be numeric");
        return 1;
    }
    if (args->arg_type[1] != INT_RESULT) {
        strcpy(message, "The second argument (k) must be an integer");
        return 1;
    }
    if (args->arg_type[2] != REAL_RESULT && args->arg_type[2] != INT_RESULT) {
        strcpy(message, "The third argument (threshold) must be numeric");
        return 1;
    }

    struct knn_data *data = (struct knn_data *)malloc(sizeof(struct knn_data));
    if (data == NULL) {
        strcpy(message, "Could not allocate memory");
        return 1;
    }

    data->count = 0;
    data->capacity = 1024;
    data->values = (double *)malloc(data->capacity * sizeof(double));
    if (data->values == NULL) {
        free(data);
        strcpy(message, "Could not allocate memory for values");
        return 1;
    }

    data->k = *(long long *)args->args[1];
    data->threshold = *(double *)args->args[2];

    initid->ptr = (char *)data;

    return 0;
}

EXPORT void outlier_knn_clear(UDF_INIT *initid, char *is_null, char *error) {
    struct knn_data *data = (struct knn_data *)initid->ptr;
    data->count = 0;
}

EXPORT void outlier_knn_add(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error) {
    struct knn_data *data = (struct knn_data *)initid->ptr;
    double value = *(double *)args->args[0];

    if (data->count >= data->capacity) {
        data->capacity *= 2;
        double *new_values = (double *)realloc(data->values, data->capacity * sizeof(double));
        if (new_values == NULL) {
            *error = 1;
            return;
        }
        data->values = new_values;
    }

    data->values[data->count] = value;
    data->count += 1;
}

int compare_doubles(const void *a, const void *b) {
    double diff = (*(double *)a) - (*(double *)b);
    if (diff < 0.0) return -1;
    else if (diff > 0.0) return 1;
    else return 0;
}

EXPORT long long outlier_knn(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error) {
    struct knn_data *data = (struct knn_data *)initid->ptr;

    if (data->count == 0) {
        *is_null = 1;
        return 0;
    }

    uint64_t i, j;
    double *avg_distances = (double *)malloc(data->count * sizeof(double));
    if (avg_distances == NULL) {
        *error = 1;
        return 0;
    }

    for (i = 0; i < data->count; i++) {
        double distances[data->count - 1];
        uint64_t idx = 0;
        for (j = 0; j < data->count; j++) {
            if (i != j) {
                distances[idx] = fabs(data->values[i] - data->values[j]);
                idx++;
            }
        }

        qsort(distances, data->count - 1, sizeof(double), compare_doubles);

        uint64_t k = data->k;
        if (k > data->count - 1) {
            k = data->count - 1;
        }
        double sum = 0.0;
        for (j = 0; j < k; j++) {
            sum += distances[j];
        }
        avg_distances[i] = sum / k;
    }

    double total_avg_distance = 0.0;
    for (i = 0; i < data->count; i++) {
        total_avg_distance += avg_distances[i];
    }
    total_avg_distance /= data->count;

    uint64_t outlier_count = 0;
    for (i = 0; i < data->count; i++) {
        if (avg_distances[i] > data->threshold * total_avg_distance) {
            outlier_count++;
        }
    }

    free(avg_distances);

    return outlier_count;
}

EXPORT void outlier_knn_deinit(UDF_INIT *initid) {

    if (initid->ptr != NULL) {
        struct knn_data *data = (struct knn_data *)initid->ptr;
        if (data->values != NULL) {
            free(data->values);
        }
        free(data);
        initid->ptr = NULL;
    }
}