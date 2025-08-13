init python:
	all_locations_bg = im.rect('#444')
	
	location_list_bg   = im.rect('#EEE')
	used_location_bg   = im.rect('#0D0')
	unused_location_bg = im.rect('#F60')
	
	location_props_bg        = im.rect('#DDD')
	location_props_toggle_bg = im.rect('#EEE')
	obj_bg                   = im.rect('#B80')
	
	
	selected_prop     = im.composite((100, 20),
	                                 (0, 0), im.rect('#111', 100, 20),
	                                 (3, 3), im.rect('#EEE',  94, 14))
	not_selected_prop = im.composite((100, 20),
	                                 (0, 0), im.rect('#888', 100, 20),
	                                 (3, 3), im.rect('#EEE',  94, 14))
	
	checkbox_no  = im.composite((20, 20),
	                            (0, 0), im.rect('#111', 20, 20),
	                            (2, 2), im.rect('#EEE', 16, 16))
	checkbox_yes = im.composite((20, 20),
	                            (0, 0), im.rect('#111', 20, 20),
	                            (2, 2), im.rect('#EEE', 16, 16),
	                            (4, 4), im.rect('#111', 12, 12))


init:
	style rpg_editor_btn is textbutton:
		ground im.round_rect('#08F', 20, 20, 6)
		hover  im.round_rect('#F80', 20, 20, 6)
		font  'Calibri'
		color '#111'
		text_size 18
	
	style add_location_btn is rpg_editor_btn:
		ground im.round_rect('#0E0', 20, 20, 6)
		hover  im.round_rect('#0F0', 20, 20, 6)
		size (100, 24)
	
	style del_location_btn is add_location_btn:
		ground im.round_rect('#F80', 20, 20, 6)
		hover  im.round_rect('#F90', 20, 20, 6)
	
	
	style unselect_btn is rpg_editor_btn:
		xalign 0.5
		size (200, 40)
		text_size 20
	
	style place_btn is rpg_editor_btn:
		xalign    0.5
		size     (250, 25)
		text_size 22
	
	style place_error_btn is place_btn:
		color       '#F00'
		outlinecolor 0
	
	style rotate_btn is rpg_editor_btn:
		ground not_selected_prop
		hover  selected_prop
		size (80, 22)
		color 0
	
	style prop_btn is rotate_btn:
		size (100, 22)
		text_size 16
	
	style change_prop_btn is rpg_editor_btn:
		size (50, 20)
	
	style bool_btn is rpg_editor_btn:
		ground im.rect('#00000001')
		hover  im.rect('#00000010')
		size (100, 16)
		corner_sizes 0
	style bool_btn_checkbox is image:
		corner_sizes -1
		xpos 4
		size 16
		yalign 0.5
	style bool_btn_text is text:
		color 0
		text_size 14
		xpos style.bool_btn_checkbox.xpos + style.bool_btn_checkbox.xsize + 4
		yalign 0.5
	
	style selected_name is text:
		color '#222'
		text_size 25
		text_align 'center'
		xalign 0.5
		yanchor 1.0
