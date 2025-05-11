init -1000 python:
	
	# partly copy-past from renpy/translation
	
	locales = {
		"ab": "abkhazian",
		"aa": "afar",
		"af": "afrikaans",
		"ak": "akan",
		"sq": "albanian",
		"am": "amharic",
		"ar": "arabic",
		"an": "aragonese",
		"hy": "armenian",
		"as": "assamese",
		"av": "avaric",
		"ae": "avestan",
		"ay": "aymara",
		"az": "azerbaijani",
		"bm": "bambara",
		"ba": "bashkir",
		"eu": "basque",
		"be": "belarusian",
		"bn": "bengali",
		"bh": "bihari",
		"bi": "bislama",
		"bs": "bosnian",
		"br": "breton",
		"bg": "bulgarian",
		"my": "burmese",
		"ca": "catalan",
		"ch": "chamorro",
		"ce": "chechen",
		"ny": "chewa",
		"cv": "chuvash",
		"kw": "cornish",
		"co": "corsican",
		"cr": "cree",
		"hr": "croatian",
		"cs": "czech",
		"da": "danish",
		"dv": "maldivian",
		"nl": "dutch",
		"dz": "dzongkha",
		"en": "english",
		"et": "estonian",
		"ee": "ewe",
		"fo": "faroese",
		"fj": "fijian",
		"fi": "finnish",
		"fr": "french",
		"ff": "fulah",
		"gl": "galician",
		"ka": "georgian",
		"de": "german",
		"el": "greek",
		"gn": "guaran",
		"gu": "gujarati",
		"ht": "haitian",
		"ha": "hausa",
		"he": "hebrew",
		"hz": "herero",
		"hi": "hindi",
		"ho": "hiri_motu",
		"hu": "hungarian",
		"id": "indonesian",
		"ga": "irish",
		"ig": "igbo",
		"ik": "inupiaq",
		"is": "icelandic",
		"it": "italian",
		"iu": "inuktitut",
		"ja": "japanese",
		"jv": "javanese",
		"kl": "greenlandic",
		"kn": "kannada",
		"kr": "kanuri",
		"ks": "kashmiri",
		"kk": "kazakh",
		"km": "khmer",
		"ki": "kikuyu",
		"rw": "kinyarwanda",
		"ky": "kirghiz",
		"kv": "komi",
		"kg": "kongo",
		"ko": "korean",
		"ku": "kurdish",
		"kj": "kuanyama",
		"la": "latin",
		"lb": "luxembourgish",
		"lg": "ganda",
		"li": "limburgan",
		"ln": "lingala",
		"lo": "lao",
		"lt": "lithuanian",
		"lv": "latvian",
		"gv": "manx",
		"mk": "macedonian",
		"mg": "malagasy",
		"ms": "malay",
		"ml": "malayalam",
		"mt": "maltese",
		"mi": "maori",
		"mr": "marathi",
		"mh": "marshallese",
		"mn": "mongolian",
		"na": "nauru",
		"nv": "navaho",
		"ne": "nepali",
		"ng": "ndonga",
		"no": "norwegian",
		"ii": "nuosu",
		"nr": "ndebele",
		"oc": "occitan",
		"oj": "ojibwa",
		"om": "oromo",
		"or": "oriya",
		"os": "ossetian",
		"pa": "panjabi",
		"pi": "pali",
		"fa": "persian",
		"pl": "polish",
		"ps": "pashto",
		"pt": "portuguese",
		"qu": "quechua",
		"rm": "romansh",
		"rn": "rundi",
		"ro": "romanian",
		"ru": "russian",
		"sa": "sanskrit",
		"sc": "sardinian",
		"sd": "sindhi",
		"se": "sami",
		"sm": "samoan",
		"sg": "sango",
		"sr": "serbian",
		"gd": "gaelic",
		"sn": "shona",
		"si": "sinhala",
		"sk": "slovak",
		"sl": "slovene",
		"so": "somali",
		"st": "sotho",
		"es": "spanish",
		"su": "sundanese",
		"sw": "swahili",
		"ss": "swati",
		"sv": "swedish",
		"ta": "tamil",
		"te": "telugu",
		"tg": "tajik",
		"th": "thai",
		"ti": "tigrinya",
		"bo": "tibetan",
		"tk": "turkmen",
		"tl": "tagalog",
		"tn": "tswana",
		"to": "tongan",
		"tr": "turkish",
		"ts": "tsonga",
		"tt": "tatar",
		"tw": "twi",
		"ty": "tahitian",
		"ug": "uighur",
		"uk": "ukrainian",
		"ur": "urdu",
		"uz": "uzbek",
		"ve": "venda",
		"vi": "vietnamese",
		"wa": "walloon",
		"cy": "welsh",
		"wo": "wolof",
		"fy": "frisian",
		"xh": "xhosa",
		"yi": "yiddish",
		"yo": "yoruba",
		"za": "zhuang",
		"zu": "zulu",
		"cn": "simplified_chinese",
		"chs": "simplified_chinese",
		"cht": "traditional_chinese",
		"zh": "traditional_chinese",
		"chinese-simplified": "simplified_chinese",
		"chinese-traditional": "traditional_chinese",
	}
	def detect_user_locale():
		import locale
		locale_name = locale.getlocale()
		if locale_name is not None:
			locale_name = locale_name[0]
		
		if locale_name is None:
			return None, None
		
		normalize = locale.normalize(locale_name)
		if normalize == locale_name:
			language = region = locale_name
		else:
			locale_name = normalize
			if '.' in locale_name:
				locale_name, _ = locale_name.split('.', 1)
			language, region = locale_name.lower().split('_')
		return language, region
	
	def locale_to_language(locale, region):
		lang_name = locales.get(region)
		if lang_name is not None and lang_name in renpy.known_languages():
			return lang_name
		
		if locale in locales.values():
			return locale
		lang_name = locales.get(locale)
		if lang_name is not None and lang_name in renpy.known_languages():
			return lang_name
		
		return None
	
	def _choose_lang():
		lang = config.language
		
		if not lang:
			lang = (os.getenv('RE_LANG') or '').lower() # lang or locale
			
			if '@' in lang:
				lang = lang[:lang.index('@')]
			if '.' in lang:
				lang = lang[:lang.index('.')]
			if '_' in lang:
				params = lang.split('_')
				lang = locale_to_language(params[0], params[1])
		
		if not lang:
			locale, region = detect_user_locale()
			lang = locale_to_language(locale, region)
		
		if not lang:
			lang = config.default_language or 'english'
		
		config.language = lang
		_set_lang(str(lang))
		signals.send('language')

