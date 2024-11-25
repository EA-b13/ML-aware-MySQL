# sentiment_analyzer_wrapper.py

import torch
from transformers import AlbertForSequenceClassification

class SentimentAnalyzerWrapper(torch.nn.Module):
    def __init__(self, model):
        super(SentimentAnalyzerWrapper, self).__init__()
        self.model = model

    def forward(self, input_ids, attention_mask):
        # Ensure inputs_embeds is always None
        return self.model(input_ids=input_ids, attention_mask=attention_mask, inputs_embeds=None)
