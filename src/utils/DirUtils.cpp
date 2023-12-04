#include <unistd.h>
#include <sys/types.h>
#ifdef __MINGW32__
#include <shlwapi.h>
#include <psapi.h>
#pragma comment(lib, "shlwapi.lib")
#else
#include <pwd.h>
#endif
#include "src/utils/DirUtils.h"

#if defined(__CYGWIN__) || defined(MS_WINDOWS)
#define PATH_MAX 260
#elif defined(linux)
#include <linux/limits.h>
#elif defined(FreeBSD)
#include <sys/syslimits.h>
#else

//we don't know how to handle this situation, check if it's defined
//the is not necessarily an issue, actually in most cases it will just work
#warning "Unknown system or arch, trying fallback strategy. Please check if the compilation is correct"
#ifndef PATH_MAX
#define PATH_MAX 260
#endif

#endif


namespace nacos{
NacosString DirUtils::getHome() {
#ifdef __MINGW32__
  NacosString path= "";
  const char* homeProfile = "USERPROFILE";
  char homePath[1024] = {0};
  unsigned int pathSize = GetEnvironmentVariable(homeProfile, homePath, 1024);
  if (pathSize == 0 || pathSize > 1024){
      // 获取失败 或者 路径太长 
      int ret = GetLastError();
  }
  else {
      path = homePath;
  }
#else
    struct passwd *pw = getpwuid(getuid());
    NacosString homedir = pw->pw_dir;
    return homedir;
#endif
}

NacosString DirUtils::getCwd() {
    char cwd[PATH_MAX];
    NacosString cwds;
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        cwds = cwd;
        return cwds;
    }

    return NULLSTR;
}
}//namespace nacos
