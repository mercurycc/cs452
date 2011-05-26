#ifndef _RPS_GAME_H_
#define _RPS_GAME_H_

#define CLIENT_GROUP_COUNT             4

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
	RESULT_QUIT,			// opponent quit
	RESULT_WIN,
	RESULT_LOSE,
	RESULT_DRAW
};

typedef struct RPSmsg {
	int command;
	int group_num;
} RPSmsg;

typedef struct RPSreply {
	int result;
} RPSreply;

/* Server and client entry */
void RPSServer();
void rps_client();

#endif

