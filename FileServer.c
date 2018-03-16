#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main( int argc, char *argv[] )
{
  int listener, talker;

  struct sockaddr_in server_addr, client_addr ;
  int addrlen;

  int yes;    // for setsockopt() SO_REUSEADDR;
  
  char buf[BUFSIZ];

  FILE* fp;
  
  int nbytes;

  if( argc != 3 ) {
    printf( "Usage : server <filename> <port>\n" );
    printf( "   ex ) server index.html 8000\n" );
    exit(1);
  }

  if( ( fp = fopen( argv[1], "r" ) ) == NULL ) {
    perror( "fopen error" ); exit(1);
  }

  // create a listener socket, IPv4, TCP
  if( ( listener = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 ) {
    perror( "socket error" );
    fclose( fp ); exit(1);
  }
  
  // los the pesky "address already in use" error message
  setsockopt( listener, SOL_SOCKET, SO_REUSEADDR, (void*)&yes, sizeof(int) );

  // server address setting
  // IPv4, addr -> any address for bind(), port number from user input
  memset( &server_addr, 0, sizeof( server_addr ) );
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl( INADDR_ANY );
  server_addr.sin_port = htons( (uint16_t)atoi( argv[2] ) );

  // bind a socket to local IP address and port number
  if( bind( listener, (struct sockaddr*)&server_addr, 
        sizeof( server_addr ) ) < 0 ) {
    perror( "bind error" );
    close( listener ); fclose( fp ); exit(1);
  }

  // wait for connection
  if( listen( listener, 10 ) < 0 ) {
    perror( "listen error" );
    close( listener ); fclose( fp ); exit(1);
  }
  
  if( ( talker = accept(listener, 
          (struct sockaddr*)&client_addr, (socklen_t*)&addrlen ) ) < 0 ) {
    close( listener ); fclose( fp ); exit(1);
  }

  // Send a file name
  write(talker, argv[1], strlen( argv[1] ) + 1 );

  read(talker, buf, BUFSIZ );

  if( strcmp( buf, argv[1] ) == 0 ) {

    // File Transfer
    while( !feof( fp ) ) {
      nbytes = fread( buf, sizeof( char ), BUFSIZ, fp );
      write( talker, buf, nbytes );
    }

  }

  fclose( fp );

  close( talker );
  close( listener );
    
  return 0 ;
}
