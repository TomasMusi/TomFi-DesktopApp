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

// Base64 decode
vector<unsigned char> base64_decode(const string &encoded)
{
    BIO *bio, *b64;
    int decodeLen = encoded.length();
    vector<unsigned char> buffer(decodeLen);

    bio = BIO_new_mem_buf(encoded.data(), encoded.length());
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);

    int length = BIO_read(bio, buffer.data(), encoded.length());
    buffer.resize(length);

    BIO_free_all(bio);
    return buffer;
}

// Decrypt PIN using private key
string decrypt_pin(const string &encrypted_pin_base64, const string &privkey_path)
{
    vector<unsigned char> encrypted_data = base64_decode(encrypted_pin_base64);

    FILE *privkey_file = fopen(privkey_path.c_str(), "r");
    if (!privkey_file)
    {
        cerr << "Failed to open private key file" << endl;
        return "";
    }

    RSA *rsa = PEM_read_RSAPrivateKey(privkey_file, nullptr, nullptr, nullptr);
    fclose(privkey_file);
    if (!rsa)
    {
        cerr << "Failed to load private key" << endl;
        return "";
    }

    vector<unsigned char> decrypted(RSA_size(rsa));
    int result = RSA_private_decrypt(
        encrypted_data.size(),
        encrypted_data.data(),
        decrypted.data(),
        rsa,
        RSA_PKCS1_OAEP_PADDING);
    RSA_free(rsa);

    if (result == -1)
    {
        cerr << " PIN decryption failed" << endl;
        return "";
    }

    return string(reinterpret_cast<char *>(decrypted.data()), result);
}

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

    string result(buffer_ptr->data, buffer_ptr->length);
    BIO_free_all(b64);
    return result;
}

// Encrypt PIN using public key
string encrypt_pin(const string &pin, const string &pubkey_path)
{
    FILE *pubkey_file = fopen(pubkey_path.c_str(), "r");
    if (!pubkey_file)
    {
        cerr << "Failed to open public key" << endl;
        return "";
    }

    RSA *rsa = PEM_read_RSA_PUBKEY(pubkey_file, NULL, NULL, NULL);
    fclose(pubkey_file);
    if (!rsa)
    {
        cerr << "Failed to read public key" << endl;
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
        cerr << "Encryption failed" << endl;
        return "";
    }

    return base64_encode(encrypted, result);
}

LoginCheckResult verify_login(const string &email, const string &password)
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
        cerr << "mysql_init() failed" << endl;
        return {false, -1, false};
    }

    if (!mysql_real_connect(conn,
                            DATABASE_IP.c_str(),
                            DATABASE_USER.c_str(),
                            DATABASE_PASSWORD.c_str(),
                            DATABASE_NAME.c_str(),
                            stoi(DATABASE_PORT),
                            NULL, 0))
    {
        cerr << "Connection failed: " << mysql_error(conn) << endl;
        mysql_close(conn);
        return {false, -1, false};
    }

    // 3. Prepare SQL query with 1 placeholder (?)
    const char *sql = "SELECT * FROM Users WHERE email = ?"; // this is the question we want to ask database: ,,Hey database, give me all the info about the user whose email is waht I'm going to give you." The ? is a placeholder. We will plug the real email in later. This protects us from bad hackers who try to break into systems using tricks.
    stmt = mysql_stmt_init(conn);
    if (!stmt)
    {
        cerr << "mysql_stmt_init() failed" << endl;
        mysql_close(conn);
        return {false, -1, false};
    }

    if (mysql_stmt_prepare(stmt, sql, strlen(sql)))
    {
        cerr << "Prepare failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return {false, -1, false};
    }

    // 4. Bind input parameter securely (prevents SQL injection)
    memset(param_bind, 0, sizeof(param_bind));

    // This part says: ,,Okay, put the email the user typed into that ? placeholder."
    param_bind[0].buffer_type = MYSQL_TYPE_STRING;
    param_bind[0].buffer = (void *)email.c_str();
    param_bind[0].buffer_length = email.length();

    if (mysql_stmt_bind_param(stmt, param_bind))
    {
        cerr << " Bind failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return {false, -1, false};
    }

    // 5. Execute the query. This actually sends the question to the database and waits for a reply.
    if (mysql_stmt_execute(stmt))
    {
        cerr << " Execute failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return {false, -1, false};
    }

    // 6. Bind result columns to output variables. This gets info about what the answer will look like (how many columns, what type of data etc.)
    prepare_meta_result = mysql_stmt_result_metadata(stmt);
    if (!prepare_meta_result)
    {
        cerr << " Failed to get result metadata" << endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return {false, -1, false};
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
        cerr << " Bind result failed: " << mysql_stmt_error(stmt) << endl;
        mysql_free_result(prepare_meta_result);
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return {false, -1, false};
    }

    // 7. Fetch the result. This actually fetches the row from the database where the email matched.
    if (mysql_stmt_fetch(stmt) != 0)
    {
        cerr << "⚠️ No user found with email: " << email << endl;
        mysql_free_result(prepare_meta_result);
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return {false, -1, false};
    }

    // 8. Check password
    const char *hashed = password_db;
    const char *plain = password.c_str();

    if (bcrypt_checkpw(plain, hashed) != 0)
    {
        cerr << " Incorrect password!" << endl;
        mysql_free_result(prepare_meta_result);
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return {false, -1, false};
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
        cerr << " Prepare card failed: " << mysql_stmt_error(stmt) << endl;
        mysql_close(conn);
        return {false, -1, false};
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
        cerr << " Card info query failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return {false, -1, false};
    }

    if (mysql_stmt_fetch(stmt) == 0)
    {
        current_session.card_number = card_number;
        current_session.balance = balance;
        current_session.is_active = is_active;
        current_session.user_id = id;

        cout << "💳 Card Info Loaded:" << endl;
        cout << "Card Number: " << card_number << endl;
        cout << "Balance: " << balance << endl;
        cout << "Active: " << is_active << endl;
        cout << "User id:" << id << endl;
    }
    else
    {
        cerr << "No card found for user_id: " << id << endl;
    }

    // Clean up
    mysql_stmt_close(stmt);

    // ================= FETCH TRANSACTIONS =================
    const char *tx_sql = R"(
    SELECT timestamp, description, category, amount, direction
    FROM Transactions
    WHERE sender_account_id = ? OR receiver_user_id = ?
    ORDER BY timestamp DESC
)";

    stmt = mysql_stmt_init(conn);
    if (!stmt || mysql_stmt_prepare(stmt, tx_sql, strlen(tx_sql)))
    {
        cerr << " Prepare transaction query failed: " << mysql_stmt_error(stmt) << endl;
        mysql_close(conn);
        return {true, id, twofa_enabled == 1};
    }

    // Bind input parameters
    MYSQL_BIND tx_param[2];
    memset(tx_param, 0, sizeof(tx_param));
    tx_param[0].buffer_type = MYSQL_TYPE_LONG;
    tx_param[0].buffer = &id;
    tx_param[1].buffer_type = MYSQL_TYPE_LONG;
    tx_param[1].buffer = &id;

    // Bind output fields
    char timestamp[32], description[226], category[64], amount[64], direction[10];

    MYSQL_BIND tx_result[5];
    memset(tx_result, 0, sizeof(tx_result));

    tx_result[0].buffer_type = MYSQL_TYPE_STRING;
    tx_result[0].buffer = timestamp;
    tx_result[0].buffer_length = sizeof(timestamp);

    tx_result[1].buffer_type = MYSQL_TYPE_STRING;
    tx_result[1].buffer = description;
    tx_result[1].buffer_length = sizeof(description);

    tx_result[2].buffer_type = MYSQL_TYPE_STRING;
    tx_result[2].buffer = category;
    tx_result[2].buffer_length = sizeof(category);

    tx_result[3].buffer_type = MYSQL_TYPE_STRING;
    tx_result[3].buffer = amount;
    tx_result[3].buffer_length = sizeof(amount);

    tx_result[4].buffer_type = MYSQL_TYPE_STRING;
    tx_result[4].buffer = direction;
    tx_result[4].buffer_length = sizeof(direction);

    // Execute and bind
    if (mysql_stmt_bind_param(stmt, tx_param) ||
        mysql_stmt_bind_result(stmt, tx_result) ||
        mysql_stmt_execute(stmt))
    {
        cerr << " Transaction fetch failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return {true, id, twofa_enabled == 1};
    }

    // Store results
    current_session.transactions.clear();
    while (mysql_stmt_fetch(stmt) == 0)
    {
        current_session.transactions.push_back(Transaction{
            string(timestamp),
            string(description),
            string(category),
            string(amount),
            string(direction)});
    }

    mysql_stmt_close(stmt);
    mysql_close(conn);
    return {true, id, twofa_enabled == 1};
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
    string created_at = "NOW()"; // We’ll let MySQL insert timestamp

    // Load env config

    string DATABASE_IP = env_vars["DATABASE_IP"];
    string DATABASE_USER = env_vars["DATABASE_USER"];
    string DATABASE_PASSWORD = env_vars["DATABASE_PASSWORD"];
    string DATABASE_NAME = env_vars["DATABASE_NAME"];
    string DATABASE_PORT = env_vars["DATABASE_PORT"];

    conn = mysql_init(NULL);
    if (!conn)
    {
        cerr << " mysql_init() failed" << endl;
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
        cerr << " Connection failed: " << mysql_error(conn) << endl;
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
        cerr << " mysql_stmt_init() failed for check\n";
        mysql_close(conn);
        return false;
    }

    if (mysql_stmt_prepare(stmt, check_sql, strlen(check_sql)))
    {
        cerr << " Prepare (check email) failed: " << mysql_stmt_error(stmt) << endl;
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
        cerr << " Bind (check email) failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }

    if (mysql_stmt_execute(stmt))
    {
        cerr << " Execute (check email) failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }

    if (mysql_stmt_store_result(stmt))
    {
        cerr << " Store result (check email) failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }

    if (mysql_stmt_num_rows(stmt) > 0)
    {
        cerr << "Email already exists: " << email << endl;
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
        cerr << " mysql_stmt_init() failed" << endl;
        mysql_close(conn);
        return false;
    }

    if (mysql_stmt_prepare(stmt, sql, strlen(sql)))
    {
        cerr << " Prepare failed: " << mysql_stmt_error(stmt) << endl;
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
        cerr << " Bind failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }

    if (mysql_stmt_execute(stmt))
    {
        cerr << " Execute (user insert) failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }

    string card_number = generateFormattedNumber();
    int pin = generate_four_digit_number();
    string encrypted_pin = encrypt_pin(to_string(pin), "keys/public.pem");
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
        cerr << " Prepare card failed: " << mysql_stmt_error(stmt) << endl;
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
        cerr << " Bind failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }

    if (mysql_stmt_execute(stmt))
    {
        cerr << " Execute failed: " << mysql_stmt_error(stmt) << endl;
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

// Credit card events.

// Change status of the card.

bool card_status_toggle(int user_id)
{
    MYSQL *conn;
    MYSQL_STMT *stmt;

    // load env data.
    string DATABASE_IP = env_vars["DATABASE_IP"];
    string DATABASE_USER = env_vars["DATABASE_USER"];
    string DATABASE_PASSWORD = env_vars["DATABASE_PASSWORD"];
    string DATABASE_NAME = env_vars["DATABASE_NAME"];
    string DATABASE_PORT = env_vars["DATABASE_PORT"];

    conn = mysql_init(NULL);
    if (!conn)
    {
        cerr << " mysql_init() failed" << endl;
        return false;
    }

    if (!mysql_real_connect(conn, DATABASE_IP.c_str(), DATABASE_USER.c_str(),
                            DATABASE_PASSWORD.c_str(), DATABASE_NAME.c_str(),
                            stoi(DATABASE_PORT), NULL, 0))
    {
        cerr << " Connection failed: " << mysql_error(conn) << endl;
        mysql_close(conn);
        return false;
    }

    // 1 → 0 or 0 → 1 (Chaging the values.)
    int new_status = current_session.is_active ? 0 : 1;

    const char *sql = R"(
        UPDATE Credit_card
        SET is_active = ?
        WHERE user_id = ?
    )";

    stmt = mysql_stmt_init(conn);
    if (!stmt || mysql_stmt_prepare(stmt, sql, strlen(sql)))
    {
        cerr << " Prepare failed: " << mysql_stmt_error(stmt) << endl;
        mysql_close(conn);
        return false;
    }

    MYSQL_BIND bind[2];
    memset(bind, 0, sizeof(bind));

    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = &new_status;

    bind[1].buffer_type = MYSQL_TYPE_LONG;
    bind[1].buffer = &user_id;

    if (mysql_stmt_bind_param(stmt, bind))
    {
        cerr << " Bind failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }

    if (mysql_stmt_execute(stmt))
    {
        cerr << " Execute failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }

    // ✅ Update session state after DB success
    current_session.is_active = new_status;

    mysql_commit(conn);
    mysql_stmt_close(stmt);
    mysql_close(conn);
    return true;
}

// Depositing money.

bool add_funds_balance(int user_id, int amount)
{
    MYSQL *conn;
    MYSQL_STMT *stmt;

    string DATABASE_IP = env_vars["DATABASE_IP"];
    string DATABASE_USER = env_vars["DATABASE_USER"];
    string DATABASE_PASSWORD = env_vars["DATABASE_PASSWORD"];
    string DATABASE_NAME = env_vars["DATABASE_NAME"];
    string DATABASE_PORT = env_vars["DATABASE_PORT"];

    conn = mysql_init(NULL);
    if (!conn)
    {
        cerr << " mysql_init() failed" << endl;
        return false;
    }

    if (!mysql_real_connect(conn, DATABASE_IP.c_str(), DATABASE_USER.c_str(),
                            DATABASE_PASSWORD.c_str(), DATABASE_NAME.c_str(),
                            stoi(DATABASE_PORT), NULL, 0))
    {
        cerr << " Connection failed: " << mysql_error(conn) << endl;
        mysql_close(conn);
        return false;
    }

    // Step 1: Fetch current balance and card_number
    const char *select_sql = "SELECT balance, card_number FROM Credit_card WHERE user_id = ?";
    stmt = mysql_stmt_init(conn);
    if (!stmt || mysql_stmt_prepare(stmt, select_sql, strlen(select_sql)))
    {
        cerr << " Prepare (select) failed: " << mysql_stmt_error(stmt) << endl;
        mysql_close(conn);
        return false;
    }

    MYSQL_BIND param[1], result[2];
    memset(param, 0, sizeof(param));
    memset(result, 0, sizeof(result));

    param[0].buffer_type = MYSQL_TYPE_LONG;
    param[0].buffer = &user_id;

    char balance_buffer[64];
    char card_number[64];

    result[0].buffer_type = MYSQL_TYPE_STRING;
    result[0].buffer = balance_buffer;
    result[0].buffer_length = sizeof(balance_buffer);

    result[1].buffer_type = MYSQL_TYPE_STRING;
    result[1].buffer = card_number;
    result[1].buffer_length = sizeof(card_number);

    if (mysql_stmt_bind_param(stmt, param) ||
        mysql_stmt_bind_result(stmt, result) ||
        mysql_stmt_execute(stmt) ||
        mysql_stmt_fetch(stmt))
    {
        cerr << " Fetch balance/card_number failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }

    mysql_stmt_close(stmt);

    int current_balance = atoi(balance_buffer);
    int new_balance = current_balance + amount;

    // Step 2: Update balance
    const char *update_sql = "UPDATE Credit_card SET balance = ? WHERE user_id = ?";
    stmt = mysql_stmt_init(conn);
    if (!stmt || mysql_stmt_prepare(stmt, update_sql, strlen(update_sql)))
    {
        cerr << " Prepare (update) failed: " << mysql_stmt_error(stmt) << endl;
        mysql_close(conn);
        return false;
    }

    MYSQL_BIND update_bind[2];
    memset(update_bind, 0, sizeof(update_bind));

    update_bind[0].buffer_type = MYSQL_TYPE_LONG;
    update_bind[0].buffer = &new_balance;

    update_bind[1].buffer_type = MYSQL_TYPE_LONG;
    update_bind[1].buffer = &user_id;

    if (mysql_stmt_bind_param(stmt, update_bind) ||
        mysql_stmt_execute(stmt))
    {
        cerr << " Update balance failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }

    mysql_stmt_close(stmt);

    // Step 3: Insert into Transactions table
    const char *insert_tx_sql = R"(
        INSERT INTO Transactions (
            sender_account_id,
            receiver_account,
            receiver_user_id,
            description,
            reciever_name,
            category,
            amount,
            direction,
            timestamp
        ) VALUES (?, ?, NULL, 'deposit', 'deposit', 'other', ?, 'in', CURRENT_TIMESTAMP)
    )";

    stmt = mysql_stmt_init(conn);
    if (!stmt || mysql_stmt_prepare(stmt, insert_tx_sql, strlen(insert_tx_sql)))
    {
        cerr << " Prepare (insert tx) failed: " << mysql_stmt_error(stmt) << endl;
        mysql_close(conn);
        return false;
    }

    string card_number_str(card_number);
    string amount_str = to_string(amount);

    MYSQL_BIND tx_bind[3];
    memset(tx_bind, 0, sizeof(tx_bind));

    tx_bind[0].buffer_type = MYSQL_TYPE_LONG;
    tx_bind[0].buffer = &user_id;

    tx_bind[1].buffer_type = MYSQL_TYPE_STRING;
    tx_bind[1].buffer = (void *)card_number_str.c_str();
    tx_bind[1].buffer_length = card_number_str.length();

    tx_bind[2].buffer_type = MYSQL_TYPE_STRING;
    tx_bind[2].buffer = (void *)amount_str.c_str();
    tx_bind[2].buffer_length = amount_str.length();

    if (mysql_stmt_bind_param(stmt, tx_bind) || mysql_stmt_execute(stmt))
    {
        cerr << " Insert transaction failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }

    mysql_commit(conn);
    mysql_stmt_close(stmt);
    mysql_close(conn);

    // Update session
    current_session.balance = to_string(new_balance);
    return true;
}

// See Credit Card Pin.

string get_decrypted_pin(string user_input_password, int user_id)
{
    MYSQL *conn;
    MYSQL_STMT *stmt;

    string DATABASE_IP = env_vars["DATABASE_IP"];
    string DATABASE_USER = env_vars["DATABASE_USER"];
    string DATABASE_PASSWORD = env_vars["DATABASE_PASSWORD"];
    string DATABASE_NAME = env_vars["DATABASE_NAME"];
    string DATABASE_PORT = env_vars["DATABASE_PORT"];

    conn = mysql_init(NULL);
    if (!conn)
    {
        cerr << "mysql_init() failed" << endl;
        return "";
    }

    if (!mysql_real_connect(conn, DATABASE_IP.c_str(), DATABASE_USER.c_str(), DATABASE_PASSWORD.c_str(), DATABASE_NAME.c_str(), stoi(DATABASE_PORT), NULL, 0))
    {
        cerr << "Connection failed: " << mysql_error(conn) << endl;
        mysql_close(conn);
        return "";
    }

    // Step 2: Get hashed password from DB
    const char *pass_sql = "SELECT password FROM Users WHERE id = ?";
    stmt = mysql_stmt_init(conn);
    if (!stmt || mysql_stmt_prepare(stmt, pass_sql, strlen(pass_sql)))
    {
        cerr << "Prepare (password) failed: " << mysql_stmt_error(stmt) << endl;
        mysql_close(conn);
        return "";
    }

    MYSQL_BIND pass_bind[1], pass_result[1];
    memset(pass_bind, 0, sizeof(pass_bind));
    memset(pass_result, 0, sizeof(pass_result));

    pass_bind[0].buffer_type = MYSQL_TYPE_LONG;
    pass_bind[0].buffer = &user_id;

    char hash_buf[100];
    pass_result[0].buffer_type = MYSQL_TYPE_STRING;
    pass_result[0].buffer = hash_buf;
    pass_result[0].buffer_length = sizeof(hash_buf);

    if (mysql_stmt_bind_param(stmt, pass_bind) ||
        mysql_stmt_bind_result(stmt, pass_result) ||
        mysql_stmt_execute(stmt) ||
        mysql_stmt_fetch(stmt))
    {
        cerr << "Failed to fetch password hash: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return "";
    }

    mysql_stmt_close(stmt);

    // Step 3: Bcrypt verify

    const char *plain = user_input_password.c_str();

    if (bcrypt_checkpw(plain, hash_buf) != 0)
    {
        cerr << "Incorrect password." << endl;
        mysql_close(conn);
        return "";
    }

    // Step 4: Now get PIN
    const char *pin_sql = "SELECT pin_hash FROM Credit_card WHERE user_id = ?";
    stmt = mysql_stmt_init(conn);
    if (!stmt || mysql_stmt_prepare(stmt, pin_sql, strlen(pin_sql)))
    {
        cerr << "Prepare (pin) failed: " << mysql_stmt_error(stmt) << endl;
        mysql_close(conn);
        return "";
    }

    MYSQL_BIND pin_bind[1], pin_result[1];
    memset(pin_bind, 0, sizeof(pin_bind));
    memset(pin_result, 0, sizeof(pin_result));

    pin_bind[0].buffer_type = MYSQL_TYPE_LONG;
    pin_bind[0].buffer = &user_id;

    char pin_buf[512];
    pin_result[0].buffer_type = MYSQL_TYPE_STRING;
    pin_result[0].buffer = pin_buf;
    pin_result[0].buffer_length = sizeof(pin_buf);

    if (mysql_stmt_bind_param(stmt, pin_bind) ||
        mysql_stmt_bind_result(stmt, pin_result) ||
        mysql_stmt_execute(stmt) ||
        mysql_stmt_fetch(stmt))
    {
        cerr << "Failed to fetch pin_hash: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return "";
    }

    mysql_stmt_close(stmt);
    mysql_close(conn);

    string encrypted_pin_base64(pin_buf);
    return decrypt_pin(encrypted_pin_base64, "keys/private.pem");
}

// 2FA

// Create, just parsing the generated base32 into two_fa_secret
bool store_2fa_secret_to_db(int user_id, const string &secret)
{
    MYSQL *conn;
    MYSQL_STMT *stmt;

    // Load environment config
    string DATABASE_IP = env_vars["DATABASE_IP"];
    string DATABASE_USER = env_vars["DATABASE_USER"];
    string DATABASE_PASSWORD = env_vars["DATABASE_PASSWORD"];
    string DATABASE_NAME = env_vars["DATABASE_NAME"];
    string DATABASE_PORT = env_vars["DATABASE_PORT"];

    conn = mysql_init(NULL);
    if (!conn)
    {
        cerr << "mysql_init() failed" << endl;
        return false;
    }

    if (!mysql_real_connect(conn, DATABASE_IP.c_str(), DATABASE_USER.c_str(),
                            DATABASE_PASSWORD.c_str(), DATABASE_NAME.c_str(),
                            stoi(DATABASE_PORT), NULL, 0))
    {
        cerr << "Connection failed: " << mysql_error(conn) << endl;
        mysql_close(conn);
        return false;
    }

    // only update two_fa_secret
    const char *sql = "UPDATE Users SET two_fa_secret = ? WHERE id = ?";
    stmt = mysql_stmt_init(conn);
    if (!stmt || mysql_stmt_prepare(stmt, sql, strlen(sql)))
    {
        cerr << "Prepare failed: " << mysql_stmt_error(stmt) << endl;
        mysql_close(conn);
        return false;
    }

    MYSQL_BIND bind[2];
    memset(bind, 0, sizeof(bind));

    // Bind 2FA secret
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (void *)secret.c_str();
    bind[0].buffer_length = secret.length();

    // Bind user ID
    bind[1].buffer_type = MYSQL_TYPE_LONG;
    bind[1].buffer = &user_id;

    if (mysql_stmt_bind_param(stmt, bind))
    {
        cerr << "Bind failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }

    if (mysql_stmt_execute(stmt))
    {
        cerr << "Execute failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }

    mysql_stmt_close(stmt);
    mysql_close(conn);

    cout << "2FA secret stored successfully for user_id: " << user_id << endl;
    return true;
}

bool create_payment(const int sender_account_id,
                    const string &receiver_card_number,
                    const string &description,
                    const string &receiver_name,
                    const string &category,
                    const string &amount,
                    const string &user_pin)
{
    MYSQL *conn;
    MYSQL_STMT *stmt;

    string DATABASE_IP = env_vars["DATABASE_IP"];
    string DATABASE_USER = env_vars["DATABASE_USER"];
    string DATABASE_PASSWORD = env_vars["DATABASE_PASSWORD"];
    string DATABASE_NAME = env_vars["DATABASE_NAME"];
    string DATABASE_PORT = env_vars["DATABASE_PORT"];

    conn = mysql_init(NULL);
    if (!conn)
    {
        cerr << "mysql_init() failed" << endl;
        return false;
    }

    if (!mysql_real_connect(conn, DATABASE_IP.c_str(), DATABASE_USER.c_str(), DATABASE_PASSWORD.c_str(), DATABASE_NAME.c_str(), stoi(DATABASE_PORT), NULL, 0))
    {
        cerr << "Connection failed: " << mysql_error(conn) << endl;
        mysql_close(conn);
        return false;
    }

    // Step 1: Check if card is active
    const char *check_active_sql = "SELECT is_active, pin_hash, balance FROM Credit_card WHERE user_id = ?";
    stmt = mysql_stmt_init(conn);
    if (!stmt || mysql_stmt_prepare(stmt, check_active_sql, strlen(check_active_sql)))
    {
        cerr << "Prepare failed: " << mysql_stmt_error(stmt) << endl;
        mysql_close(conn);
        return false;
    }

    MYSQL_BIND param[1], result[3];
    memset(param, 0, sizeof(param));
    memset(result, 0, sizeof(result));

    param[0].buffer_type = MYSQL_TYPE_LONG;
    param[0].buffer = (void *)&sender_account_id;

    int is_active;
    char pin_hash[800];
    char balance_buf[80];

    result[0].buffer_type = MYSQL_TYPE_LONG;
    result[0].buffer = &is_active;

    result[1].buffer_type = MYSQL_TYPE_STRING;
    result[1].buffer = pin_hash;
    result[1].buffer_length = sizeof(pin_hash);

    result[2].buffer_type = MYSQL_TYPE_STRING;
    result[2].buffer = balance_buf;
    result[2].buffer_length = sizeof(balance_buf);

    if (mysql_stmt_bind_param(stmt, param) ||
        mysql_stmt_bind_result(stmt, result) ||
        mysql_stmt_execute(stmt) ||
        mysql_stmt_fetch(stmt))
    {
        cerr << "Card lookup failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }
    mysql_stmt_close(stmt);

    if (is_active != 1)
    {
        cerr << "Card is not active." << endl;
        mysql_close(conn);
        return false;
    }

    // Step 2: Validate PIN
    string decrypted_pin = decrypt_pin(pin_hash, "keys/private.pem");
    if (decrypted_pin != user_pin)
    {
        cerr << "Incorrect PIN." << endl;
        mysql_close(conn);
        return false;
    }

    // Step 3: Check balance and subtract
    double current_balance = stod(balance_buf);
    double transaction_amount = stod(amount);

    if (transaction_amount > current_balance)
    {
        cerr << "Insufficient balance." << endl;
        mysql_close(conn);
        return false;
    }

    double new_balance = current_balance - transaction_amount;
    string new_balance_str = to_string(new_balance);

    const char *update_balance_sql = "UPDATE Credit_card SET balance = ? WHERE user_id = ?";
    stmt = mysql_stmt_init(conn);
    if (!stmt || mysql_stmt_prepare(stmt, update_balance_sql, strlen(update_balance_sql)))
    {
        cerr << "Prepare update balance failed: " << mysql_stmt_error(stmt) << endl;
        mysql_close(conn);
        return false;
    }

    MYSQL_BIND update_bind[2];
    memset(update_bind, 0, sizeof(update_bind));
    update_bind[0].buffer_type = MYSQL_TYPE_STRING;
    update_bind[0].buffer = (void *)new_balance_str.c_str();
    update_bind[0].buffer_length = new_balance_str.length();
    update_bind[1].buffer_type = MYSQL_TYPE_LONG;
    update_bind[1].buffer = (void *)&sender_account_id;

    if (mysql_stmt_bind_param(stmt, update_bind) || mysql_stmt_execute(stmt))
    {
        cerr << "Failed to update balance: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }
    mysql_stmt_close(stmt);

    // Step 4: Insert transaction
    const char *insert_sql = R"(
        INSERT INTO Transactions (
            sender_account_id,
            receiver_account,
            receiver_user_id,
            description,
            reciever_name,
            category,
            amount,
            direction,
            timestamp
        ) VALUES (?, ?, NULL, ?, ?, ?, ?, 'out', CURRENT_TIMESTAMP)
    )";

    stmt = mysql_stmt_init(conn);
    if (!stmt || mysql_stmt_prepare(stmt, insert_sql, strlen(insert_sql)))
    {
        cerr << "Prepare transaction insert failed: " << mysql_stmt_error(stmt) << endl;
        mysql_close(conn);
        return false;
    }

    MYSQL_BIND tx_bind[6];
    memset(tx_bind, 0, sizeof(tx_bind));

    tx_bind[0].buffer_type = MYSQL_TYPE_LONG;
    tx_bind[0].buffer = (void *)&sender_account_id;

    tx_bind[1].buffer_type = MYSQL_TYPE_STRING;
    tx_bind[1].buffer = (void *)receiver_card_number.c_str();
    tx_bind[1].buffer_length = receiver_card_number.length();

    tx_bind[2].buffer_type = MYSQL_TYPE_STRING;
    tx_bind[2].buffer = (void *)description.c_str();
    tx_bind[2].buffer_length = description.length();

    tx_bind[3].buffer_type = MYSQL_TYPE_STRING;
    tx_bind[3].buffer = (void *)receiver_name.c_str();
    tx_bind[3].buffer_length = receiver_name.length();

    tx_bind[4].buffer_type = MYSQL_TYPE_STRING;
    tx_bind[4].buffer = (void *)category.c_str();
    tx_bind[4].buffer_length = category.length();

    tx_bind[5].buffer_type = MYSQL_TYPE_STRING;
    tx_bind[5].buffer = (void *)amount.c_str();
    tx_bind[5].buffer_length = amount.length();

    if (mysql_stmt_bind_param(stmt, tx_bind) || mysql_stmt_execute(stmt))
    {
        cerr << "Insert transaction failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }

    mysql_commit(conn);
    mysql_stmt_close(stmt);
    mysql_close(conn);
    return true;
}
