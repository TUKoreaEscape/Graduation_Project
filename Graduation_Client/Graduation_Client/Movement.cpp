#pragma once
#include "Movement.h"
#include "Input.h"

void CommonMovement::start()
{
}

void CommonMovement::update(float elapsedTime)
{
	UCHAR keyBuffer[256];
	memcpy(keyBuffer,Input::GetInstance()->keyBuffer, (sizeof(keyBuffer)));
	if (keyBuffer['w'] & 0xF0 || keyBuffer['W'] & 0xF0)
		std::cout << "w키" << std::endl;
	if (keyBuffer['s'] & 0xF0 || keyBuffer['S'] & 0xF0)
		std::cout << "s키" << std::endl;
	if (keyBuffer['a'] & 0xF0 || keyBuffer['A'] & 0xF0)
		std::cout << "a키" << std::endl;
	if (keyBuffer['d'] & 0xF0 || keyBuffer['D'] & 0xF0) 
		std::cout << "d키" << std::endl;
}
