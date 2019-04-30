s = ''
for i in range(ord('a'), ord('z')):
	s += chr(i)
for i in range(ord('A'), ord('Z')):
	s += chr(i)
for i in range(0, 10):
	s += str(i)

with open('test.txt', 'w') as fw:
	for i in range(2000):
		fw.write(str(i+1))
		fw.write(s)
		fw.write('\n')
