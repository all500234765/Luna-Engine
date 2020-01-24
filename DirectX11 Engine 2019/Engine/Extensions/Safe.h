#ifndef __SAFE_DELETE__
#define __SAFE_DELETE__

#define SAFE_DELETE(x) if( x ) { delete (x); (x) = 0; }
#define SAFE_RELEASE(x) if( x ) { (x)->Release(); SAFE_DELETE(x); }
#define SAFE_RELEASE_N(x, y) { for( int i = 0; i < y; i++ ) SAFE_RELEASE((x)[i]); }
#define SAFE_DELETE_N(x, y) { for( int i = 0; i < y; i++ ) SAFE_DELETE((x)[i]); }
#define SAFE_RELEASE_RENDERER(x, y) if( y ) { ((x*)y)->Release(); delete (x*)y; y = 0; }

#endif
