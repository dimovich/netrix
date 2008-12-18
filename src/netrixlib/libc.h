// CRL substitutes

#ifndef __MLIBC_H__
#define __MLIBC_H__

#ifdef __cplusplus
extern "C" {
#endif

#define N_EOF (-1)
#define RANDOM_CONST 0x015a4e35L

void * _N_Malloc(size_t size);
// allocates some memory

void _N_Free(void *ptr);
// frees some memory

void* _N_Realloc( void *ptr, size_t size );

#ifdef DEBUG_COMPILE

			// Memory
	typedef struct _memory_t {
		TCHAR file[50];			/* source file memory was allocated in*/
		int line;				/* line number in the source file */
		void *ptr;
		struct _memory_t * Next;
	} memory_t;


	void * _N_MallocD(size_t size, TCHAR * file, int line);
	// allocates some memory
	// Debug Version

	void* _N_ReallocD( void * pOrig, size_t size, TCHAR *file, int line );

	void _N_FreeD(void *ptr);
	// frees some memory
	// Debug Version
	
	#define N_Malloc( size ) _N_MallocD( size, __N_FILE__, __LINE__ )
	#define N_Realloc( ptr, size ) _N_ReallocD( ptr, size, __N_FILE__, __LINE__ )
	#define N_Free( ptr ) _N_FreeD( ptr )
	
#else

	#define N_Malloc(size) _N_Malloc(size)
	#define N_Free(ptr) _N_Free(ptr)
	#define N_Realloc(ptr,size) _N_Realloc(ptr,size)
#endif	



//copies memory data from "src" to "dst"
//
void * N_Memcpy(void *dst, const void *src, size_t count);

//set memory to val
//
void * N_Memset(void *dst, int val, size_t count);

//check if character is letter
//
BOOL N_Isalpha( int ch );

//check if character is digit
//
BOOL N_Isdigit( int ch);

//copy string
//
TCHAR * N_Strncpy( TCHAR *dst, const TCHAR *src, size_t count);

//safe string printf
//
int N_Sprintf(TCHAR* str, size_t size, const TCHAR* format, ... );

//get the length of the string
//
DWORD N_Strlen(const TCHAR *str);

//open a file for reading
//
HANDLE N_FOpenR( TCHAR * Path );

//open a file for writing
//
HANDLE N_FOpenW( TCHAR * Path );

//open a file for writing (with CREATE_ALWAYS flag)
//
HANDLE N_FOpenC( TCHAR * Path );

//set the file pointer
//
DWORD N_FSeek( HANDLE hFile, LONG lPos, DWORD iMode );

//write data to file
//
BOOL N_FWrite( HANDLE hFile, void *buff, DWORD size );

//read data from file
//
BOOL N_FRead( HANDLE hFile, void *buff, DWORD size );

//close a file
//
BOOL N_FClose( HANDLE hFile );

//returns the next char from the hFile
//
int N_FGetc( HANDLE hFile );

//writes a character into the file
//
int N_FPutc( HANDLE, char );

// returns the address of a substring
TCHAR * N_Strstr( TCHAR *, TCHAR * );

//non-case-sensitive string comparison
//
int N_Strnicmp( const TCHAR *, const TCHAR *, size_t );

//case sensitive string comparison
//
int N_Strncmp( const TCHAR *first, const TCHAR *last, size_t count);

//safe concatenate two strings
//
TCHAR * N_Strncat( TCHAR *dst, const TCHAR *src, size_t count );

//unsafe concatenate two strings
//
TCHAR * N_Strcat( TCHAR *dst, const TCHAR *src );

//unsafe copy of src to dst
//
TCHAR * N_Strcpy( TCHAR *dst, const TCHAR *src );

//unsafe comparison of two strings
//
int N_Strcmp( TCHAR *str1, TCHAR *str2 );

//init random system
//
void initRandom();

//get a random number
//
int N_Random( int );

//convert all kinds of paths to the only valid one
//
void N_ConvertPath( TCHAR *szPath );

//calculate hash of string
//
unsigned long N_CalcHash( char *str );

#ifdef __cplusplus
}
#endif

#endif