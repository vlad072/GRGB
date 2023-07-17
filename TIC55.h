#ifndef _TIC55_INCLUDED_
#define _TIC55_INCLUDED_  

#define _VER_TEXT 				5000
#define _SLEEP_TEXT 			5001
#define _EMPTY_TEXT 			5002
#define _RESET_TEXT				5003


#define __minus 25 // -
#define ___ 26
#define __A 10
#define __b 11
#define __C 12
#define __d 13
#define __E 14
#define __F 15
#define __G 16
#define __H 17
#define __I 1
#define __L 18
#define __n 19
#define __o 20
#define __O 0
#define __P 21
#define __S 5
#define __r 22
#define __t 23
#define __u 24
//#define __U 28
//#define __Y 25

code unsigned char  symbol[] = 
{
		111, 	// 0
		3, 		// 1
		118, 	// 2
		87, 	// 3
		27, 	// 4
		93, 	// 5
		125, 	// 6
		15, 	// 7
		127, 	// 8
		95, 	// 9
		63, 	// 10 = A	
		121, 	// 11 = b	
		108,	// 12 = C	
		115,	// 13 = d	
		124, 	// 14 = E	
		60, 	// 15 = F	
		109,	// 16 = G	
		59, 	// 17 = H	
		104, 	// 18 = L
		49,		// 19 = n		
		113,	// 20 = о	
		62,		// 21 = P	
		48,		// 22 = r		
		120,	// 23 = t	
		97,  	// 24 = u	
		16, 	// 25 = -		
		0 		// 26 = пустой (пробел)
}; 

#endif