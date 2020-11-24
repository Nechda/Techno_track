#pragma once

#ifdef __GNUG__
#define __FUNCSIG__ __PRETTY_FUNCTION__
#endif

#define Assert_c(expr) if(!(expr)) printf("Expression %s is false.\nIn file: %s\nfunction: %s\nline: %d\n", #expr, __FILE__, __FUNCSIG__, __LINE__);
