// Copyright (c) 2020 Slack Technologies, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include "shell/common/platform_util.h"

#include <utility>

#include "base/functional/bind.h"
#include "base/task/thread_pool.h"
#include "content/public/browser/browser_task_traits.h"
#include "shell/common/platform_util_internal.h"

namespace platform_util {

namespace {

struct TrashItemResult {
  bool success;
  std::string error;
};

TrashItemResult TrashItemOnBlockingThread(const base::FilePath& full_path) {
  std::string error;
  bool success = internal::PlatformTrashItem(full_path, &error);
  return {success, error};
}

void TrashItemSync(
    const base::FilePath& full_path,
    base::OnceCallback<void(bool, const std::string&)> callback) {
  // XXX: is continue_on_shutdown right?
  base::ThreadPool::PostTaskAndReplyWithResult(
      FROM_HERE,
      {base::MayBlock(), base::WithBaseSyncPrimitives(),
       base::TaskPriority::USER_BLOCKING,
       base::TaskShutdownBehavior::CONTINUE_ON_SHUTDOWN},
      base::BindOnce(&TrashItemOnBlockingThread, full_path),
      base::BindOnce(
          [](base::OnceCallback<void(bool, const std::string&)> callback,
             TrashItemResult result) {
            std::move(callback).Run(result.success, result.error);
          },
          std::move(callback)));
}

}  // namespace

void TrashItem(const base::FilePath& full_path,
               base::OnceCallback<void(bool, const std::string&)> callback) {
#if BUILDFLAG(IS_LINUX)
  internal::PlatformTrashItemAsync(
      full_path,
      base::BindOnce(
          [](const base::FilePath& full_path,
             base::OnceCallback<void(bool, const std::string&)> callback,
             const internal::PlatformTrashItemAsyncResult result,
             const std::string& error) {
            switch (result) {
              case internal::PlatformTrashItemAsyncResult::Success:
                std::move(callback).Run(true, error);
                break;
              case internal::PlatformTrashItemAsyncResult::Failure:
                std::move(callback).Run(false, error);
                break;
              case internal::PlatformTrashItemAsyncResult::Unsupported:
                TrashItemSync(full_path, std::move(callback));
                break;
            }
          },
          full_path, std::move(callback)));
#else
  TrashItemSync(full_path, std::move(callback));
#endif
}

}  // namespace platform_util
