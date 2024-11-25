# download_albert.py

from transformers import AlbertTokenizer, AlbertForSequenceClassification
import torch

model_name = "textattack/albert-base-v2-SST-2"

# Load tokenizer and model
tokenizer = AlbertTokenizer.from_pretrained(model_name)
model = AlbertForSequenceClassification.from_pretrained(model_name)
model.eval()

# Save tokenizer for later use
tokenizer.save_pretrained("/Users/eashanbajaj/Documents/Purdue/CS541/CS541Project/albert_tokenizer")
model.save_pretrained("/Users/eashanbajaj/Documents/Purdue/CS541/CS541Project/albert_model")