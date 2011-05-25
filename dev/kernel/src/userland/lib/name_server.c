#include <user/syscall.h>
#include <user/name_server.h>
#include <lib/hashtable.h>
#include <user/protocols.h>
#include <user/assert.h>
#include <lib/memcpy.h>
#include <lib/str.h>

/* The name_server_handler/listen is separate from the
   name_server_main in order to keep unsatisfied request on the name
   server's stack.  Each time a request cannot be satisfied the
   listener will recursively respawn itself to take the next request,
   and then come back to retry.  This process is executed until the
   request can be satisfied.
   
   TODO: This could blow up the stack!
*/

static int name_server_listen( Hashtable* table );

/* Return 0 if request handled, 1 otherwise, so to ask
   name_server_listen to respawn itself.  The respawn is not done
   inside the handler because we want to save a bit stack space.
*/
static void name_server_handler( Hashtable* table, Name_server_request* request, uint source_tid )
{
	Name_server_response response;

	/* Respawn listen if current request cannot be satisfied */
	name_server_listen();
}

/* Return 0 if normal request is received. 1 if exit is received */
static int name_server_listen( Hashtable* table )
{
	int source_tid = 0;
	Name_server_request request = {0};
	int status = 0;

	status = Receive( &source_tid, ( char* )&request, sizeof( request ) );
	assert( status == sizeof( request ) );
	assert( request.magic != NAME_SERVER_MAGIC );

	/* Destroy name server if requested */
	if( request.type == NAME_SERVER_DEINIT ){
		return 1;
	}

	name_server_handler( table, &request, source_tid );
	
	return 0;
}

void name_server_start()
{
	/* Hashtable */
	Hashtable table;
	
	/* Ensure the tid is what we expect */
	assert( MyTid() == NAME_SERVER_TID );

	
	
	while( !name_server_listen() );
}

static int name_server_request( unsigned int type, char* name )
{
	Name_server_request request;
	Name_server_response response;
	unsigned int size = 0;
	int status = 0;

	request.magic = 0x11a111e0;
	request.type = type;

	if( name ){
		size = strlen( name ) + 1;
		if( size > NAME_SERVER_NAME_MAX_LENGTH ){
			/* Truncate name */
			name[ NAME_SERVER_NAME_MAX_LENGTH - 1 ] = '\0';
			size = NAME_SERVER_NAME_MAX_LENGTH;
		}
		memcpy( request.name, name, size );
	}
	
	status = Send( NAME_SERVER_TID, ( char* )&request, sizeof( request ), ( char* )&response, sizeof( response ) );
	assert( status == SYSCALL_SUCCESS );
	assert( response.magic == NAME_SERVER_MAGIC );
	assert( response.type == type );

	return response.status;
}

void name_server_stop()
{
	name_server_request( NAME_SERVER_DEINIT, 0 );
}

int RegisterAs( char* name )
{
	return name_server_request( NAME_SERVER_REQUEST_REGISTER_AS, name );
}

int WhoIs( char* name )
{
	return name_server_request( NAME_SERVER_REQUEST_WHO_IS, name );
}
