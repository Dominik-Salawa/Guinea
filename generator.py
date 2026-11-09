from random import randint

chars = "_abcdefghijklmnopABCDEFGHIJKLMNOPQRSTUVWXYZ"

def name(length:int) -> str:
	c = ""
	for i in range(length):
		c += chars[randint(0,len(chars)-1)]
	return c

string = ""

for i in range(1000):
	string += f"var {name(10)}: int = 5\n"

file = open("test.gn", "w")
file.write(string)
file.close()

