// example.cpp
#include "../sentiment_analyzer.h"
#include <iostream>

int main() {
    try {
        // Update these paths according to your setup
        std::string model_path = "../albert_model/albert_sentiment_model.pt";
        std::string tokenizer_model_path = "../albert_tokenizer/spiece.model";

        SentimentAnalyzer analyzer(model_path, tokenizer_model_path);
        std::string text = "I don't like this product!";
        double score = analyzer.analyze(text);
        std::cout << "Sentiment Score (Positive Probability): " << score << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}