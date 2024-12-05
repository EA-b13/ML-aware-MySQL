#include <cmath>
#include <cstdlib>
#include <cstring>
#include <cstdio>

#include "mysql.h"
#include "mysql/udf_registration_types.h"

struct knn_data {
    unsigned long long count;
    double *values;
    unsigned long long capacity;

    char *outlier_str;
    unsigned long outlier_str_len;
    unsigned long outlier_str_capacity;
};

extern "C" bool knn_outlier_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
    // Check that there is one argument of type REAL or INT
    if (args->arg_count != 1) {
        strcpy(message, "knn_outlier() requires exactly one argument");
        return 1;
    }
    if (args->arg_type[0] != INT_RESULT && args->arg_type[0] != REAL_RESULT) {
        strcpy(message, "knn_outlier() requires an integer or real argument");
        return 1;
    }

    knn_data *data = (knn_data *)malloc(sizeof(knn_data));
    if (data == NULL) {
        strcpy(message, "Could not allocate memory for knn_data");
        return 1;
    }

    data->count = 0;
    data->capacity = 1024;
    data->values = (double *)malloc(sizeof(double) * data->capacity);
    if (data->values == NULL) {
        free(data);
        strcpy(message, "Could not allocate memory for values array");
        return 1;
    }

    data->outlier_str = NULL;
    data->outlier_str_len = 0;
    data->outlier_str_capacity = 0;

    initid->ptr = (char *)data;
    initid->maybe_null = 1;
    initid->decimals = 6;
    initid->max_length = 1024;

    return 0;
}

extern "C" void knn_outlier_deinit(UDF_INIT *initid) {
    knn_data *data = (knn_data *)initid->ptr;
    if (data != NULL) {
        if (data->values != NULL) {
            free(data->values);
        }
        if (data->outlier_str != NULL) {
            free(data->outlier_str);
        }
        free(data);
    }
}

extern "C" void knn_outlier_clear(UDF_INIT *initid, char *is_null, char *error) {
    knn_data *data = (knn_data *)initid->ptr;
    data->count = 0;
    data->outlier_str_len = 0;
}

extern "C" void knn_outlier_add(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error) {
    knn_data *data = (knn_data *)initid->ptr;
    if (args->args[0] == NULL) {
        return;
    }

    double val = 0.0;
    if (args->arg_type[0] == REAL_RESULT) {
        val = *((double *)args->args[0]);
    } else if (args->arg_type[0] == INT_RESULT) {
        val = (double)(*((long long *)args->args[0]));
    } else {
        *error = 1;
        return;
    }

    if (data->count >= data->capacity) {
        data->capacity *= 2;
        double *new_values = (double *)realloc(data->values, sizeof(double) * data->capacity);
        if (new_values == NULL) {
            *error = 1;
            return;
        }
        data->values = new_values;
    }
    data->values[data->count] = val;
    data->count++;
}

extern "C" char *knn_outlier(UDF_INIT *initid, UDF_ARGS *args, char *result,
                             unsigned long *length, char *is_null, char *error) {
    knn_data *data = (knn_data *)initid->ptr;

    if (data->count == 0) {
        *is_null = 1;
        return NULL;
    }

    unsigned int k = 5;
    if (k >= data->count) {
        k = data->count - 1;
    }

    double *avg_distances = (double *)malloc(sizeof(double) * data->count);
    if (avg_distances == NULL) {
        *error = 1;
        return NULL;
    }

    for (unsigned long long i = 0; i < data->count; i++) {
        double *distances = (double *)malloc(sizeof(double) * data->count);
        if (distances == NULL) {
            free(avg_distances);
            *error = 1;
            return NULL;
        }
        for (unsigned long long j = 0; j < data->count; j++) {
            distances[j] = fabs(data->values[i] - data->values[j]);
        }
        for (unsigned long long m = 0; m < data->count - 1; m++) {
            for (unsigned long long n = m + 1; n < data->count; n++) {
                if (distances[m] > distances[n]) {
                    double temp = distances[m];
                    distances[m] = distances[n];
                    distances[n] = temp;
                }
            }
        }
        double sum = 0.0;
        for (unsigned int l = 1; l <= k; l++) {
            sum += distances[l];
        }
        avg_distances[i] = sum / k;

        free(distances);
    }

    double sum = 0.0;
    double sumsq = 0.0;
    for (unsigned long long i = 0; i < data->count; i++) {
        sum += avg_distances[i];
        sumsq += avg_distances[i] * avg_distances[i];
    }
    double mean = sum / data->count;
    double variance = (sumsq / data->count) - (mean * mean);
    double stddev = sqrt(variance);

    if (stddev == 0.0) {
        free(avg_distances);
        *is_null = 1;
        return NULL;
    }

    const double threshold = mean + 2 * stddev; // Threshold for outliers
    if (data->outlier_str != NULL) {
        free(data->outlier_str);
    }
    data->outlier_str_capacity = 1024;
    data->outlier_str = (char *)malloc(data->outlier_str_capacity);
    if (data->outlier_str == NULL) {
        free(avg_distances);
        *error = 1;
        return NULL;
    }
    data->outlier_str[0] = '[';
    data->outlier_str_len = 1;

    for (unsigned long long i = 0; i < data->count; i++) {
        if (avg_distances[i] > threshold) {
            char buf[64];
            int n = snprintf(buf, sizeof(buf), "%f", data->values[i]);
            if (data->outlier_str_len + n + 2 > data->outlier_str_capacity) {
                data->outlier_str_capacity *= 2;
                char *new_outlier_str = (char *)realloc(data->outlier_str, data->outlier_str_capacity);
                if (new_outlier_str == NULL) {
                    free(avg_distances);
                    free(data->outlier_str);
                    data->outlier_str = NULL;
                    *error = 1;
                    return NULL;
                }
                data->outlier_str = new_outlier_str;
            }
            if (data->outlier_str_len > 1) {
                data->outlier_str[data->outlier_str_len++] = ',';
                data->outlier_str[data->outlier_str_len++] = ' ';
            }
            memcpy(data->outlier_str + data->outlier_str_len, buf, n);
            data->outlier_str_len += n;
        }
    }
    data->outlier_str[data->outlier_str_len++] = ']';
    data->outlier_str[data->outlier_str_len] = '\0';

    free(avg_distances);

    *length = data->outlier_str_len;
    return data->outlier_str;
}