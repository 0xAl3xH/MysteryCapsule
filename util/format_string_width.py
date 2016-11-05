import textwrap

user_string = raw_input("Type the string that you would like to be reformatted: ")
output = textwrap.fill(user_string, width = 70)
print 
print output.encode('string_escape')
