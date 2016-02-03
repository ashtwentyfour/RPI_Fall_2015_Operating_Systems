Operating Systems - Fall 2015 (Inter-Process Communication)

http://www.cs.rpi.edu/~goldsd/docs/fall2015-csci4210/hw3.pdf

to compile:

- gcc main.c

to run:

- ./a.out input_file.txt

comments:

- the input file is of the format:

  # expression
  <expression>
  
- the program ignores lines that begin with '#' and selects the first line
  that begins with '('

- the program assumes that there are spaces between operands and between operands and operators

- a recursive function is used to process nested expressions. each recursive call creates its own child
  processes


