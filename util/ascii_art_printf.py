### 
# Helper script to convert ASCII art string to a format that can be copy pasted 
# into something like printf
###
filename = raw_input("Pleae enter the file name containing the ASCII text:")

#open file for read only
art = open(filename).readlines();

printf_string = ""

for line in art:
    printf_string += line.encode('string_escape')

print printf_string
