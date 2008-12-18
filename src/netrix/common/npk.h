
#ifndef __GNPK_H__
#define __GNPK_H__

#ifdef __cplusplus
extern "C" {
#endif

//extract a specific file entry from a pack file
BOOL beginNpkExtract( fileEntry_t *, TCHAR * );

//ends file entry extraction
void endNpkExtract( TCHAR * );

#ifdef __cplusplus
}
#endif

#endif