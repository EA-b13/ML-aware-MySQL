#!/bin/bash

# File and URL variables
FILE="sentiment-analysis-dataset.zip"
URL="https://www.kaggle.com/api/v1/datasets/download/dineshpiyasamara/sentiment-analysis-dataset"

# Check if the file exists
if [ ! -f "$FILE" ]; then
    echo "$FILE not found. Downloading..."
    curl -L -o "$FILE" "$URL"
else
    echo "$FILE already exists. Skipping download."
fi

# Unzip the file
echo "Extracting $FILE..."
unzip -o "$FILE" && echo "Extraction complete."
