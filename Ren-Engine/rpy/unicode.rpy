init -10000 python:
	
	# second and next bytes in UTF-8 start from 10 in binary (0b10xxxxxx)
	def utf8__is_first_byte(c):
		c = ord(c)
		return not(c & 128) or bool(c & 64)
	
	def utf8__len(s):
		res = 0
		for c in s:
			if utf8.is_first_byte(c):
				res += 1
		return res
	
	def utf8__width(s, text_size):
		return int(text_size / 1.5 * utf8.len(s))
	
	build_object('utf8')
