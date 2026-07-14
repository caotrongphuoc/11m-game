CFLAGS		+= -I./sources/app/game/game_eleven_meter
CPPFLAGS	+= -I./sources/app/game/game_eleven_meter

VPATH += sources/app/game/game_eleven_meter

# CPP source files
SOURCES_CPP += sources/app/game/game_eleven_meter/em_game_match.cpp
SOURCES_CPP += sources/app/game/game_eleven_meter/em_game_shooter.cpp
SOURCES_CPP += sources/app/game/game_eleven_meter/em_game_keeper.cpp
SOURCES_CPP += sources/app/game/game_eleven_meter/em_game_ball.cpp
SOURCES_CPP += sources/app/game/game_eleven_meter/em_game_goal.cpp
