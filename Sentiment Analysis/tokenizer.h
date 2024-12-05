// tokenizer.h
#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <sentencepiece_processor.h>
#include <string>
#include <vector>

class Tokenizer {
public:
    Tokenizer(const std::string& model_path);
    std::pair<std::vector<int64_t>, std::vector<int64_t>> tokenize(const std::string& text, int max_length = 128);

private:
    sentencepiece::SentencePieceProcessor sp;
};

#endif // TOKENIZER_H
