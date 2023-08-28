#ifndef my_MACROS_H
#define my_MACROS_H


#define LE_SWAP16(x)	( (( x & 0x00ff ) << 8 ) | (( x & 0xff00 ) >> 8 ))


#define LE_SWAP32(x)	( (( x & 0x000000ffUL ) << 24 ) \
						| (( x & 0x0000ff00UL ) << 8  ) \
						| (( x & 0x00ff0000UL ) >> 8  ) \
						| (( x & 0xff000000UL ) >> 24 ) )

#define LE_SWAP64(x)	( (( x & 0x00000000000000ffULL ) << 56 ) \
						| (( x & 0x000000000000ff00ULL ) << 40 ) \
						| (( x & 0x0000000000ff0000ULL ) << 24 ) \
						| (( x & 0x00000000ff000000ULL ) << 8  ) \
						| (( x & 0x000000ff00000000ULL ) >> 8  ) \
						| (( x & 0x0000ff0000000000ULL ) >> 24 ) \
						| (( x & 0x00ff000000000000ULL ) >> 40 ) \
						| (( x & 0xff00000000000000ULL ) >> 56 ) )


// These two are specific to audio driver
// swapping endian AND shifting into MSBits of int32

#define LE_SWAP16TO32(x)	( (( x & 0x00ff ) << 24 ) | (( x & 0xff00 ) << 8 ))

// swap 32 into MSB 24
#define LE_SWAP24TO32(x)	( (( x & 0x0000ff00UL ) << 8  ) \
						| (( x & 0x00ff0000UL ) >> 8  ) \
						| (( x & 0xff000000UL ) >> 24 ) )


#endif
