/*
	NOTE:
	~~~~~~
	This is a straightforward implementation of Adaptive Huffman
	algorithm. The ranks are implicitly defined by a node's position
	in the node array. The binary tree is simulated by an ordinary array.
	
	code parts adapted from:

	"The Data Compression Book" by Mark Nelson;
	Quake3 source code by id Software;
*/

#undef __N_FILE__
#define __N_FILE__ TEXT( "compress.c" )


#include "compile.h"
#include <windows.h>

#include "types.h"
#include "libc.h"
#include "const.h"


#define MAX_WEIGHT 0x8000
#define SYMBOL_COUNT 257	//max char + 1 (NYT)
#define NODE_TABLE_COUNT 513 //SYMBOL_COUNT * 2 - 1
#define ROOT_NODE 0
#define NYT 256


typedef struct {
	int leaf[ SYMBOL_COUNT ];
	int next_free_node;

	struct node {
		int weight;
		int parent;
		int child_is_leaf;
		int child;
	} nodes[ NODE_TABLE_COUNT ];
} huffTree_t;


static int out_block;
static unsigned char out_pack;
/*
=====================
	put_bit
=====================
*/
static void put_bit( int v, HANDLE hFileOut ) {

	if( (out_block == 8) || (v == -1) ) {

		//write packet
		N_FPutc( hFileOut, out_pack );

		out_pack = 0;
		out_block = 0;

		if( v == -1 ) {
			return;
		}
	}
	
	out_pack |= ( v & 0x1 ) << out_block;
	out_block++;
}


/*
=====================
	putBits
=====================
*/
static void putBits( int v, int size, HANDLE hFileOut ) {
	int i;

	for( i=size-1; i>=0; i-- ) {
		put_bit( (v >> i) & 0x1, hFileOut );
	}
}


static int in_pack;
static int in_block;
/*
=====================
	get_bit
=====================
*/
static void get_bit( int *v, HANDLE hFileIn ) {

	if( v != NULL ) {

		if( in_block == 0 ) {
			in_pack = N_FGetc( hFileIn );
			in_block = 8;
		}
		
		*v = ( ((unsigned int)in_pack) >> ( 8 - in_block ) ) & 0x1;
		in_block--;
	}
}


/*
=====================
	getBits
=====================
*/
static void getBits( int *v, int size, HANDLE hFileIn ) {
	int i;
	int tmp, tmp2;
	
	if( v != NULL ) {

		tmp2 = 0;
		for( i=size-1; i>=0; i-- ) {
			get_bit( &tmp, hFileIn );
			tmp2 |= (tmp << i);
		}
		*v = tmp2;
	}
}


/*
=====================
	initTree
=====================
*/
static void initTree( huffTree_t *tree ) {
	int i;

	//check arguments validity
	if( tree == NULL ) {
		return;
	}

	//init NYT
	tree->nodes[ROOT_NODE].child = NYT;
	tree->nodes[ROOT_NODE].child_is_leaf = TRUE;
	tree->nodes[ROOT_NODE].weight = 1;
	tree->nodes[ROOT_NODE].parent = -1;

	tree->leaf[NYT] = ROOT_NODE;

	tree->next_free_node = ROOT_NODE + 1;
	
	for( i=0; i<NYT; i++ )
		tree->leaf[i] = -1;
}


/*
=====================
	addNode
=====================
*/
static void addNode( huffTree_t *tree, unsigned char ch ) {
	int lightest_node;
	int new_node;
	int zero_weight_node;
	
	//check arguments validity
	if( tree == NULL ) {
		return;
	}

	//spawn 2 new nodes (leafs) from the lightest node (leaf),
	//and transform it into a parent node

	lightest_node = tree->next_free_node - 1;
	new_node = tree->next_free_node;
	zero_weight_node = tree->next_free_node + 1;
	tree->next_free_node += 2;

	//move old leaf, to a new one

	tree->nodes[new_node] = tree->nodes[lightest_node];
	tree->nodes[new_node].parent = lightest_node;
	tree->leaf[ tree->nodes[new_node].child ] = new_node;

	tree->nodes[lightest_node].child = new_node;
	tree->nodes[lightest_node].child_is_leaf = FALSE;
	
	//init new added leaf
	
	tree->nodes[zero_weight_node].child = ch;
	tree->nodes[zero_weight_node].child_is_leaf = TRUE;
	tree->nodes[zero_weight_node].parent = lightest_node;
	tree->nodes[zero_weight_node].weight = 0;
	tree->leaf[ch] = zero_weight_node;
}


/*
=====================
	encodeSymbol
=====================
*/
static void encodeSymbol( huffTree_t *tree, unsigned char ch, HANDLE hFileOut ) {
	int code = 0;
	unsigned int current_bit = 1u;
	int code_size = 0;
	int current_node;
	
	//check arguments validity
	if( tree == NULL ) {
		return;
	}
	
	current_node = tree->leaf[ch];
	
	if( current_node == -1 ) {	//not yet transmited
		current_node = tree->leaf[NYT];
	}

	if( tree->leaf[NYT] != ROOT_NODE ) { //avoid empty tree
		//calculate code
		while( current_node != ROOT_NODE ) {
			if( ( current_node & 1 ) == 0 ) { //even
				code |= current_bit;
			}
			current_bit <<= 1;
			code_size++;
			current_node = tree->nodes[current_node].parent;
		}
		putBits( code, code_size, hFileOut );
	}

	if( tree->leaf[ch] == -1 ) {
		putBits( ch, 8, hFileOut );
		addNode( tree, ch );
	}
}


/*
=====================
	swapNodes
=====================
*/
static void swapNodes( huffTree_t *tree, int node1, int node2 ) {
	struct node temp;
	
	//check arguments validity
	if( tree == NULL ) {
		return;
	}
	
	//re-set relationships

	if( tree->nodes[node1].child_is_leaf ) {
		tree->leaf[ tree->nodes[ node1 ].child ] = node2;
	}
	else {
		tree->nodes[ tree->nodes[ node1 ].child ].parent = node2;
		tree->nodes[ tree->nodes[ node1 ].child + 1].parent = node2;
	}
	
	if( tree->nodes[node2].child_is_leaf ) {
		tree->leaf[ tree->nodes[node2].child ] = node1;
	}
	else {
		tree->nodes[ tree->nodes[ node2 ].child ].parent = node1;
		tree->nodes[ tree->nodes[ node2 ].child + 1 ].parent = node1;
	}
	
	//finally swap the nodes

	temp = tree->nodes[node1];
	tree->nodes[node1] = tree->nodes[node2];
	tree->nodes[node1].parent = temp.parent;
	temp.parent = tree->nodes[node2].parent;
	tree->nodes[node2] = temp;
}


/*
=====================
	rebuildTree
=====================
*/
static void rebuildTree( huffTree_t *tree ) {
	int i, j, k, weight;
	
	//check arguments validity
	if( tree == NULL ) {
		return;
	}
	
	//align all leafs at the end
	//and divide their weight by 2

	j = tree->next_free_node - 1;
	for( i=j; i>=ROOT_NODE; i-- ) {
		if( tree->nodes[i].child_is_leaf ) {
			tree->nodes[j] = tree->nodes[i];
			tree->nodes[j].weight = ( tree->nodes[j].weight + 1 ) / 2;
			j--;
		}
	}
	
	//rebuild the tree (internal nodes)
	//j: the last empty node before the first leaf
	
	for( i = tree->next_free_node - 2; j>=ROOT_NODE; i-=2, j-- ) {

		//calculate parent weight
		tree->nodes[ j ].weight =
		weight = tree->nodes[i].weight + tree->nodes[i+1].weight;
		
		//find a proper place for our newly created
		//internal node
		for( k=j+1; tree->nodes[k].weight > weight; k++ )
			/*null block*/;
		
		k--;
		
		//shift all nodes so that there is room for our newly created
		//internal node
		memmove( &tree->nodes[j], &tree->nodes[j+1], (k-j)*sizeof( struct node ) );
		
		//set-up our new internal node

		tree->nodes[k].weight = weight;
		tree->nodes[k].child = i;
		tree->nodes[k].child_is_leaf = FALSE;
	}

	//set-up parent relationships, and leaf array
	
	for( i=tree->next_free_node-1; i>=ROOT_NODE; i-- ) {

		k = tree->nodes[i].child;

		if( tree->nodes[i].child_is_leaf ) {
			tree->leaf[k] = i;
		}
		else {
			tree->nodes[k].parent = tree->nodes[k+1].parent = i;
		}
	}
}


/*
=====================
	updateTree
=====================
*/
static void updateTree( huffTree_t *tree, unsigned char ch ) {
	int current_node;
	int new_node;

	//check arguments validity
	if( tree == NULL ) {
		return;
	}

	if( tree->nodes[ROOT_NODE].weight >= MAX_WEIGHT ) {
		rebuildTree( tree );
	}
	
	current_node = tree->leaf[ch];
	while( current_node != -1 ) {
	
		//update weight
		tree->nodes[current_node].weight++;

		//re-place the node
		for( new_node = current_node; new_node > ROOT_NODE; new_node-- ) {
			//check if weight is bigger than sibling's,
			if( tree->nodes[new_node-1].weight >= tree->nodes[current_node].weight ) {
				break;
			}
		}

		//if weight is bigger than sibling's,
		//then swap these 2 nodes
		if( current_node != new_node ) {
			swapNodes( tree, current_node, new_node );
			current_node = new_node;
		}

		current_node = tree->nodes[current_node].parent;
	}
}


/*
=====================
	decodeSymbol
=====================
*/
static int decodeSymbol( huffTree_t *tree, HANDLE hFileIn ) {
	int current_node;
	int ich;
	int v;
	
	//check arguments validity
	if( tree == NULL ) {
		return 0;
	}
	
	current_node = ROOT_NODE;
	while( ! tree->nodes[current_node].child_is_leaf ) {
		current_node = tree->nodes[current_node].child;
		getBits( &v, 1, hFileIn );
		current_node += v & 0x1;
	}
	
	ich = tree->nodes[current_node].child;
	if( ich == NYT ) {
		getBits( &v, 8, hFileIn );
		ich = v;
		addNode( tree, (unsigned char) ich );
	}
	
	return ich;
}


/*
=====================
	beginCompress
=====================
*/
BOOL beginCompress( TCHAR *szPath ) {
	TCHAR		szArchPath[MAX_PATH] = {0};
	HANDLE		hFileIn;
	HANDLE		hFileOut;
	huffTree_t	*Tree;
	int			ich;
	BOOL		bRes;
	int			size;


	//check arguments validity
	if( szPath == NULL ) {
		return FALSE;
	}


	hFileIn =
	hFileOut = INVALID_HANDLE_VALUE;
	
	Tree = NULL;
	
	__try {
		bRes = FALSE;

		//open files

		hFileIn = N_FOpenR( szPath );
		if( hFileIn == INVALID_HANDLE_VALUE ) {
			__leave;
		}

		N_Sprintf( szArchPath, MAX_PATH, TEXT( "%s.z" ), szPath );
		hFileOut = N_FOpenW( szArchPath );
		if( hFileOut == INVALID_HANDLE_VALUE ) {
			__leave;
		}

		size = 0;
		N_FWrite( hFileOut, &size, sizeof( size ) );

		//init compress model
		Tree = N_Malloc( sizeof( *Tree ) );
		initTree( Tree );
		
		out_pack = 0;
		out_block = 0;
		
		//compress
		while( (ich = N_FGetc( hFileIn )) != N_EOF ) {
			encodeSymbol( Tree, (unsigned char) ich, hFileOut );
			updateTree( Tree, (unsigned char) ich );
			size++;
		}
		
		//flush remaining bits
		put_bit( -1, hFileOut );
		
		N_FSeek( hFileOut, 0, FILE_BEGIN );
		N_FWrite( hFileOut, &size, sizeof( size ) );
		
		bRes = TRUE;
	}
	
	__finally {
		if( hFileIn != INVALID_HANDLE_VALUE ) {
			N_FClose( hFileIn );
		}
		
		if( hFileOut != INVALID_HANDLE_VALUE ) {
			N_FClose( hFileOut );
		}
		
		if( Tree ) {
			N_Free( Tree );
		}
	}
	
	return bRes;
}


/*
=====================
	endCompress
=====================
*/
void endCompress( TCHAR *szPath ) {
	TCHAR szArchPath[MAX_PATH] = {0};
	
	//check arguments validity
	//
	if( szPath == NULL ) {
		return;
	}
	
	//delete compressed file
	//
	N_Sprintf( szArchPath, MAX_PATH, TEXT( "%s.z" ), szPath );
	DeleteFile( szArchPath );
}


static int tmpID = 0; //temporary file suffix

/*
=====================
	beginDecompress
=====================
*/
BOOL beginDecompress( TCHAR *szPack, LONG offset, TCHAR *szPath, int *pUSize ) {
	HANDLE hFileOut;
	HANDLE hFileIn;
	BOOL bRes;
	huffTree_t *Tree;
	int size;
	int ich;

	
	//check arguments validity
	if( (szPack == NULL) || (offset<0) || (szPath == NULL) ) {
		return FALSE;
	}


	//init variables

	hFileOut	=
	hFileIn		= INVALID_HANDLE_VALUE;

	Tree = NULL;

	__try {
		bRes = FALSE;

		//init files
		N_Sprintf( szPath, FILENAMESIZE, TEXT( "tmp%d.u" ), tmpID );
		tmpID++;
		
		hFileOut = N_FOpenW( szPath );
		if( hFileOut == INVALID_HANDLE_VALUE ) {
			__leave;
		}
		
		hFileIn = N_FOpenR( szPack );
		if( hFileIn == INVALID_HANDLE_VALUE ) {
			__leave;
		}
		
		N_FSeek( hFileIn, offset, FILE_BEGIN );
		
		//get original (uncompressed) file size
		N_FRead( hFileIn, &size, sizeof( size ) );
		if( pUSize ) {
			*pUSize = size;
		}
	
		//init decompress model
		Tree = N_Malloc( sizeof( *Tree ) );
		initTree( Tree );
		
		in_block = 0;
		in_pack = 0;
		
		//decompress
		while( size-- ) {
			ich = decodeSymbol( Tree, hFileIn );
			N_FPutc( hFileOut, (unsigned char)ich );
			updateTree( Tree, ich );
		}
		
		bRes = TRUE;
	}
	
	__finally {
		if( hFileIn != INVALID_HANDLE_VALUE ) {
			N_FClose( hFileIn );
		}
		
		if( hFileOut != INVALID_HANDLE_VALUE ) {
			N_FClose( hFileOut );
		}
		
		if( Tree ) {
			N_Free( Tree );
		}
	}
	
	return bRes;
}


/*
=====================
	endDecompress
=====================
*/
void endDecompress( TCHAR *szPath ) {
	DeleteFile( szPath );
	
	if( tmpID >= 10 ) {
		tmpID = 0;
	}
}