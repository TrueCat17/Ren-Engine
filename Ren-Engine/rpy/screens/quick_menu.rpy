init -995 python:
	quick_menu = True
	
	quick_menu_screen = Object()
	quick_menu_screen.items = [
		['History', ShowScreen('history')],
		['Skip',    ToggleDict(db, 'skip_tab')],
		['Save',    ShowScreen('save')],
		['Q.Save',  QuickSave()],
		['Q.Load',  QuickLoad()],
		['Prefs',   ShowScreen('preferences')],
	]

init -995:
	style quick_hbox is hbox:
		align (0.5, 1.0)
	
	style quick_text_button is textbutton:
		ground im.rect('#00000002')
		hover  im.rect('#00000002')
		ysize     20
		text_size 16
		color        0xDDDDDD
		outlinecolor 0x000000


screen quick_menu:
	has hbox
	style 'quick_hbox'
	
	for text, action in quick_menu_screen.items:
		textbutton _(text):
			style 'quick_text_button'
			xsize int(style.quick_text_button.text_size / 1.5 * len_unicode(_(text)))
			action action
