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

struct zscore_data {
    double sum;
    double sum_sq;
    uint64_t count;
    double threshold;
};

EXPORT bool outlier_zscore_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
    if (args->arg_count != 2) {
        strcpy(message, "outlier_zscore() requires exactly two arguments: value, threshold");
        return 1;
    }

    if (args->arg_type[0] != REAL_RESULT && args->arg_type[0] != INT_RESULT) {
        strcpy(message, "The first argument must be numeric");
        return 1;
    }

    if (args->arg_type[1] != REAL_RESULT && args->arg_type[1] != INT_RESULT) {
        strcpy(message, "The second argument must be numeric (threshold)");
        return 1;
    }

    struct zscore_data *data = (struct zscore_data *)malloc(sizeof(struct zscore_data));
    if (data == NULL) {
        strcpy(message, "Could not allocate memory");
        return 1;
    }

    data->sum = 0.0;
    data->sum_sq = 0.0;
    data->count = 0;
    data->threshold = *(double *)args->args[1];

    initid->ptr = (char *)data;

    return 0;
}

EXPORT void outlier_zscore_clear(UDF_INIT *initid, char *is_null, char *error) {
    struct zscore_data *data = (struct zscore_data *)initid->ptr;
    data->sum = 0.0;
    data->sum_sq = 0.0;
    data->count = 0;
}

EXPORT void outlier_zscore_add(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error) {
    struct zscore_data *data = (struct zscore_data *)initid->ptr;
    double value = *(double *)args->args[0];

    data->sum += value;
    data->sum_sq += value * value;
    data->count += 1;
}

EXPORT long long outlier_zscore(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error) {
    struct zscore_data *data = (struct zscore_data *)initid->ptr;

    if (data->count == 0) {
        *is_null = 1;
        return 0;
    }

    double mean = data->sum / data->count;
    double variance = (data->sum_sq / data->count) - (mean * mean);
    double std_dev = sqrt(variance);

    if (std_dev == 0) {
        *is_null = 1;
        return 0;
    }

    double last_value = data->sum - ((data->count - 1) * mean);
    double z_score = fabs((last_value - mean) / std_dev);

    if (z_score > data->threshold) {
        return 1;
    } else {
        return 0;
    }
}

EXPORT void outlier_zscore_deinit(UDF_INIT *initid) {
    if (initid->ptr != NULL) {
        free(initid->ptr);
        initid->ptr = NULL;
    }
}