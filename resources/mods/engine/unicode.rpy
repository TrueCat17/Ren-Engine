init -10000 python:
	
	# second and next bytes in UTF-8 start from 10 in binary (0b10xxxxxx)
	def is_first_byte(c):
		c = ord(c)
		return not(c & 128) or bool(c & 64)
	
	def len_unicode(s):
		res = 0
		for c in s:
			if is_first_byte(c):
				res += 1
		return res
