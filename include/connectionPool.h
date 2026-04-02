#ifndef _CONNECTIONPOOL_H_
#define _CONNECTIONPOOL_H_

#include <atomic>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

#include "commonConnection.h"

using namespace std;

class ConnectionPool {
 public:
  static ConnectionPool* getConnectionPool();

  shared_ptr<Connection> getConnection();

 private:
  ConnectionPool();
  ~ConnectionPool();

  bool loadConfigFile();

  bool loadConfigFile(const std::string& filename,
                      std::unordered_map<std::string, std::string>& config);

  void produceConnTask();

  void scannerConnTask();
  void stop();  // 结束后台线程
  string m_ip;
  unsigned short m_port;
  string m_username;
  string m_passwd;
  string m_dbname;

  int m_init_size;
  int m_max_size;
  int m_max_IdleTime;
  int m_connectionTimeout;

  // create connection_pool
  queue<Connection*> m_connectionQueue;
  mutex m_queueMutex;  // the mutex

  atomic_int m_connectionCnt;  // count for connection
  condition_variable cv;

  bool m_stopFlag;

  // 后台线程
  std::thread m_producerThread;
  std::thread m_scannerThread;
};

#endif