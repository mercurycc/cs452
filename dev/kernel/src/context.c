#include <types.h>
#include <err.h>
#include <devices/console.h>
#include <config.h>
#include <mem.h>
#include <context.h>

int ctx_init( Context* ctx )
{
	ctx->terminal = 0;
	ctx->mem = 0;
	ctx->current_task = 0;
	ctx->next_tid = 0;

	return ERR_NONE;
}

uint ctx_next_tid( Context* ctx )
{
	uint next_tid = ctx->next_tid;
	ctx->next_tid += 1;

	return next_tid;
}

