
//CRTL interface
//

#undef __N_FILE__
#define __N_FILE__ TEXT( "(netrixlib) libc.c" )

#include "compile.h"
#include <windows.h>
#include <tchar.h>
#include <stdio.h>

#include "libc.h"
#include "common.h"

#ifdef DEBUG_COMPILE
	memory_t * kMemory = NULL;	//memory allocation tracing 
#endif

/*
=====================
	_N_Malloc
=====================
*/
void * _N_Malloc(size_t size) {
	void *pMem = NULL;

	do {
		pMem = HeapAlloc( GetProcessHeap(),
			HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY, size );

		if(pMem==NULL) {
			Sleep(100);
		}
	} while( pMem == NULL );

	return pMem;
}


/*
=====================
	_N_Realloc
=====================
*/
void* _N_Realloc( void * pOrig, size_t size ) {
	void *pMem = NULL;

	if( pOrig == NULL ) {
		return NULL;
	}
		
	do {
		pMem = HeapReAlloc( GetProcessHeap(),
			HEAP_ZERO_MEMORY, pOrig, size );
		if( pMem == NULL ) {
			Sleep( 100 );
		}
	} while( pMem == NULL );

	return pMem;
}


/*
=====================
	_N_Free
=====================
*/
void _N_Free(void *ptr) {
	HeapFree(GetProcessHeap(),0,ptr);
}



#ifdef DEBUG_COMPILE
/*
=====================
	_N_MallocD
=====================
*/
void * _N_MallocD(size_t size, TCHAR *file, int line) {
	void *pMem = NULL;
	memory_t * mem = NULL;
	
	pMem = _N_Malloc( size );

	if( kMemory ) {
		mem = _N_Malloc( sizeof( *mem ) );
		N_Strncpy(mem->file, file, 50 );
		mem->line = line;
		mem->ptr = pMem;
		mem->Next = kMemory;
		kMemory = mem;
	} else {
		kMemory = _N_Malloc( sizeof( memory_t ) );
		N_Strncpy( kMemory->file, file, 50 );
		kMemory->line = line;
		kMemory->ptr = pMem;
		kMemory->Next = NULL;
	}

	return pMem;
}


/*
=====================
	_N_ReallocD
=====================
*/
void* _N_ReallocD( void * pOrig, size_t size, TCHAR *file, int line ) {
	void *pMem = NULL;
	memory_t *mem, *tmp;

	if( pOrig == NULL )
		return NULL;
	
	//remove any previous trace of memory info

	mem = kMemory;
	while( mem!=NULL ) {
		if( mem->ptr == pOrig ) {
			if( mem == kMemory ) {
				kMemory = kMemory->Next;
			} else {
				tmp->Next = mem->Next;
			}
			_N_Free( mem );
			break;
		}
		tmp = mem;
		mem = mem->Next;
	}
	
	//reallocate

	pMem = _N_Realloc( pOrig, size );

	//enter new memory information into the list
	
	if( kMemory ) {
		mem = _N_Malloc( sizeof( *mem ) );
		N_Strncpy(mem->file, file, 50 );
		mem->line = line;
		mem->ptr = pMem;
		mem->Next = kMemory;
		kMemory = mem;
	} else {
		kMemory = _N_Malloc( sizeof( memory_t ) );
		N_Strncpy( kMemory->file, file, 50 );
		kMemory->line = line;
		kMemory->ptr = pMem;
		kMemory->Next = NULL;
	}
	
	return pMem;
}


/*
=====================
	_N_FreeD
=====================
*/
void _N_FreeD(void *ptr) {
	memory_t * mem = NULL;
	memory_t * tmp = NULL;
	
	tmp = mem = kMemory;
	
	while( mem!=NULL ) {
		if( mem->ptr == ptr ) {
			if( mem == kMemory ) {
				kMemory = kMemory->Next;
			} else {
				tmp->Next = mem->Next;
			}
			_N_Free( mem );
			return;
		}
		tmp = mem;
		mem = mem->Next;
	}
	
	//free memory
	HeapFree(GetProcessHeap(),0,ptr);
}

#endif /* !DEBUG_COMPILE */


/*
=====================
	N_Memcpy
=====================
*/
void * N_Memcpy(void *dst, const void *src, size_t count) {
	return memcpy( dst, src, count );
}


/*
=====================
	N_Memset
=====================
*/
void * N_Memset(void *dst, int val, size_t count) {
	return memset( dst, val, count );
}


/*
=====================
	N_Isalpha
=====================
*/
BOOL N_Isalpha( int ch ) {
	if( (ch>=TEXT( 'A' ) && ch<=TEXT( 'Z' ) ) ||
		( ch>=TEXT( 'a' ) && ch<=TEXT( 'z' ) ) )
		return TRUE;
	return FALSE;
}


/*
=====================
	N_Isdigit
=====================
*/
BOOL N_Isdigit( int ch ) {
	if( ch>=TEXT( '0' ) && ch<=TEXT( '9' ) )
		return TRUE;
	return FALSE;
}


/*
=====================
	N_Strncpy
=====================
*/
TCHAR * N_Strncpy( TCHAR *dst, const TCHAR *src, size_t count ) {

	return _tcsncpy( dst, src, count );
}


/*
=====================
	N_Sprintf
=====================
*/
int N_Sprintf(TCHAR* str, size_t count, const TCHAR* format, ...) {
	va_list va;
	int ret;
	
	if( (format == NULL) || (str == NULL) ) {
		return (-1);
	}
	
	va_start(va, format);

	ret = _vsntprintf(str, count, format, va);
	va_end(va); 
	
	return ret;
}


/*
=====================
	N_Strlen
=====================
*/
DWORD N_Strlen(const TCHAR *str) {
	return lstrlen( str );
}


/*
=====================
	N_FOpenR
=====================
*/
HANDLE N_FOpenR( TCHAR * Path ) {

	return CreateFile( Path, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
}


/*
=====================
	N_FOpenW
=====================
*/
HANDLE N_FOpenW( TCHAR * Path ) {

	return CreateFile( Path, GENERIC_WRITE, FILE_SHARE_READ,
		NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
}


/*
=====================
	N_FOpenC
=====================
*/
HANDLE N_FOpenC( TCHAR * Path ) {
	
	return CreateFile( Path, GENERIC_WRITE, FILE_SHARE_READ,
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
}


/*
=====================
	N_FSeek
=====================
*/
DWORD N_FSeek( HANDLE hFile, LONG lPos, DWORD iMode ) {

	return SetFilePointer( hFile, lPos, NULL, iMode);
}


/*
=====================
	N_FWrite
=====================
*/
BOOL N_FWrite( HANDLE hFile, void *pData, DWORD size ) {
	int tmp;
	BOOL res;
	
	res = WriteFile( hFile, pData, size, &tmp, NULL );
	
	return ( res && ( tmp > 0 ) );
}


/*
=====================
	N_FRead
=====================
*/
BOOL N_FRead( HANDLE hFile, void *pData, DWORD size ) {
	int tmp;
	BOOL bRes;
	
	bRes = ReadFile( hFile, pData, size, &tmp, NULL );
	
	return ( bRes && ( tmp > 0 ) );
}


/*
=====================
	N_FClose
=====================
*/
BOOL N_FClose( HANDLE hFile ) {

	return CloseHandle( hFile );
}


/*
=====================
	N_FGetc
=====================
*/
int N_FGetc( HANDLE hFile ) {
	char ch;
	int nr;
	BOOL bRes;
	
	bRes = ReadFile( hFile, &ch, sizeof(char), &nr, NULL );

	if( (0 == nr) || (bRes == FALSE) ) {
		return N_EOF;
	}
	
	return ((unsigned char) ch);
}


/*
=====================
	N_FPutc
=====================
*/
int N_FPutc( HANDLE hFile, char ch ) {
	int tmp;
	BOOL bRes;
	
	bRes = WriteFile( hFile, &ch, sizeof(char), &tmp, NULL );

	if( !bRes || tmp==0 ) {
		return N_EOF;
	}
	
	return ((unsigned char)ch);
}


static int seed;

/*
=====================
	initRandom
=====================
*/
void initRandom() {
	seed = GetTickCount();
}


/*
=====================
	N_Random
=====================
*/
int N_Random(int i) {
	if( i > 0 ) {
		seed = RANDOM_CONST * seed + 1;
		return ((int)(seed >> 16) & 0x7FFF) % i;
	}
	return 0;
}


/*
=====================
	N_Strstr
=====================
*/
TCHAR * N_Strstr( TCHAR * str, TCHAR *substr ) {

	return _tcsstr( str, substr );
}


/*
=====================
	N_Strnicmp
=====================
*/
int N_Strnicmp( const TCHAR *first, const TCHAR *last, size_t count ) {

	return _tcsnicmp( first, last, count );
}


/*
=====================
	N_Strncmp
=====================
*/
int N_Strncmp( const TCHAR *first, const TCHAR *last, size_t count) {
	return _tcsncmp( first, last, count );
}


/*
=====================
	N_Strncat
=====================
*/
TCHAR * N_Strncat( TCHAR *dst, const TCHAR *src, size_t count ) {
	return _tcsncat( dst, src, count );
}


/*
=====================
	N_Strcat
=====================
*/
TCHAR * N_Strcat( TCHAR *dst, const TCHAR *src ) {
	return _tcscat( dst, src );
}


/*
=====================
	N_Strcpy
=====================
*/
TCHAR * N_Strcpy( TCHAR *dst, const TCHAR *src ) {
	return _tcscpy( dst, src );
}


/*
=====================
	N_Strcmp
=====================
*/
int N_Strcmp( TCHAR *str1, TCHAR *str2 ) {
	return _tcscmp( str1, str2 );
}


/*
=====================
	N_CalcHash
=====================
*/
unsigned long N_CalcHash( char *str ) {
	unsigned long hash = 0;
	char *p = str;
	
	while( *p ) {
		hash = ((hash << 7) & (unsigned long)(-1)) | (hash >> 25);
		hash = hash ^ (*p++);
	}
	
	return hash;
}


/*
=====================
	N_CalcHash
=====================
*/
void N_ConvertPath( TCHAR *szPath ) {
	int i;
	
	i = 0;
	while( szPath[i] ) {
		//if( szPath[i] == '/'  )
		;
	}
}