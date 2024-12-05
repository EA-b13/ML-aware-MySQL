# test_export.py

import torch
from transformers import AlbertTokenizer, AlbertForSequenceClassification

def load_and_test(model_path, tokenizer_path):
    # Load the TorchScript model
    traced_model = torch.jit.load(model_path)
    traced_model.eval()

    # Load tokenizer
    tokenizer = AlbertTokenizer.from_pretrained(tokenizer_path)

    # Prepare input
    example_text = "This is a horrible product!"
    inputs = tokenizer(example_text, return_tensors="pt")

    # Create token_type_ids (all zeros for single sentence)
    token_type_ids = torch.zeros_like(inputs["input_ids"])

    # Create position_ids (optional, if required by the model)
    position_ids = torch.arange(0, inputs["input_ids"].size(1)).unsqueeze(0).expand_as(inputs["input_ids"])

    # Perform inference
    with torch.no_grad():
        outputs = traced_model(input_ids=inputs["input_ids"], attention_mask=inputs["attention_mask"], token_type_ids=token_type_ids, position_ids=position_ids)
        # If model_outputs is a tuple, extract the first element (logits)
        if isinstance(outputs, tuple):
            logits = outputs[0]
        else:
            logits = outputs

        # Apply softmax to logits
        probabilities = torch.softmax(logits, dim=1)
        positive_score = probabilities[0][1].item()
        print(f"Sentiment Score (Positive Probability): {positive_score}")

if __name__ == "__main__":
    model_path = "/Users/eashanbajaj/Documents/Purdue/CS541/CS541Project/albert_sentiment_model.pt"
    tokenizer_path = "/Users/eashanbajaj/Documents/Purdue/CS541/CS541Project/albert_tokenizer"
    load_and_test(model_path, tokenizer_path)