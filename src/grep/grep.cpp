#include "grep.h"

grep_with_scheduler::grep_with_scheduler(bool &searching, QStringList &&start_pathes, bool case_policy,
                                         std::function<void()> &&on_success,
                                         std::function<void(int, const QStringList &)> &&on_changes)
    : working_threads_amount{8}
    , exited_threads_amount{0}
    , checking{[this]() {
        while (exited_threads_amount != working_threads_amount) {
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }
        std::for_each(threads.begin(), threads.end(), [](auto &t) { t.join(); });
        this->on_success();
    }}
    , searching{searching}
    , plusing{true}
    , on_success{std::move(on_success)}
    , on_changes{std::move(on_changes)} {
    for (int i = 0; i < working_threads_amount; ++i) {
        threads.emplace_back([this, case_policy] {
            QString now;
            while (this->searching) {
                {
                    std::lock_guard<std::mutex> locker(this->lock_files_queue);
                    if (files.empty()) {
                        if (plusing) {
                            std::this_thread::sleep_for(std::chrono::microseconds(1));
                            continue;
                        } else {
                            break;
                        }
                    } else {
                        now = files.front();
                        files.pop();
                    }
                }
                Q_UNUSED(case_policy)
                this->on_changes(static_cast<int>(++scanned / total), QStringList{});
                ++scanned;
            }
            ++this->exited_threads_amount;
        });
    }
    for (auto &path : start_pathes) {
        if (!searching) {
            return;
        }
        if (QFileInfo info{path}; info.isDir()) {
            QDirIterator it(path, QDir::Files, QDirIterator::Subdirectories);
            while (it.hasNext()) {
                if (!searching) {
                    return;
                }
                if (QString file_now = it.next(); already_added.count(file_now) == 0 && QFileInfo{file_now}.isReadable()) {
                    ++total;
                    std::lock_guard<std::mutex> locker(lock_files_queue);
                    files.push(path);
                }
            }
        } else if (already_added.count(path) == 0 && info.isReadable()) {
            ++total;
            std::lock_guard<std::mutex> locker(lock_files_queue);
            files.push(path);
        }
    }
    plusing = false;
}

grep_with_scheduler::~grep_with_scheduler() {
    checking.join();
}
