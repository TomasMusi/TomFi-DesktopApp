#ifndef DATABASE_H
#define DATABASE_H

#include <string>

using namespace std;

bool verify_login(const string &email, const string &password);
bool register_data(const string &name, const string &email, const string &password);

#endif
