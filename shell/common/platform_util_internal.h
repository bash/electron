// Copyright (c) 2020 Slack Technologies, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#ifndef ELECTRON_SHELL_COMMON_PLATFORM_UTIL_INTERNAL_H_
#define ELECTRON_SHELL_COMMON_PLATFORM_UTIL_INTERNAL_H_

#include "shell/common/platform_util.h"

#include <string>

namespace base {
class FilePath;
}

namespace platform_util::internal {

enum struct PlatformTrashItemAsyncResult { Success, Unsupported, Failure };

// Called by platform_util.cc on to invoke platform specific logic to move
// |path| to trash using a suitable handler.
bool PlatformTrashItem(const base::FilePath& path, std::string* error);

#if BUILDFLAG(IS_LINUX)
void PlatformTrashItemAsync(
    const base::FilePath& path,
    base::OnceCallback<void(PlatformTrashItemAsyncResult, const std::string&)>
        callback);
#endif

}  // namespace platform_util::internal

#endif  // ELECTRON_SHELL_COMMON_PLATFORM_UTIL_INTERNAL_H_
