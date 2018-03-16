#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main( int argc, char *argv[] )
{
  int sock;
  
  struct sockaddr_in server_addr;
  struct hostent *host_name; 

  char buf[BUFSIZ];

  FILE *fp;
  
  int nbytes;
  
  if ( argc != 3 ) {
    printf( "Usage : client <server hostname> <server port>\n" );
    printf( "   ex ) client mnc.korea.ac.kr 8000\n" );
    exit(1);
  }
  
  if( ( host_name = gethostbyname( argv[1] ) ) == NULL ) {
    perror( "gethostbyname error" ); exit(1);
  }
  
  if( ( sock = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 ) {
    perror( "socket error" ); exit(1);
  }
  
  // server address setting
  // IPv4, server address and port from user arguments
  memset( &server_addr, 0, sizeof( server_addr ) );
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = *(unsigned long*)host_name->h_addr;
  server_addr.sin_port = htons( (uint16_t)atoi( argv[2] ) );

  if ( connect( sock, (struct sockaddr*)&server_addr, 
                sizeof( server_addr ) ) < 0 ) {
    perror( "connect error" );
    close( sock ); exit(1);
  }

  nbytes = read( sock, buf, BUFSIZ );

  if( ( fp = fopen( buf, "w" ) ) == NULL ) {
    perror( "fopen error" );
    close( sock ); exit(1);
  }
  
  write( sock, buf, nbytes );

  while( ( nbytes = read( sock, buf, BUFSIZ ) ) > 0 ) {
    fwrite( buf, sizeof(char), nbytes, fp);
  }

  fclose( fp );
  close( sock );

  return 0 ;
}
