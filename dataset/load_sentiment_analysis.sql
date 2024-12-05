-- Step 1: Create the table
CREATE TABLE sentiment_analysis_table (
    id INT,
    label TINYINT,
    tweet TEXT
);

-- Step 2: Load the CSV data
LOAD DATA INFILE '/var/lib/mysql-files/sentiment_analysis_cleaned.csv'  -- Modify directory if you're not using Linux
INTO TABLE sentiment_analysis_table
FIELDS TERMINATED BY ','
OPTIONALLY ENCLOSED BY '"'
LINES TERMINATED BY '\n'
IGNORE 1 LINES 
(id, label, tweet);        -- Map columns in the order of the CSV
