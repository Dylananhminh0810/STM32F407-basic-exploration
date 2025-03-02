#ifndef REGISTER_H
#define REGISTER_H

#define REG32(x) *((uint32_t volatile*)(x)) //make sure the object doesn't get optimized out with volatile
	
#endif