#ifndef _RPS_GAME_H_
#define _RPS_GAME_H_

enum COMMAND
{
	SIGN_UP,		// sign up
	QUIT,			// quit
	ROCK,
	PAPER,
	SCISSORS,
	SUICIDE			// game server exit
};

enum RESPONSE
{
	QUIT,			// opponent quit
	WIN,
	LOSE,
	DRAW
};

struct RPSmsg {
	int command;
	int group_num;
};

struct RPSreply {
	int result;
};


#endif

