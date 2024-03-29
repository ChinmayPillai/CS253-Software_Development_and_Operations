#!/bin/bash

# Check for exactly two arguments
if [ $# -ne 2 ]; then
  echo "Usage: $0 <input_file> <output_file>"
  exit 1
fi

# Check if the input file exists
if [ ! -f "$1" ]; then
  echo "Error: Input file '$1' does not exist."
  exit 1
fi

# Define output file
output_file="$2"

# 1. Unique cities
echo "------------------" > "$output_file"
echo "Unique cities in the given data file: " >> "$output_file"
# sort the entries by city and get unique entries
awk -F ", " 'NR > 1 {print $3}' "$1" | sort | uniq >> "$output_file"
echo "------------------" >> "$output_file"

# 2. Top 3 highest salaries
echo "Details of top 3 individuals with the highest salary: " >> "$output_file"
# sort the entries by salary in decending order and get top 3 entries
awk -F ", " 'NR > 1 {print $4", "$0}' "$1" | sort -nr | head -n 3 | awk -F ", " '{print $2", "$3", "$4", "$5}' >> "$output_file"
echo "------------------" >> "$output_file"

# 3. Average salary per city
echo "Details of average salary of each city: " >> "$output_file"
# calculate the average salary for each city
awk -F ", " 'NR > 1 {cities[$3] += $4; count[$3]++} END {for (city in cities) {printf("City: %s, Salary: %g\n", city, cities[city] / count[city])}}' "$1" >> "$output_file"
echo "------------------" >> "$output_file"

# 4. Individuals with salary above overall average
echo "Details of individuals with a salary above the overall average salary: " >> "$output_file"
# calculate the overall average salary
average=$(awk -F ", " 'NR > 1 {sum+=$4; count++} END {print sum/count}' "$1")
# get the entries with salary above the average
awk -F ", " -v avg="$average" 'NR > 1 && $4 > avg {print $1"  "$2"  "$3"  "$4}' "$1" >> "$output_file"
echo "------------------" >> "$output_file"

echo "Script execution complete. Please check the output file: $output_file"
