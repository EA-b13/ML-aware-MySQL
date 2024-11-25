# tracing_export.py

import torch
from transformers import AlbertTokenizer, AlbertForSequenceClassification
import os

def export_traced_model(model_dir, tokenizer_dir, output_path):
    try:
        # Load tokenizer and model
        tokenizer = AlbertTokenizer.from_pretrained(tokenizer_dir)
        model = AlbertForSequenceClassification.from_pretrained("textattack/albert-base-v2-SST-2")
        model.eval()

        # Example input for tracing
        example_text = "I love this product!"
        inputs = tokenizer(example_text, return_tensors="pt")

        # Define a tracing function
        def forward_fn(input_ids, attention_mask):
            return model(input_ids=input_ids, attention_mask=attention_mask)

        # Trace the model
        traced_model = torch.jit.trace(
            forward_fn,
            (inputs["input_ids"], inputs["attention_mask"])
        )

        # Save the traced model
        traced_model.save(output_path)
        torch.jit.save(traced_model, output_path)
        print(f"TorchScript model successfully saved to: {output_path}")

        loaded_model = torch.jit.load("albert_sentiment_model.pt")
        loaded_model.eval()

    except Exception as e:
        print(f"An error occurred during export: {e}")

if __name__ == "__main__":
    # Define absolute paths
    project_dir = os.path.expanduser("~/Documents/Purdue/CS541/CS541Project")
    model_directory = os.path.join(project_dir, "albert_model")
    tokenizer_directory = os.path.join(project_dir, "albert_tokenizer")
    output_model_path = os.path.join(project_dir, "albert_sentiment_model.pt")

    # Export the traced model
    export_traced_model(model_directory, tokenizer_directory, output_model_path)
