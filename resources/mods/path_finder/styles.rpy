init:
	style path_finder_btn is textbutton:
		xalign 0.5
		ground im.round_rect('#08F', 20, 20, 6)
		hover  im.round_rect('#F80', 20, 20, 6)
		text_size 22
		color       '#EEE'
		hover_color '#111'
	
	style path_finder_to_side_btn is path_finder_btn:
		text_size style.path_finder_to_side_btn.text_size - 2
	
	style path_finder_point_btn is path_finder_btn:
		size 26
		text_size 16
		ground im.round_rect('#04E', 20, 20, 6)
		hover  im.round_rect('#E40', 20, 20, 6)
	
	
	style path_finder_bg_btn is button:
		ground im.rect('#000')
		hover  im.rect('#000')
		corner_sizes 0
		
		size 1.0
		alpha 0.01
		mouse False

init python:
	path_finder_checkbox_yes = im.recolor(gui.checkbox_yes, 0, 0, 0)
	path_finder_checkbox_no  = im.recolor(gui.checkbox_no,  0, 0, 0)
