// sentiment_analyzer.cpp
#include "sentiment_analyzer.h"
#include <iostream>

SentimentAnalyzer::SentimentAnalyzer(const std::string& model_path, const std::string& tokenizer_model_path)
    : tokenizer(tokenizer_model_path) {
    try {
        module = torch::jit::load(model_path);
        module.eval();
    }
    catch (const c10::Error& e) {
        std::cerr << "Error loading the TorchScript model: " << model_path << std::endl;
        throw e;
    }
}

double SentimentAnalyzer::analyze(const std::string& text) {
    // Tokenize the input text
    auto tokens = tokenizer.tokenize(text, max_length);
    std::vector<int64_t> input_ids = tokens.first;
    std::vector<int64_t> attention_mask = tokens.second;

    // Convert vectors to tensors
    torch::Tensor input_ids_tensor = torch::from_blob(input_ids.data(), {1, max_length}, torch::kInt64).clone();
    torch::Tensor attention_mask_tensor = torch::from_blob(attention_mask.data(), {1, max_length}, torch::kInt64).clone();

    // Create token_type_ids (all zeros for single-sequence classification)
    torch::Tensor token_type_ids_tensor = torch::zeros({1, max_length}, torch::kInt64);

    // Create position_ids (sequential indices for input tokens)
    torch::Tensor position_ids_tensor = torch::arange(0, max_length, torch::kInt64).unsqueeze(0);

    // Create input tuple
    std::vector<torch::jit::IValue> inputs;
    inputs.emplace_back(input_ids_tensor);
    inputs.emplace_back(attention_mask_tensor);
    inputs.emplace_back(token_type_ids_tensor);
    inputs.emplace_back(position_ids_tensor); // Add position_ids

    // Perform inference
    auto outputs = module.forward(inputs);

    // Extract logits from the model's output
    torch::Tensor logits;
    if (outputs.isTuple()) {
        logits = outputs.toTuple()->elements()[0].toTensor();
    } else {
        logits = outputs.toTensor();
    }

    // Apply softmax to get probabilities
    torch::Tensor probabilities = torch::softmax(logits, 1);

    // Get the probability of the positive class (assuming class 1 is positive)
    double positive_score = probabilities[0][1].item<double>();

    return positive_score;
}
