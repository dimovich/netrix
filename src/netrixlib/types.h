
// common types

#ifndef __TYPES_H__
#define __TYPES_H__

//fileEntry_t flags
#define FILE_PACKED		(1<<0)
#define FILE_SINGLE		(1<<1)


//fileEntry_t
//
typedef struct fileEntry_s {
	DWORD	dwFlags;	//file flags
	DWORD	pathID;		//path id
	DWORD	dwCRC;		//file CRC
	LONG	lSize;		//file size	(used when FILE_PACKED)
	LONG	lOffset;	//file offset within pack (used when FILE_PACKED)
} fileEntry_t;


#endif
