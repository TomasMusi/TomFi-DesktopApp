#include "database.h"
#include "mysql.h"
#include <iostream>
#include "../env.hpp"
#include "../bcrypt/bcrypt.h"
#include <cstring>

using namespace std;

bool verify_login(const string &email, const string &password)
{
    MYSQL *conn;                    // ,,A phone call to the database"
    MYSQL_STMT *stmt;               // prepared question we want to ask the database
    MYSQL_BIND param_bind[1];       // where we hold what we want to ask (the email)
    MYSQL_BIND result_bind[9];      // where we will keep the answer
    MYSQL_RES *prepare_meta_result; // helps organize the answers

    // Output buffer variables for result columns. These are like boxes to store answers we get from database.
    char name[100], profile_picture[255], email_db[100], password_db[255], role[20], twofa_secret[100], created_at[100];
    int id, twofa_enabled;

    // 1. Load environment config
    string DATABASE_IP = env_vars["DATABASE_IP"];
    string DATABASE_USER = env_vars["DATABASE_USER"];
    string DATABASE_PASSWORD = env_vars["DATABASE_PASSWORD"];
    string DATABASE_NAME = env_vars["DATABASE_NAME"];
    string DATABASE_PORT = env_vars["DATABASE_PORT"];

    // 2. Init MySQL connection
    conn = mysql_init(NULL);
    if (!conn)
    {
        cerr << "❌ mysql_init() failed\n";
        return false;
    }

    if (!mysql_real_connect(conn,
                            DATABASE_IP.c_str(),
                            DATABASE_USER.c_str(),
                            DATABASE_PASSWORD.c_str(),
                            DATABASE_NAME.c_str(),
                            stoi(DATABASE_PORT),
                            NULL, 0))
    {
        cerr << "❌ Connection failed: " << mysql_error(conn) << endl;
        mysql_close(conn);
        return false;
    }

    // 3. Prepare SQL query with 1 placeholder (?)
    const char *sql = "SELECT * FROM Users WHERE email = ?"; // this is the question we want to ask database: ,,Hey database, give me all the info about the user whose email is waht I'm going to give you." The ? is a placeholder. We will plug the real email in later. This protects us from bad hackers who try to break into systems using tricks.
    stmt = mysql_stmt_init(conn);
    if (!stmt)
    {
        cerr << "❌ mysql_stmt_init() failed\n";
        mysql_close(conn);
        return false;
    }

    if (mysql_stmt_prepare(stmt, sql, strlen(sql)))
    {
        cerr << "❌ Prepare failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }

    // 4. Bind input parameter securely (prevents SQL injection)
    memset(param_bind, 0, sizeof(param_bind));

    // This part says: ,,Okay, put the email the user typed into that ? placeholder."
    param_bind[0].buffer_type = MYSQL_TYPE_STRING;
    param_bind[0].buffer = (void *)email.c_str();
    param_bind[0].buffer_length = email.length();

    if (mysql_stmt_bind_param(stmt, param_bind))
    {
        cerr << "❌ Bind failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }

    // 5. Execute the query. This actually sends the question to the database and waits for a reply.
    if (mysql_stmt_execute(stmt))
    {
        cerr << "❌ Execute failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }

    // 6. Bind result columns to output variables. This gets info about what the answer will look like (how many columns, what type of data etc.)
    prepare_meta_result = mysql_stmt_result_metadata(stmt);
    if (!prepare_meta_result)
    {
        cerr << "❌ Failed to get result metadata\n";
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }

    memset(result_bind, 0, sizeof(result_bind));

    // ,,When you give me the answer, put each part in these boxes."
    result_bind[0].buffer_type = MYSQL_TYPE_STRING;
    result_bind[0].buffer = name;
    result_bind[0].buffer_length = sizeof(name);

    result_bind[1].buffer_type = MYSQL_TYPE_STRING;
    result_bind[1].buffer = profile_picture;
    result_bind[1].buffer_length = sizeof(profile_picture);

    result_bind[2].buffer_type = MYSQL_TYPE_LONG;
    result_bind[2].buffer = &id;

    result_bind[3].buffer_type = MYSQL_TYPE_STRING;
    result_bind[3].buffer = email_db;
    result_bind[3].buffer_length = sizeof(email_db);

    result_bind[4].buffer_type = MYSQL_TYPE_STRING;
    result_bind[4].buffer = password_db;
    result_bind[4].buffer_length = sizeof(password_db);

    result_bind[5].buffer_type = MYSQL_TYPE_STRING;
    result_bind[5].buffer = role;
    result_bind[5].buffer_length = sizeof(role);

    result_bind[6].buffer_type = MYSQL_TYPE_LONG;
    result_bind[6].buffer = &twofa_enabled;

    result_bind[7].buffer_type = MYSQL_TYPE_STRING;
    result_bind[7].buffer = twofa_secret;
    result_bind[7].buffer_length = sizeof(twofa_secret);

    result_bind[8].buffer_type = MYSQL_TYPE_STRING;
    result_bind[8].buffer = created_at;
    result_bind[8].buffer_length = sizeof(created_at);

    if (mysql_stmt_bind_result(stmt, result_bind))
    {
        cerr << "❌ Bind result failed: " << mysql_stmt_error(stmt) << endl;
        mysql_free_result(prepare_meta_result);
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }

    // 7. Fetch the result. This actually fetches the row from the database where the email matched.
    if (mysql_stmt_fetch(stmt) != 0)
    {
        cerr << "⚠️ No user found with email: " << email << endl;
        mysql_free_result(prepare_meta_result);
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }

    // 8. Check password
    const char *hashed = password_db;
    const char *plain = password.c_str();

    if (bcrypt_checkpw(plain, hashed) != 0)
    {
        cerr << "❌ Incorrect password!" << endl;
        mysql_free_result(prepare_meta_result);
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }

    // ✅ Successful login
    cout << "✅ User found!" << endl;
    cout << "Name: " << name << endl;
    cout << "Profile Picture: " << profile_picture << endl;
    cout << "ID: " << id << endl;
    cout << "Email: " << email_db << endl;
    cout << "Role: " << role << endl;
    cout << "2FA Enabled: " << twofa_enabled << endl;
    cout << "2FA Secret: " << twofa_secret << endl;
    cout << "Created At: " << created_at << endl;

    // Clean up
    mysql_free_result(prepare_meta_result);
    mysql_stmt_close(stmt);
    mysql_close(conn);
    return true;
}

// Register

bool register_data(const string &name, const string &email, const string &password)
{

    MYSQL *conn;
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[8];

    // Inserting Default Fields.
    string profile_picture = "/pfp/default.png"; // Default avatar path
    string role = "user";                        // Role assigned to all new users
    int is_2fa_enabled = 0;                      // Disabled by default
    string two_fa_secret = "";
    string created_at = "NOW()"; // We’ll let MySQL insert timestamp (see note below)

    // Load env config

    string DATABASE_IP = env_vars["DATABASE_IP"];
    string DATABASE_USER = env_vars["DATABASE_USER"];
    string DATABASE_PASSWORD = env_vars["DATABASE_PASSWORD"];
    string DATABASE_NAME = env_vars["DATABASE_NAME"];
    string DATABASE_PORT = env_vars["DATABASE_PORT"];

    conn = mysql_init(NULL);
    if (!conn)
    {
        cerr << "❌ mysql_init() failed" << endl;
        return false;
    }

    // Connection

    if (!mysql_real_connect(conn,
                            DATABASE_IP.c_str(),
                            DATABASE_USER.c_str(),
                            DATABASE_PASSWORD.c_str(),
                            DATABASE_NAME.c_str(),
                            stoi(DATABASE_PORT),
                            NULL, 0))
    {
        cerr << "❌ Connection failed: " << mysql_error(conn) << endl;
        mysql_close(conn);
        return false;
    }

    // Check if the Email Already exists.

    const char *check_sql = "SELECT 1 FROM Users WHERE email = ?";
    stmt = mysql_stmt_init(conn);
    if (!stmt)
    {
        cerr << "❌ mysql_stmt_init() failed for check\n";
        mysql_close(conn);
        return false;
    }

    if (mysql_stmt_prepare(stmt, check_sql, strlen(check_sql)))
    {
        cerr << "❌ Prepare (check email) failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }

    MYSQL_BIND check_bind[1];
    memset(check_bind, 0, sizeof(check_bind));

    check_bind[0].buffer_type = MYSQL_TYPE_STRING;
    check_bind[0].buffer = (void *)email.c_str();
    check_bind[0].buffer_length = email.length();

    if (mysql_stmt_bind_param(stmt, check_bind))
    {
        cerr << "❌ Bind (check email) failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }

    if (mysql_stmt_execute(stmt))
    {
        cerr << "❌ Execute (check email) failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }

    if (mysql_stmt_store_result(stmt))
    {
        cerr << "❌ Store result (check email) failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }

    if (mysql_stmt_num_rows(stmt) > 0)
    {
        cerr << "⚠️ Email already exists: " << email << endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }

    mysql_stmt_close(stmt); // Done Checking

    // Preparing question

    const char *sql = R"(
        INSERT INTO Users 
        (name, email, password, user_profile_picture, role, is_2fa_enabled, two_fa_secret, created_at)
        VALUES (?, ?, ?, ?, ?, ?, ?, CURRENT_TIMESTAMP)
    )";
    stmt = mysql_stmt_init(conn);
    if (!stmt)
    {
        cerr << "❌ mysql_stmt_init() failed" << endl;
        mysql_close(conn);
        return false;
    }

    if (mysql_stmt_prepare(stmt, sql, strlen(sql)))
    {
        cerr << "❌ Prepare failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }

    memset(bind, 0, sizeof(bind));

    // Name
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (void *)name.c_str();
    bind[0].buffer_length = name.length();

    // Email
    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = (void *)email.c_str();
    bind[1].buffer_length = email.length();

    // Password
    bind[2].buffer_type = MYSQL_TYPE_STRING;
    bind[2].buffer = (void *)password.c_str();
    bind[2].buffer_length = password.length();

    // Profile Picture
    bind[3].buffer_type = MYSQL_TYPE_STRING;
    bind[3].buffer = (void *)profile_picture.c_str();
    bind[3].buffer_length = profile_picture.length();

    // Role
    bind[4].buffer_type = MYSQL_TYPE_STRING;
    bind[4].buffer = (void *)role.c_str();
    bind[4].buffer_length = role.length();

    // 2FA Enabled
    bind[5].buffer_type = MYSQL_TYPE_LONG;
    bind[5].buffer = &is_2fa_enabled;

    // TWO_FA_SECRET
    bind[6].buffer_type = MYSQL_TYPE_STRING;
    bind[6].buffer = (void *)two_fa_secret.c_str();
    bind[6].buffer_length = two_fa_secret.length();
    if (mysql_stmt_bind_param(stmt, bind))
    {
        cerr << "❌ Bind failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }

    if (mysql_stmt_execute(stmt))
    {
        cerr << "❌ Execute failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }

    cout << "✅ User registered successfully." << endl;

    mysql_stmt_close(stmt);
    mysql_close(conn);
    return true;
}