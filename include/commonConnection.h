#ifndef COMMONCONNECTION_H
#define COMMONCONNECTION_H

#include <mysql/mysql.h>

#include <ctime>
#include <string>

using namespace std;
#include "public.h"

class Connection {
 public:
  Connection();
  ~Connection();

  // connect sql
  bool connect(string ip, unsigned short port, string user, string password,
               string dbname);

  // update function example:insert 、delete、update
  bool update(string sql);

  // the query for select
  MYSQL_RES* query(string sql);

  void refreshAliveTime() { m_alivetime = clock(); }
  clock_t getAlivetime() { return clock() - m_alivetime; }

  bool isAlive();

  MYSQL* getRawConn() { return m_conn; }

 private:
  MYSQL* m_conn;
  clock_t m_alivetime;
};

#endif