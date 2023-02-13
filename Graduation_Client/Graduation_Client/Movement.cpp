#pragma once
#include "Movement.h"
#include "Input.h"

void CommonMovement::start()
{
}

void CommonMovement::update()
{
	UCHAR keyBuffer[256];
	memcpy(keyBuffer,Input::GetInstance()->keyBuffer, (sizeof(keyBuffer)));
	if (keyBuffer['w'] & 0xF0 || keyBuffer['W'] & 0xF0)
		std::cout << "wŰ" << std::endl;
	if (keyBuffer['s'] & 0xF0 || keyBuffer['S'] & 0xF0)
		std::cout << "sŰ" << std::endl;
	if (keyBuffer['a'] & 0xF0 || keyBuffer['A'] & 0xF0)std::cout << "aŰ" << std::endl;
	if (keyBuffer['d'] & 0xF0 || keyBuffer['D'] & 0xF0) std::cout << "dŰ" << std::endl;
	if (keyBuffer['q'] & 0xF0 || keyBuffer['Q'] & 0xF0) std::cout << "qŰ" << std::endl;
	if (keyBuffer['e'] & 0xF0 || keyBuffer['E'] & 0xF0)std::cout << "eŰ" << std::endl;
}
