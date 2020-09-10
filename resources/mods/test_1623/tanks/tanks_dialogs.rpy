# test_1623_tanks__

init:
	$ default_decl_at = ["size (1.0, 1.0)"]
	
	image bg test_1623_tanks__loc_0_1 = 'mods/test_1623/images/bg/loc_0_1.jpg'
	image bg test_1623_tanks__loc_2_3_4 = 'mods/test_1623/images/bg/loc_2_3_4.jpg'
	image bg test_1623_tanks__loc_win = 'mods/test_1623/images/bg/loc_win.jpg'
	image bg test_1623_tanks__loc_fail = 'mods/test_1623/images/bg/loc_fail.jpg'
	
	$ default_decl_at = []


label test_1623_tanks__dialogue_0:
	scene bg test_1623_tanks__loc_0_1
	
	window show
	"..."
	jump test_1623_tanks__start_next_level


label test_1623_tanks__dialogue_1:
	scene bg test_1623_tanks__loc_0_1
	
	window show
	"..."
	jump test_1623_tanks__start_next_level


label test_1623_tanks__dialogue_2:
	scene bg test_1623_tanks__loc_2_3_4
	
	$ test_1623_tanks__limit_bullets = False
	
	window show
	"..."
	jump test_1623_tanks__start_next_level


label test_1623_tanks__dialogue_3:
	scene bg test_1623_tanks__loc_2_3_4
	
	window show
	"..."
	jump test_1623_tanks__start_next_level

label test_1623_tanks__dialogue_4:
	scene bg test_1623_tanks__loc_2_3_4
	
	$ test_1623_tanks__limit_moves = False
	
	window show
	"..."
	jump test_1623_tanks__start_next_level


label test_1623_tanks__dialogue_5:
	jump test_1623_tanks__win


label test_1623_tanks__fail:
	scene bg test_1623_tanks__loc_fail
	
	window show
	"..."
	jump test_1623__main


label test_1623_tanks__win:
	scene bg test_1623_tanks__loc_win
	
	window show
	"..."
	jump test_1623__main
