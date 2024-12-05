#!/bin/bash

# Check if the correct number of arguments is provided
if [ "$#" -ne 2 ]; then
  echo "Usage: $0 <input_file> <output_file>"
  exit 1
fi

# Input and output file names from arguments
input_file="$1"
output_file="$2"

# Remove rows that contain quoted fields
awk 'BEGIN {FS=","; OFS=","} {
  # Check if any field contains quotes
  has_quotes=0
  for(i=1; i<=NF; i++) {
    if ($i ~ /"/) {
      has_quotes=1
      break
    }
  }

  # Only print rows that do not contain quotes
  if (has_quotes == 0) {
    print $0
  }
}' "$input_file" > "$output_file"

echo "File cleaned and saved as $output_file"
