init -100000 python:
	
	def https__get_file(url, local_path, on_end, on_error, clean = False):
		if os.path.exists(local_path) and (clean or os.stat(local_path).st_size < 256):
			os.remove(local_path)
		
		dont_save_https.url = url
		dont_save_https.local_path = local_path
		dont_save_https.on_end = on_end
		dont_save_https.on_error = on_error
		dont_save_https.fails = 0
		dont_save_https.stopped = False
		interruptable_while(https.load_next_part)
	
	def https__close(close_conn = True, stopped = True):
		if close_conn:
			dont_save_https.conn = None
			dont_save_https.prev_domain = ''
		dont_save_https.response = None
		dont_save_https.prev_server_file_path = ''
		dont_save_https.file = None
		dont_save_https.stopped = stopped
	
	
	def https__load_next_part():
		if dont_save_https.get('stopped'):
			return True
		
		if 'exception' in dont_save_https:
			e = dont_save_https.exception
			del dont_save_https.exception
			
			import traceback
			traceback_str = ''.join(traceback.format_tb(e.__traceback__))
			print(e)
			print(traceback_str)
			
			notification.out(_('Error on https.get_files:\n%s') % (e, ))
			if dont_save_https.on_error:
				dont_save_https.on_error()
			https.close()
			return True
		
		if 'local_path' not in dont_save_https: # end or loaded without data
			on_end = dont_save_https.get('on_end')
			if on_end:
				on_end()
			return True
		
		try:
			url = dont_save_https.url
			sep = url.index('/')
			domain = url[:sep]
			server_file_path = url[sep+1:]
			
			if dont_save_https.get('prev_domain') != domain:
				try:
					from tlslite import HTTPTLSConnection
					dont_save_https.conn = HTTPTLSConnection(domain, 443, timeout = 2)
					dont_save_https.prev_domain = domain
					dont_save_https.fails = 0
				except BaseException as e:
					dont_save_https.fails += 1
					if dont_save_https.fails == 1:
						dont_save_https.first_exception = e
					if dont_save_https.fails == 3:
						dont_save_https.exception = dont_save_https.first_exception
				return False
			
			if dont_save_https.get('prev_server_file_path') != server_file_path:
				local_path = dont_save_https.local_path
				size = os.stat(local_path).st_size if os.path.exists(local_path) else 0
				
				try:
					headers = {}
					if size:
						headers['range'] = 'bytes=%s-' % size
					
					from urllib.parse import quote
					quoted_path = quote(server_file_path) # space -> %20, etc
					
					dont_save_https.conn.request('GET', quoted_path, headers = headers)
					dont_save_https.response = dont_save_https.conn.getresponse()
					code = dont_save_https.response.getcode()
					if code >= 400:
						raise BaseException('Got error code %s' % code)
					
					dont_save_https.fails = 0
				except BaseException as e:
					dont_save_https.fails += 1
					if dont_save_https.fails == 1:
						dont_save_https.first_exception = e
					if dont_save_https.fails == 3:
						dont_save_https.exception = dont_save_https.first_exception
					return False
				
				dont_save_https.prev_server_file_path = server_file_path
				
				os.makedirs(os.path.dirname(local_path), exist_ok = True)
				
				range_support = 'content-range' in dont_save_https.response.info()
				mode = 'ab' if range_support else 'wb'
				dont_save_https.file = open(local_path, mode)
				
				dont_save_https.fails = 0
				return False
			
			part = dont_save_https.response.read(256)
			if part:
				dont_save_https.file.write(part)
			else:
				https.close(close_conn = False, stopped = False)
				del dont_save_https.local_path
		
		except BaseException as e:
			dont_save_https.fails += 1
			if dont_save_https.fails == 1:
				dont_save_https.first_exception = e
			if dont_save_https.fails == 3:
				dont_save_https.exception = dont_save_https.first_exception
		
		return False
	
	
	build_object('https')
	dont_save_https = DontSave()
