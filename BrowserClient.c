# sourcecode

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>


int MakeSocket( char *webAddress, int portNumber );
void RunHTTPInteraction( char *webAddress);
int SendGetReq( int sock, char *HostName, char *ObjectName );
void RecvResponse( int sock, char *FileName, char *HostName );
int ParseObject( char *FileName, char *hostAddress[], char *objectName[] );


#define HTTP_PORT       80
#define BUF_SIZ         1024
#define LARGE_BUF_SIZ   20480
#define TRUE    1
#define FALSE   0


/////////////////////////main

int main( int argc, char** argv )
{
	if( argc == 2){
	RunHTTPInteraction( argv[1]);
	return 0;
	}
	else {	
	printf(" usage : ./gen URL \n");	
	}

return 0;
}




void RunHTTPInteraction( char *webAddress )
{
  int sock;
  
  char *objectName[BUF_SIZ];
  char *hostAddress[BUF_SIZ];
  int objectCounter;

  int id=1;
  
  char FileName[BUF_SIZ];

  int i, j;

  if( ( sock = MakeSocket( webAddress, HTTP_PORT ) ) < 0 ) {
    return;
	}

  sprintf( FileName, "%d.html", id );
  
  if( SendGetReq( sock, webAddress, "/" ) == TRUE ) {
    RecvResponse( sock, FileName, webAddress );
    close( sock );
  }

  else {
    close( sock);
    return;
  }

  objectCounter = ParseObject( FileName, hostAddress, objectName ) ;

  if( objectCounter <= 0 ) return ;
	printf( "=============objects=============\n");
	printf(" the number of objects : %d\n", objectCounter);
	
	for ( i= 0; i<objectCounter; i++){
	printf ( "host : %s  , object : %s\n",hostAddress[i],objectName[i]);
	}

  for( i = 0; i < objectCounter; i++ ) {
    free( hostAddress[i] );
    free( objectName[i] );
  }

}

int SendGetReq( int sock, char *HostName, char *ObjectName )
{
  char GetReq[ BUF_SIZ ];

  sprintf( GetReq, "GET %s HTTP/1.1\r\nAccept: */*\r\nHost: %s:%d\r\nConnection: close\r\n\r\n", ObjectName, HostName, HTTP_PORT );
 
  if ( write( sock, GetReq, strlen(GetReq)+1 ) < 0 ) {
    perror( "write error" ) ;
    return FALSE;
	  }
  return TRUE;
}




/* Make a socket that is connected to a server */
int MakeSocket( char *webAddress, int portNumber ) 
{
  int sock ;                    /* A socket connects to server */
  struct sockaddr_in server_addr ;

  struct hostent *host_name ;   /* server host name */
  unsigned long addr_number ;   /* server address ( network byte order ) */
  uint16_t port ;               /* server port number ( network byte order ) */
  
  /* get the ip address by hostname, also check it is a correct host name */
  host_name = gethostbyname( webAddress ) ;
  if ( host_name == NULL ) {
	       perror( "gethostbyname error" );
    return -1 ;
  }

  addr_number = *(unsigned long*)host_name->h_addr ; /* network byte order */
  port = htons( (uint16_t)portNumber ) ;

  /* create a socket to connet server
   * IPv4, TCP                        */
  if ( ( sock = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 ) {
    perror( "socket error" ) ;
    return -1 ;
  }

  /* server address setting
   * IPv4, server address and port by user inputs ( network byte order ) */
  memset( &server_addr, 0, sizeof( server_addr ) ) ;
  server_addr.sin_family = AF_INET ;
  server_addr.sin_addr.s_addr = addr_number ;
  server_addr.sin_port = port ;

  /* connect to the server
   * If the server does not serve it, it is refused */
  if ( connect( sock, (struct sockaddr*)&server_addr,
        sizeof( server_addr ) ) < 0 ) {
    perror( "connect error" ) ;
    return -1 ;
  }

  /* return the socket number */
  return sock ;
}


void RecvResponse( int sock, char *FileName, char *hostName ) 
{
//  char buf[BUFSIZ+1];
  char buf[BUF_SIZ+1];
  char Temp[BUF_SIZ+1];
  int nbytes;

//  char *p_buf;

  int html;
  
  if ( ( html = open( FileName, O_WRONLY | O_CREAT | O_TRUNC ) ) < 0 ) {
    perror( "open error" ) ;
    return;
  }

  if( ( nbytes = read( sock, buf, BUF_SIZ ) ) < 0 ) {
    printf( "read error!\n" );
    return;
  }

  strcpy( Temp, buf );

  *(strstr( Temp, "\r\n\r\n" )) = '\0';
  
  printf( "=============header==============\n");
  printf( "%s\n", Temp );
	



  while( TRUE ) {
    if ( write( html, buf, nbytes ) < 0 ) {
      perror( "write error" ) ;
      close( html ) ;
      return;
    } 
    if ( ( nbytes = read( sock, buf, BUF_SIZ ) ) == 0 ) break;
  }

  close( html );
}


int ParseObject( char *FileName, char *hostAddress[], char *objectName[] )
{

  int i = 0;
  FILE* response;
  char Temp[LARGE_BUF_SIZ];
  char *pChar_0;
  char *pChar_1;
  char *pChar_2;

  int hostLength;
  int objectLength;
 
  if ( ( response = fopen(FileName,"rb") ) == NULL ) return -1;

  while( fscanf( response, "%s", Temp) != EOF ){

    if( ( pChar_0 = strstr( Temp, "src=\"http://" ) ) != NULL ) {
      pChar_0 += 12;
      
      if( ( pChar_1 = strchr( pChar_0, '/' ) ) != NULL ) {

        if( ( pChar_2 = strchr( pChar_1, '\"' ) ) != NULL ) {

          hostLength = pChar_1 - pChar_0;
          objectLength = pChar_2 - pChar_1;

          hostAddress[i] = (char*)malloc( hostLength + 1 );
          objectName[i] = (char*)malloc( objectLength + 1 );

          strncpy( hostAddress[i], pChar_0, hostLength );
          hostAddress[i][hostLength] = '\0';

          strncpy( objectName[i], pChar_1, objectLength );
          objectName[i][objectLength] = '\0';

          i++;
        }
      
      }
    }
  }

  fclose( response );

  return i;
}

