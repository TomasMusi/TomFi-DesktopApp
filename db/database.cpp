#include "database.h"
#include "mysql.h"
#include <iostream>
#include "../env.hpp"
#include <bcrypt/bcrypt.h>

using namespace std;

bool verify_login(const string &email, const string &password)
{
    MYSQL *conn;    // Pointer to the connection object
    MYSQL_RES *res; // Result of the query
    MYSQL_ROW row;

    // Loading ENV DATA.

    string DATABASE_IP = env_vars["DATABASE_IP"];
    string DATABASE_USER = env_vars["DATABASE_USER"];
    string DATABASE_PASSWORD = env_vars["DATABASE_PASSWORD"];
    string DATABASE_NAME = env_vars["DATABASE_NAME"];
    string DATABASE_PORT = env_vars["DATABASE_PORT"];

    // Initialize MySQL connection
    conn = mysql_init(NULL);

    if (!conn)
    {
        cerr << "❌ mysql_init() failed" << endl;
        return false;
    }

    // Connection to the database.

    if (!mysql_real_connect(conn, DATABASE_IP.c_str(), DATABASE_USER.c_str(), DATABASE_PASSWORD.c_str(), DATABASE_NAME.c_str(), stoi(DATABASE_PORT), NULL, 0))
    {
        cerr << "❌ Connection failed:" << mysql_error(conn) << endl;
        mysql_close(conn);
        return false;
    }

    // Building Query.

    // ⚠️ VERY RISKY QUERY JUST FOR TESTING PURPOUSES NOW! ⚠️

    string query = "SELECT * FROM Users WHERE email='" + email + "'";

    // executing QUERY.

    if (mysql_query(conn, query.c_str()))
    {
        cerr << " Query Failed! " << mysql_error(conn) << endl;
        mysql_close(conn);
        return false;
    }

    res = mysql_store_result(conn);
    if (mysql_num_rows(res) == 0)
    {
        cout << "⚠️ No user found with email: " << email << endl;
        mysql_free_result(res); // CLEANUP Not holding any values.
        mysql_close(conn);
        return false;
    }

    // fetching, meaning getting the data.
    row = mysql_fetch_row(res);

    const char *hashed = row[4];          // Password from DB
    const char *plain = password.c_str(); // User input

    int bcrypt_result = bcrypt_checkpw(plain, hashed);

    if (bcrypt_result != 0)
    {
        // Password wrong
        mysql_free_result(res);
        mysql_close(conn);
        return false;
    }

    cout << "✅ User found!" << endl;
    cout << "Name: " << row[0] << endl;
    cout << "Profile Picture: " << row[1] << endl;
    cout << "ID: " << row[2] << endl;
    cout << "Email: " << row[3] << endl;
    cout << "Password: " << row[4] << endl;
    cout << "Role: " << row[5] << endl;
    cout << "2FA Enabled: " << row[6] << endl;
    cout << "2FA Secret: " << row[7] << endl;
    cout << "Created At: " << row[8] << endl;

    // Clean up
    mysql_free_result(res);
    mysql_close(conn);
    return true;
}