#include "sentiment_analyzer.h"
#include <iostream>

int main() {
    try {
        SentimentAnalyzer analyzer("/path/to/albert_sentiment_model.pt", "/path/to/albert_tokenizer/sentencepiece.model");
        std::string text = "I absolutely love this product!";
        double score = analyzer.analyze(text);
        std::cout << "Sentiment Score (Positive Probability): " << score << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}
