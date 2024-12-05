#include <cmath>
#include <cstdlib>
#include <cstring>
#include <cstdio>

#include "mysql.h"
#include "mysql/udf_registration_types.h"

struct zscore_data {
    unsigned long long count;
    double sum;
    double sumsq;
    double *values;
    unsigned long long capacity;

    char *outlier_str;
    unsigned long outlier_str_len;
    unsigned long outlier_str_capacity;
};

extern "C" bool zscore_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
    if (args->arg_count != 1) {
        strcpy(message, "zscore() requires exactly one argument");
        return 1;
    }
    if (args->arg_type[0] != INT_RESULT && args->arg_type[0] != REAL_RESULT) {
        strcpy(message, "zscore() requires an integer or real argument");
        return 1;
    }

    zscore_data *data = (zscore_data *)malloc(sizeof(zscore_data));
    if (data == NULL) {
        strcpy(message, "Could not allocate memory for zscore_data");
        return 1;
    }

    data->count = 0;
    data->sum = 0.0;
    data->sumsq = 0.0;
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

extern "C" void zscore_deinit(UDF_INIT *initid) {
    zscore_data *data = (zscore_data *)initid->ptr;
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

extern "C" void zscore_clear(UDF_INIT *initid, char *is_null, char *error) {
    zscore_data *data = (zscore_data *)initid->ptr;
    data->count = 0;
    data->sum = 0.0;
    data->sumsq = 0.0;
    data->outlier_str_len = 0;
}

extern "C" void zscore_add(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error) {
    zscore_data *data = (zscore_data *)initid->ptr;
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

    data->sum += val;
    data->sumsq += val * val;
}

extern "C" char *zscore(UDF_INIT *initid, UDF_ARGS *args, char *result,
                        unsigned long *length, char *is_null, char *error) {
    zscore_data *data = (zscore_data *)initid->ptr;

    if (data->count == 0) {
        *is_null = 1;
        return NULL;
    }

    double mean = data->sum / data->count;
    double variance = (data->sumsq / data->count) - (mean * mean);
    double stddev = sqrt(variance);

    if (stddev == 0.0) {
        *is_null = 1;
        return NULL;
    }

    // Identify outliers
    const double threshold = 1.0; // Z-score threshold
    if (data->outlier_str != NULL) {
        free(data->outlier_str);
    }
    data->outlier_str_capacity = 1024;
    data->outlier_str = (char *)malloc(data->outlier_str_capacity);
    if (data->outlier_str == NULL) {
        *error = 1;
        return NULL;
    }
    data->outlier_str[0] = '[';
    data->outlier_str_len = 1;

    for (unsigned long long i = 0; i < data->count; i++) {
        double z = (data->values[i] - mean) / stddev;
        if (fabs(z) > threshold) {
            char buf[64];
            int n = snprintf(buf, sizeof(buf), "%f", data->values[i]);
            if (data->outlier_str_len + n + 2 > data->outlier_str_capacity) {
                data->outlier_str_capacity *= 2;
                char *new_outlier_str = (char *)realloc(data->outlier_str, data->outlier_str_capacity);
                if (new_outlier_str == NULL) {
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

    *length = data->outlier_str_len;
    return data->outlier_str;
}