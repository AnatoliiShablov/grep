#include "grep.h"

multithreading_grep::multithreading_grep()
    : working_threads_amount{8}
    , exited_threads_amount{0}
    , exit_thread{[this] {
        std::unique_lock<std::mutex> lock(exit_mutex);
        cv_threads_exit.wait(lock, [this] { return exited_threads_amount == working_threads_amount; });
        complete_thread.join();
        std::for_each(working_threads.begin(), working_threads.end(), [](std::thread &th) { th.join(); });
    }}
    , completed_threads_amount{8}
    , complete_thread{[this] {
        std::unique_lock<std::mutex> lock(complete_mutex);
        while (!quit.load()) {
            cv_threads_complete.wait(lock, [this] {
                return (quit.load() || complete.load()) && completed_threads_amount == working_threads_amount;
            });
            {
                complete.store(false);
                std::lock_guard<std::mutex> lock_term(terminate_mutex);
                if (!quit.load() && !terminate.load()) {
                    emit send_complete_signal();
                }
                terminate.store(false);
            }
            cv_terminate.notify_one();
        }
    }}
    , terminate{false}
    , work{false}
    , complete{false}
    , quit{false}
    , parsing_threads{0} {
    for (size_t i = 0; i < working_threads_amount; ++i) {
        working_threads.emplace_back([this] {
            while (true) {
                std::unique_lock<std::mutex> work_lock(work_mutex);
                cv_work.wait(work_lock, [this] { return work.load() || quit.load(); });
                if (quit.load()) {
                    if (++exited_threads_amount == working_threads_amount) {
                        cv_threads_complete.notify_one();
                        cv_threads_exit.notify_one();
                    }
                    return;
                }
                --completed_threads_amount;
                bool case_this_thread = case_policy.load();
                QString substr_this_thread = substr;
                std::vector<int> pi;
                work_lock.unlock();
                while (!terminate.load()) {
                    std::unique_lock<std::mutex> files_lock(files_mutex);
                    if (files.empty()) {
                        std::unique_lock pathes_lock(pathes_mutex);
                        if (start_pathes.empty()) {
                            if (parsing_threads.load() == 0) {
                                break;
                            } else {
                                pathes_lock.unlock();
                                cv_file_added.wait(files_lock);
                            }
                        } else {
                            ++parsing_threads;
                            QString path = start_pathes.front();
                            start_pathes.pop();
                            pathes_lock.unlock();
                            files_lock.unlock();
                            if (QFileInfo info{path}; info.isDir()) {
                                QDirIterator it(path, QDir::Files, QDirIterator::Subdirectories);
                                while (it.hasNext()) {
                                    if (terminate.load()) {
                                        --parsing_threads;
                                        cv_file_added.notify_all();
                                        break;
                                    }
                                    files_lock.lock();
                                    if (QString file_now = it.next();
                                        already_added_files.count(file_now) == 0 && QFileInfo{file_now}.isReadable()) {
                                        ++total_amount_approx;
                                        files.push(file_now);
                                        files_lock.unlock();
                                        cv_file_added.notify_all();
                                        continue;
                                    }
                                    files_lock.unlock();
                                }
                            } else if (already_added_files.count(path) == 0 && info.isReadable()) {
                                ++total_amount_approx;
                                files_lock.lock();
                                files.push(path);
                                files_lock.unlock();
                                cv_file_added.notify_all();
                            }
                            --parsing_threads;
                            cv_file_added.notify_all();
                        }
                    } else {
                        QString tmp = files.front();
                        files.pop();
                        files_lock.unlock();
                        QFile file(tmp);
                        if (file.exists() && file.open(QFile::ReadOnly | QFile::Text)) {
                            size_t line_number = 1;
                            while (!file.atEnd()) {
                                if (terminate.load()) {
                                    break;
                                }
                                QString line;
                                try {
                                    if (case_this_thread) {
                                        line = file.readLine();
                                    } else {
                                        line = file.readLine().toLower();
                                    }
                                } catch (...) {
                                    std::fprintf(stderr, "Can't read line %9zu in file %s\n", line_number,
                                                 tmp.toStdString().c_str());
                                    break;
                                }

                                if (line.isEmpty()) {
                                    ++line_number;
                                    continue;
                                }
                                pi.assign(static_cast<size_t>(substr_this_thread.length()), 0);
                                for (int i = 1, k = 0; i < substr_this_thread.length(); ++i) {
                                    while (k && substr_this_thread[k] != substr_this_thread[i]) {
                                        k = pi[static_cast<size_t>(k - 1)];
                                    }
                                    if (substr_this_thread[k] == substr_this_thread[i]) {
                                        ++k;
                                    }
                                    pi[static_cast<size_t>(i)] = k;
                                }
                                for (int i = 0, k = 0; i < line.length(); ++i) {
                                    while (k && substr_this_thread[k] != line[i]) {
                                        k = pi[static_cast<size_t>(k - 1)];
                                    }
                                    if (substr_this_thread[k] == line[i]) {
                                        ++k;
                                    }
                                    if (k == substr_this_thread.length()) {
                                        emit send_info_substr(tmp + ":" + QString::number(line_number) + ":" +
                                                              QString::number(i - k + 2));
                                    }
                                }
                                ++line_number;
                            }
                            emit send_info_percentage(static_cast<int>((100 * ++amount_of_scanned) / total_amount_approx));
                        }
                    }
                }
                work_lock.lock();
                work.store(false);
                work_lock.unlock();
                if (++completed_threads_amount == working_threads_amount) {
                    complete.store(true);
                    cv_threads_complete.notify_one();
                }
            }
        });
    }
}

multithreading_grep::~multithreading_grep() {
    {
        std::lock_guard lock(work_mutex);
        quit.store(true);
        terminate.store(true);
    }
    cv_work.notify_all();
    exit_thread.join();
}

void multithreading_grep::terminate_process() {
    terminate.store(true);
    std::unique_lock<std::mutex> lock(terminate_mutex);
    cv_terminate.wait(lock, [this] { return !terminate.load() || !work.load(); });
}

void multithreading_grep::new_task(std::queue<QString> start_pathes, QString substr, bool case_policy) {
    this->start_pathes = std::move(start_pathes);
    this->case_policy.store(case_policy);
    if (!case_policy) {
        this->substr = substr.toLower();
    } else {
        this->substr = std::move(substr);
    }
    this->already_added_files.clear();
    this->files = decltype(this->files){};
    total_amount_approx.store(0);
    amount_of_scanned.store(0);
    {
        std::lock_guard<std::mutex> lock_work(work_mutex);
        work.store(true);
    }
    cv_work.notify_all();
}
