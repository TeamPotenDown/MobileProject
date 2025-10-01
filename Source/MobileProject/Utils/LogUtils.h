#pragma once

#define MP_FUNC ANSI_TO_TCHAR(__FUNCTION__)

#define MP_LOGF(Category, Verbosity, Format, ...) \
UE_LOG(Category, Verbosity, TEXT("[%s] ") Format, MP_FUNC, ##__VA_ARGS__)

#define MP_LOGS(Category, Verbosity, Str) \
UE_LOG(Category, Verbosity, TEXT("[%s] %s"), MP_FUNC, *(Str))

DECLARE_LOG_CATEGORY_EXTERN(LogMP, Log, All);