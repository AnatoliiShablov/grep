#ifndef GREP_H
#define GREP_H

#include <QDirIterator>
#include <QFileInfo>
#include <QString>
#include <QStringList>
#include <condition_variable>
#include <functional>
#include <iostream>
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

class multithreading_grep: public QObject {
    Q_OBJECT
public:
    multithreading_grep();
    ~multithreading_grep();

    void terminate_process();

    void new_task(std::queue<QString> start_pathes, QString substr, bool case_policy);
signals:
    void send_info_percentage(int);

    void send_info_substr(QString const &);

    void send_complete_signal();

private:
    const size_t working_threads_amount;

    std::atomic_size_t exited_threads_amount;
    std::mutex exit_mutex;
    std::condition_variable cv_threads_exit;
    std::thread exit_thread;

    std::atomic_size_t completed_threads_amount;
    std::mutex complete_mutex;
    std::condition_variable cv_threads_complete;
    std::thread complete_thread;

    std::vector<std::thread> working_threads;

    std::atomic_bool terminate;
    std::mutex terminate_mutex;
    std::condition_variable cv_terminate;

    std::atomic_bool work;
    std::mutex work_mutex;
    std::condition_variable cv_work;

    std::atomic_bool complete;
    std::atomic_bool quit;

    std::queue<QString> start_pathes;
    QString substr;
    std::atomic_bool case_policy;
    std::atomic_size_t parsing_threads;
    std::mutex pathes_mutex;

    std::unordered_set<QString> already_added_files;
    std::queue<QString> files;
    std::mutex files_mutex;
    std::mutex low_priority;
    std::mutex next_to_access;
    std::condition_variable cv_file_added;

    std::atomic_uint64_t amount_of_scanned;
    std::atomic_uint64_t total_amount_approx;
};

#endif
