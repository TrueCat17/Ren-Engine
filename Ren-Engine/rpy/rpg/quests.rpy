init -1000 python:
	
	quests = []
	def get_started_quests():
		return quests
	
	def quest_started(quest):
		return quest in get_started_quests()
	
	def get_quest_name(quest):
		return globals().get(quest + '__name', quest)
	
	def get_quest_description(quest):
		return globals().get(quest + '__description', 'No description.')
	
	def quest_start(quest):
		quests.append(quest)
		if renpy.has_label(quest + '__start'):
			renpy.call(quest + '__start')
	
	def quest_end(quest):
		quests.remove(quest)
		if renpy.has_label(quest + '__end'):
			renpy.call(quest + '__end')
