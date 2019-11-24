#ifndef GREP_H
#define GREP_H

#include <QDirIterator>
#include <QFileInfo>
#include <QString>
#include <QStringList>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_set>

namespace std {
template <>
struct hash<QString> {
    std::size_t operator()(const QString &s) const { return qHash(s); }
};
}  // namespace std

class grep_with_scheduler {
public:
    grep_with_scheduler(bool &searching, QStringList &&start_pathes, bool case_policy, std::function<void()> &&on_success,
                        std::function<void(int, QStringList const &)> &&on_changes);
    ~grep_with_scheduler();

private:
    const int working_threads_amount;
    std::atomic_int exited_threads_amount;
    std::thread checking;

    bool &searching;
    bool plusing;
    std::vector<std::thread> threads;
    std::function<void()> on_success;
    std::function<void(int, QStringList const &)> on_changes;
    std::queue<QString> files;
    std::atomic_uint64_t scanned;
    std::atomic_uint64_t total;
    std::mutex lock_files_queue;
    std::unordered_set<QString> already_added;
};

#endif
