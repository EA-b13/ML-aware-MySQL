// udf_sentiment.cpp
#include "sentiment_analyzer.h"
#include <mysql.h>
#include <string>
#include <mutex>

// Initialize a global SentimentAnalyzer instance with thread safety
static SentimentAnalyzer* analyzer = nullptr;
static std::mutex analyzer_mutex;

// UDF initialization function
extern "C" {

bool sentiment_init(UDF_INIT* initid, UDF_ARGS* args, char* message) {
    // Check number of arguments
    if (args->arg_count != 1) {
        strcpy(message, "SENTIMENT() requires exactly one argument");
        return 1;
    }

    // Check argument type
    if (args->arg_type[0] != STRING_RESULT) {
        strcpy(message, "SENTIMENT() requires a string argument");
        return 1;
    }

    // Set the return type
    initid->maybe_null = 1;
    initid->decimals = 4; // Set decimal precision as needed

    // Initialize the SentimentAnalyzer instance
    std::lock_guard<std::mutex> lock(analyzer_mutex);
    if (analyzer == nullptr) {
        try {
            // Update these paths according to your setup
            std::string model_path = "/usr/local/mysql/lib/plugin/albert_sentiment_model.pt";
            std::string tokenizer_model_path = "/usr/local/mysql/lib/plugin/spiece.model";

            analyzer = new SentimentAnalyzer(model_path, tokenizer_model_path);
        }
        catch (...) {
            strcpy(message, "Failed to initialize SentimentAnalyzer");
            return 1;
        }
    }

    return 0;
}

// UDF deinitialization function
void sentiment_deinit(UDF_INIT* initid) {
    std::lock_guard<std::mutex> lock(analyzer_mutex);
    if (analyzer != nullptr) {
        delete analyzer;
        analyzer = nullptr;
    }
}

// UDF function implementation
double sentiment(UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* error) {
    if (args->args[0] == nullptr) {
        *is_null = 1;
        return 0.0;
    }

    std::string text = args->args[0];
    double score = analyzer->analyze(text);
    return score; // Probability of the positive class
}

}