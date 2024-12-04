# CS541Project

## Introduction

This project integrates a machine learning (ML) outlier detection predicate into MySQL using User-Defined Functions (UDFs). By leveraging logistic regression, we can classify each data point as an outlier or not directly within SQL queries, enhancing the database with ML capabilities.

---

## Overview

The goal is to add an ML predicate, specifically an outlier detector, into SQL syntax. This allows for queries like:

```sql
SELECT *
FROM data_table
WHERE outlier(value) = 1;
```

This UDF leverages a logistic regression model trained to identify outliers in the data. By integrating the model into a UDF, we can apply ML-enhanced predicates within SQL queries, making the database ML-aware.

---

## Machine Learning Model

### Training the Model

The model is trained using a simple logistic regression algorithm from scikit-learn. The training script is located at:

- **File:** `UDF Samples/model_trainer.py`

**Training Data:**

The dataset consists of numerical values where one value is significantly different (an outlier):

```plaintext
Values: [10, 12, 13, 15, 100, 14, 13, 12, 11, 13]
Labels: [ 0,  0,  0,  0,   1,  0,  0,  0,  0,  0]
```

- **Value `100`** is labeled as an outlier (`1`).
- **All other values** are labeled as normal (`0`).

**Training Script (`model_trainer.py`):**

### Model Parameters

After training, the model parameters are:

- **Weight (`w`):** `0.7784232974353468`
- **Bias (`b`):** `-10.276012896759532`

These parameters are used in the UDF to compute the probability of a value being an outlier.

---

## UDF Implementation

The UDF is implemented in C++ and uses the trained model parameters to evaluate each input value.

- **File:** `UDF Samples/outlier_regression.cpp`

### How the UDF Uses the Model

- **Input:** A single numeric value from the database.
- **Process:**
  - Applies the logistic regression formula: \( p = \frac{1}{1 + e^{-(wx + b)}} \)
  - `w` and `b` are the trained model parameters.
- **Output:**
  - Returns `1` if the probability `p` is greater than `0.5` (classified as an outlier).
  - Returns `0` otherwise.

---

## Setup Instructions

### Prerequisites

- **MySQL Server** installed and running.
- **MySQL Development Headers and Libraries** (for compiling UDFs).
- **C++ Compiler** (e.g., `g++`).

### Compiling the UDF

#### On Windows

1. **Open Command Prompt** with administrative privileges.
2. **Navigate** to the directory containing `outlier_regression.cpp`.
3. **Compile** the UDF:

   ```bash
   g++ -shared -o outlier_regression.dll outlier_regression.cpp -I"C:\Program Files\MySQL\<My SQL Server Name>\include" -L"C:\Program Files\MySQL\<My SQL Server Name>\lib" -l libmysql
   ```

   - Adjust the paths to the MySQL `include` and `lib` directories if necessary.

#### On Linux

1. **Open Terminal**.
2. **Navigate** to the directory containing `outlier_regression.cpp`.
3. **Compile** the UDF:

   ```bash
   g++ -shared -fPIC -o outlier_regression.so outlier_regression.cpp $(mysql_config --include --libs)
   ```

### Installing the UDF in MySQL

1. **Copy the compiled library** to the MySQL plugin directory.

   - Find the plugin directory:

   - Copy `outlier_regression.dll` (Windows) or `outlier_regression.so` (Linux) to this directory.

2. **Create the UDF in MySQL:**

   ```sql
   CREATE FUNCTION outlier RETURNS INTEGER SONAME 'outlier_regression.dll';
   ```

   - Use `'outlier_regression.so'` on Linux.

---

## Testing the UDF

1. **Prepare the Data Table:**

   ```sql
   CREATE TABLE data_table (
       id INT PRIMARY KEY,
       value DOUBLE
   );

   INSERT INTO data_table (id, value) VALUES
   (1, 10),
   (2, 12),
   (3, 13),
   (4, 15),
   (5, 100),
   (6, 14),
   (7, 13),
   (8, 12),
   (9, 11),
   (10, 13);
   ```

2. **Test the UDF:**

   ```sql
   SELECT id, value, outlier(value) AS is_outlier
   FROM data_table;
   ```

3. **Use in a `WHERE` Clause:**

   ```sql
   SELECT *
   FROM data_table
   WHERE outlier(value) = 1;
   ```

---

## Usage Example

With the UDF installed, you can incorporate the `outlier` predicate into SQL queries to filter or analyze data.

**Example Query:**

```sql
SELECT id, value
FROM data_table
WHERE outlier(value) = 1;
```

**Explanation:**

- The query selects records where the `value` is classified as an outlier by the ML-enhanced UDF.

---

## Project Alignment

This implementation addresses several key objectives of the project:

1. **ML Predicate Integration:**
   - Added an ML predicate (`outlier`) into SQL syntax.

2. **Porting from ML Library:**
   - The logistic regression model is trained using scikit-learn (an open-source ML library).
   - The model parameters are ported into the UDF.

3. **SQL Syntax Update:**
   - The UDF allows for queries like `WHERE outlier(attribute1) = 1`, integrating ML into SQL syntax.

4. **Execution within Database:**
   - The UDF compiles and executes within MySQL, applying the outlier detection alongside other predicates.

---

## Conclusion

By integrating a logistic regression model into a MySQL UDF, we've enhanced the database with ML capabilities.

---

## References

- **MySQL Documentation:**
  - [Adding New Functions to MySQL](https://dev.mysql.com/doc/refman/8.0/en/adding-functions.html)
  - [User-Defined Functions](https://dev.mysql.com/doc/refman/8.0/en/udf.html)
- **Scikit-learn Documentation:**
  - [Logistic Regression](https://scikit-learn.org/stable/modules/generated/sklearn.linear_model.LogisticRegression.html)
- **C++ Reference:**
  - [Math Functions (`<cmath>`)](https://www.cplusplus.com/reference/cmath/)
- **Project Guidelines:**
  - **ML-aware SQLite Project Description**
