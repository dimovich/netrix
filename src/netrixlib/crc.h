
#ifndef __CRC_H__
#define __CRC_H__

//init crc
void crcInit( unsigned long * );

//update crc for a given data block
void crcUpdate( unsigned long *, const void *, int );

//finish crc calculation
void crcFinish( unsigned long * );

//calculate crc for a given block
unsigned long crcBlock( const void *, int );

#endif