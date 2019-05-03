#pragma once
#ifndef   _PATH_PREFIX___H_
#define   _PATH_PREFIX___H_

#include <string>
#include <Windows.h>

std::string compute_prefix() {

	if (IsDebuggerPresent()) return std::string("../");
	else					 return std::string("../../Sample/");

}


#endif