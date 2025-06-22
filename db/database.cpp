#include "database.h"
#include "mysql.h"
#include <iostream>
#include "../env.hpp"
#include "../bcrypt/bcrypt.h"
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include "../Session/Session.hpp"

using namespace std;

// Generating Credit Card Numbers

string generateFormattedNumber()
{
    ostringstream oss;

    srand(static_cast<unsigned int>(time(nullptr))); // seed random once per run

    for (int i = 0; i < 4; ++i)
    {
        int group = 1000 + rand() % 9000; // random 4-digit number (1000–9999)
        oss << setw(4) << setfill('0') << group;
        if (i < 3)
        {
            oss << ' ';
        }
    }

    return oss.str();
}

// Generating Random Pin

int generate_four_digit_number()
{
    // Seed the random number generator
    srand(time(nullptr));

    // Generate and return a number between 1000 and 9999
    return 1000 + rand() % 9000;
}

/*

    string card_number = generateFormattedNumber();
    cout << "Generated card number: " << card_number << endl;

    int number = generate_four_digit_number();
    cout << "Random 4-digit number: " << number << endl;

*/

// Base64 Encode.

string base64_encode(const unsigned char *input, int length)
{

    BIO *bio, *b64;
    BUF_MEM *buffer_ptr;

    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL); // no newlines
    bio = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bio);

    BIO_write(b64, input, length);
    BIO_flush(b64);
    BIO_get_mem_ptr(b64, &buffer_ptr);

    std::string result(buffer_ptr->data, buffer_ptr->length);
    BIO_free_all(b64);
    return result;
}

// Encrypt PIN using public key
string encrypt_pin(const string &pin, const string &pubkey_path)
{
    FILE *pubkey_file = fopen(pubkey_path.c_str(), "r");
    if (!pubkey_file)
    {
        cerr << "❌ Failed to open public key" << endl;
        return "";
    }

    RSA *rsa = PEM_read_RSA_PUBKEY(pubkey_file, NULL, NULL, NULL);
    fclose(pubkey_file);
    if (!rsa)
    {
        cerr << "❌ Failed to read public key" << endl;
        return "";
    }

    unsigned char encrypted[256];
    int result = RSA_public_encrypt(pin.size(),
                                    (unsigned char *)pin.c_str(),
                                    encrypted,
                                    rsa,
                                    RSA_PKCS1_OAEP_PADDING);
    RSA_free(rsa);

    if (result == -1)
    {
        cerr << "❌ Encryption failed\n";
        return "";
    }

    return base64_encode(encrypted, result);
}

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
        cerr << "❌ Failed to get result metadata" << endl;
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

    // Middleware:
    current_session.is_authenticated = true;
    current_session.email = email;
    current_session.role = role;
    current_session.name = name;

    // Cannot use the same conn!
    mysql_free_result(prepare_meta_result);
    mysql_stmt_close(stmt);

    // ================= FETCH CREDIT CARD INFO =================
    const char *card_sql = "SELECT card_number, balance, is_active FROM Credit_card WHERE user_id = ? LIMIT 1";
    stmt = mysql_stmt_init(conn);
    if (!stmt || mysql_stmt_prepare(stmt, card_sql, strlen(card_sql)))
    {
        cerr << "❌ Prepare card failed: " << mysql_stmt_error(stmt) << endl;
        mysql_close(conn);
        return false;
    }

    MYSQL_BIND card_param[1], card_result[3];
    memset(card_param, 0, sizeof(card_param));
    memset(card_result, 0, sizeof(card_result));

    card_param[0].buffer_type = MYSQL_TYPE_LONG;
    card_param[0].buffer = &id;

    char card_number[100], balance[100];
    int is_active;

    card_result[0].buffer_type = MYSQL_TYPE_STRING;
    card_result[0].buffer = card_number;
    card_result[0].buffer_length = sizeof(card_number);

    card_result[1].buffer_type = MYSQL_TYPE_STRING;
    card_result[1].buffer = balance;
    card_result[1].buffer_length = sizeof(balance);

    card_result[2].buffer_type = MYSQL_TYPE_LONG;
    card_result[2].buffer = &is_active;

    if (mysql_stmt_bind_param(stmt, card_param) ||
        mysql_stmt_bind_result(stmt, card_result) ||
        mysql_stmt_execute(stmt))
    {
        cerr << "❌ Card info query failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }

    if (mysql_stmt_fetch(stmt) == 0)
    {
        current_session.card_number = card_number;
        current_session.balance = balance;
        current_session.is_active = is_active;

        cout << "💳 Card Info Loaded:\n";
        cout << "Card Number: " << card_number << endl;
        cout << "Balance: " << balance << endl;
        cout << "Active: " << is_active << endl;
    }
    else
    {
        cerr << "⚠️ No card found for user_id: " << id << endl;
    }

    // Clean up
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
    my_ulonglong user_id = 0;

    // Inserting Default Fields.
    string profile_picture = "/pfp/default.png"; // Default avatar path
    string role = "user";                        // Role assigned to all new users
    int is_2fa_enabled = 0;                      // Disabled by default
    string two_fa_secret = "0";
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

    // Begin transaction

    mysql_autocommit(conn, 0);

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

    // Execute INSERT INTO Users
    if (mysql_stmt_bind_param(stmt, bind))
    {
        cerr << "❌ Bind failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }

    if (mysql_stmt_execute(stmt))
    {
        cerr << "❌ Execute (user insert) failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }

    string card_number = generateFormattedNumber();
    int pin = generate_four_digit_number();
    string encrypted_pin = encrypt_pin(to_string(pin), "../keys/public.pem");
    string balance = "1000";
    int is_active = 1;

    user_id = mysql_insert_id(conn);
    mysql_stmt_close(stmt);

    // Insert into Credit_card
    const char *card_sql = R"(
        INSERT INTO Credit_card (user_id, card_number, pin_hash, balance, is_active, created_at)
        VALUES (?, ?, ?, ?, ?, CURRENT_TIMESTAMP)
    )";

    stmt = mysql_stmt_init(conn);
    if (!stmt || mysql_stmt_prepare(stmt, card_sql, strlen(card_sql)))
    {
        cerr << "❌ Prepare card failed: " << mysql_stmt_error(stmt) << endl;
        mysql_rollback(conn);
        mysql_close(conn);
        return false;
    }

    MYSQL_BIND card_bind[5];
    memset(card_bind, 0, sizeof(card_bind));

    card_bind[0].buffer_type = MYSQL_TYPE_LONG;
    card_bind[0].buffer = &user_id;

    card_bind[1].buffer_type = MYSQL_TYPE_STRING;
    card_bind[1].buffer = (void *)card_number.c_str();
    card_bind[1].buffer_length = card_number.length();

    card_bind[2].buffer_type = MYSQL_TYPE_STRING;
    card_bind[2].buffer = (void *)encrypted_pin.c_str();
    card_bind[2].buffer_length = encrypted_pin.length();

    card_bind[3].buffer_type = MYSQL_TYPE_STRING;
    card_bind[3].buffer = (void *)balance.c_str();
    card_bind[3].buffer_length = balance.length();

    card_bind[4].buffer_type = MYSQL_TYPE_LONG;
    card_bind[4].buffer = &is_active;

    if (mysql_stmt_bind_param(stmt, card_bind))
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
    mysql_commit(conn);
    mysql_close(conn);
    return true;
}