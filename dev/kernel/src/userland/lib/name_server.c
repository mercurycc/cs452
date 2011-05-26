#include <user/syscall.h>
#include <user/name_server.h>
#include <lib/hashtable.h>
#include <user/protocals.h>
#include <user/assert.h>
#include <lib/str.h>
#include <config.h>
#include <err.h>

/* The name_server_handler/listen is separate from the
   name_server_main in order to keep unsatisfied request on the name
   server's stack.  Each time a request cannot be satisfied the
   listener will recursively respawn itself to take the next request,
   and then come back to retry.  This process is executed until the
   request can be satisfied.
   
   TODO: This could blow up the stack!
*/

static int name_server_listen( Hashtable* table );

static void name_server_handler( Hashtable* table, Name_server_request* request, uint source_tid )
{
	Name_server_response response;
	int success = 0;             /* Flag for if the request is satisfied */
	int status = 0;

	response.magic = NAME_SERVER_MAGIC;

	do {
		switch( request->type ){
		case NAME_SERVER_REQUEST_REGISTER_AS:
			status = hashtable_insert( table, request->name, source_tid );
			assert( status ==  0 );
			response.status = REGISTER_AS_SUCCESS;
			success = 1;
			break;
		case NAME_SERVER_REQUEST_WHO_IS:
			status = hashtable_find( table, request->name, &response.status );
			if( status == ERR_NONE ){
				DEBUG_NOTICE( DBG_NS, "Element found\n" );
				success = 1;
			} else if( status == ERR_HASHTABLE_NOTFOUND ){
				success = 0;
			} else {
				assert( 0 );
			}
			break;
		default:
			assert( 0 );
		}
		if( !success ){
			DEBUG_NOTICE( DBG_NS, "did not satisfy request.\n" );
			/* Respawn listen if current request cannot be satisfied */
			status = name_server_listen( table );
			/* We should not receive shutdown signal if not all user program have exited */
			assert( status == 0 );
		}
	} while( ! success );

	response.type = request->type;

	DEBUG_PRINT( DBG_NS, "composed response, type: 0x%x, target: 0x%x, status: 0x%x\n", response.type, source_tid, response.status );

	status = Reply( source_tid, ( char* )&response, sizeof( response ) );
	DEBUG_PRINT( DBG_NS, "reply status = %d\n", status );
	assert( status == 0 );

	DEBUG_NOTICE( DBG_NS, "response sent\n" );
}

/* Return 0 if normal request is received. 1 if exit is received */
static int name_server_listen( Hashtable* table )
{
	int source_tid = 0;
	Name_server_request request = {0};
	int status = 0;

	DEBUG_NOTICE( DBG_NS, "new listener\n" );

	status = Receive( &source_tid, ( char* )&request, sizeof( request ) );
	assert( status == sizeof( request ) );
	assert( request.magic == NAME_SERVER_MAGIC );

	DEBUG_PRINT( DBG_NS, "received request, source: 0x%x, type: 0x%x, name: %s\n", source_tid, request.type, request.name );

	/* Destroy name server if requested */
	if( request.type == NAME_SERVER_DEINIT ){
		DEBUG_NOTICE( DBG_NS, "Received deinit\n" );
		return 1;
	}

	name_server_handler( table, &request, source_tid );
	
	return 0;
}

void name_server_start()
{
	/* Hashtable */
	Hashtable table;
	char key_body[ NAME_SERVER_HASH_ENTRIES ][ NAME_SERVER_NAME_MAX_LENGTH ] = {{0}};
	char* key_table[ NAME_SERVER_HASH_ENTRIES ] = {0};
	ptr elem_table[ NAME_SERVER_HASH_ENTRIES ] = {0};
	unsigned int i = 0;
	unsigned int status = 0;

	DEBUG_NOTICE( DBG_NS, "nameserver begin\n" );

	/* Ensure the tid is what we expect */
	assert( MyTid() == NAME_SERVER_TID );

	for( i = 0; i < NAME_SERVER_HASH_ENTRIES; i += 1 ){
		key_table[ i ] = key_body[ 0 ] + i * NAME_SERVER_NAME_MAX_LENGTH;
	}

	status = hashtable_init( &table, key_table, elem_table, NAME_SERVER_HASH_ENTRIES, NAME_SERVER_NAME_MAX_LENGTH );
	assert( status == 0 );

	DEBUG_NOTICE( DBG_NS, "nameserver init done\n" );
	
	while( !name_server_listen( &table ) );

	Exit();
}

static int name_server_request( unsigned int type, char* name )
{
	Name_server_request request;
	Name_server_response response;
	unsigned int size = 0;
	int status = 0;

	request.magic = NAME_SERVER_MAGIC;
	request.type = type;

	if( name ){
		size = strlen( name ) + 1;
		if( size > NAME_SERVER_NAME_MAX_LENGTH ){
			/* Truncate name */
			name[ NAME_SERVER_NAME_MAX_LENGTH - 1 ] = '\0';
			size = NAME_SERVER_NAME_MAX_LENGTH;
		}
		memcpy( ( uchar* )request.name, ( uchar* )name, size );
	}
	
	status = Send( NAME_SERVER_TID, ( char* )&request, sizeof( request ), ( char* )&response, sizeof( response ) );
	assert( status == sizeof( response ) );
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
