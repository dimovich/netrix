
#ifndef __COMPRESS_H__
#define __COMPRESS_H__

#ifdef __cplusplus
extern "C" {
#endif

//initiates the compression of a give file
BOOL beginCompress( TCHAR * );

//ends the compression of a given file
void endCompress( TCHAR * );

//initiates a decompression of a given file entry;
//returns the path to the temporary decompressed file
BOOL beginDecompress( TCHAR *, LONG, TCHAR *, int * );

//ends the decompression of a given file
void endDecompress( TCHAR * );

#ifdef __cplusplus
}
#endif

#endif