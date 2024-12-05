// sentiment_analyzer.h
#ifndef SENTIMENT_ANALYZER_H
#define SENTIMENT_ANALYZER_H

#include <torch/script.h>
#include <string>
#include <vector>
#include "tokenizer.h"

class SentimentAnalyzer {
public:
    SentimentAnalyzer(const std::string& model_path, const std::string& tokenizer_model_path);
    double analyze(const std::string& text);

private:
    torch::jit::script::Module module;
    Tokenizer tokenizer;
    int max_length = 128;
};

#endif // SENTIMENT_ANALYZER_H
