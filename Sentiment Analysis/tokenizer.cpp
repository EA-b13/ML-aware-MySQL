// tokenizer.cpp
#include "tokenizer.h"
#include <iostream>

Tokenizer::Tokenizer(const std::string& model_path) {
    auto status = sp.Load(model_path);
    if (!status.ok()) {
        std::cerr << "Error loading SentencePiece model: " << model_path << std::endl;
        throw std::runtime_error("Failed to load SentencePiece model.");
    }
}

std::pair<std::vector<int64_t>, std::vector<int64_t>> Tokenizer::tokenize(const std::string& text, int max_length) {
    std::vector<std::string> pieces;
    sp.Encode(text, &pieces);

    std::vector<int64_t> input_ids;
    input_ids.push_back(sp.PieceToId("<s>")); // Start token

    for (const auto& piece : pieces) {
        input_ids.push_back(sp.PieceToId(piece));
    }

    input_ids.push_back(sp.PieceToId("</s>")); // End token

    // Truncate or pad input_ids
    if (input_ids.size() > static_cast<size_t>(max_length)) {
        input_ids.resize(max_length);
    } else {
        while (input_ids.size() < static_cast<size_t>(max_length)) {
            input_ids.push_back(sp.PieceToId("<pad>"));
        }
    }

    // Attention mask: 1 for tokens, 0 for padding
    std::vector<int64_t> attention_mask(max_length, 0);
    for (size_t i = 0; i < input_ids.size(); ++i) {
        if (input_ids[i] != sp.PieceToId("<pad>")) {
            attention_mask[i] = 1;
        }
    }

    return {input_ids, attention_mask};
}
