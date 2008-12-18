
#ifndef __SEQ_H__
#define __SEQ_H__


#ifdef __cplusplus
extern "C" {
#endif


BOOL seqProc();

BOOL seqProcGame( game_t * );

void seqProcGameFig( game_t * );
void seqProcGameBlocks( game_t * );

BOOL seqProcBot();

BOOL seqProcNetIn();

BOOL seqProcNetOut();

void req( game_t *, ULONG );


#ifdef __cplusplus
}
#endif


#endif