#include <mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

extern "C" {

bool outlier_init(UDF_INIT* initid, UDF_ARGS* args, char* message) {
    if (args->arg_count != 1) {
        strcpy(message, "outlier() requires exactly one argument");
        return 1;
    }
    if (args->arg_type[0] != INT_RESULT && args->arg_type[0] != REAL_RESULT) {
        strcpy(message, "outlier() requires a numeric argument");
        return 1;
    }
    initid->maybe_null = 1;
    return 0;
}

void outlier_deinit(UDF_INIT* initid) {
}

long long outlier(UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* error) {
    if (args->args[0] == NULL) {
        *is_null = 1;
        return 0;
    }
    double x;
    if (args->arg_type[0] == REAL_RESULT) {
        x = *((double*)args->args[0]);
    } else {
        x = (double)(*((long long*)args->args[0]));
    }

    // Logistic regression parameters (from the Python model)
    double w = 0.7784232974353468;
    double b = -10.276012896759532;

    // Compute the score
    double s = w * x + b;

    // Compute the probability
    double p = 1.0 / (1.0 + exp(-s));

    // Return 1 if p > 0.5, else 0
    return (p > 0.5) ? 1 : 0;
}

}