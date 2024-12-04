#include <mysql.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#ifdef _WIN32
__declspec(dllexport)
#endif
bool is_outlier_zscore_init(UDF_INIT *initid, UDF_ARGS *args, char *message);

#ifdef _WIN32
__declspec(dllexport)
#endif
void is_outlier_zscore_deinit(UDF_INIT *initid);

#ifdef _WIN32
__declspec(dllexport)
#endif
long long is_outlier_zscore(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error);

bool is_outlier_zscore_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
    // No allocation of data needed so don't worry about that
    
    if (args->arg_count != 3) {
        strcpy(message, "is_outlier_zscore() requires exactly three arguments: value, mean, std_dev");
        return 1;
    }

    // Check arguments
    for (int i = 0; i < 3; i++) {
        if (args->arg_type[i] != REAL_RESULT && args->arg_type[i] != INT_RESULT) {
            sprintf(message, "Argument %d must be numeric", i + 1);
            return 1;
        }
    }

    return 0;
}

void is_outlier_zscore_deinit(UDF_INIT *initid) {
    // No deallocation so not really anything to put here
}

long long is_outlier_zscore(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error) {
    double value = *(double *)args->args[0];
    double mean = *(double *)args->args[1];
    double std_dev = *(double *)args->args[2];

    if (std_dev == 0) {
        *error = 1;
        return 0;
    }

    double z_score = fabs((value - mean) / std_dev);

    double threshold = 3.0;

    if (z_score > threshold) {
        return 1;
    } else {
        return 0;
    }
}
