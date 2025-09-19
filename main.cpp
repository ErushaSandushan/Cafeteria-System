#include <iostream>
#include <sqlite3.h>
#include <openssl/evp.h>
#include <sstream>
#include <iomanip>

/*
Author      : Erusha Sandushan (Mast3rB0y)
Date        : 17/08/2025
Description : A C++ and SQL-based cafeteria management system built as a learning project to strengthen my C++ skills and practice database integration.
*/
class DB
{
    public:
        sqlite3* db;
        int connect()
        {
            int exit = 0;
            exit = sqlite3_open("./DB/cafeteria.db",&db);
                if (exit) {
                std::cerr << "[-] Error opening DB " << sqlite3_errmsg(db) << std::endl;
                return (-1);
            }
            return 0;
        }
        static int callback(void *data, int argc, char **argv, char **azColName){
            int i; 
            for(i = 0; i<argc; i++){
                std::cout << (argv[i] ? argv[i] : "NULL") << "\t\t";
            }
            
            std::cout << "\n";
            return 0;
            }
        void read_data()
        {   connect();
            char* zErrMsg = 0;
            char* sql = "SELECT * from Menu;";
            int rc;
            std::cout <<"Item            Name                    Price \n";
            std::cout<<"--------------------------------------------------------------\n";
            rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
            std::cout<<"--------------------------------------------------------------\n";   
            return;
        }
        void insert_order(std::string& Item,int Qty){
            connect(); 
            sqlite3_stmt* stmt;
            const char* price_sql = "SELECT Price FROM Menu WHERE Item = ?;";
            if (sqlite3_prepare_v2(db, price_sql, -1, &stmt, nullptr) != SQLITE_OK) {
                std::cerr << "Failed to prepare price query\n";
                return;
            }
                sqlite3_bind_text(stmt, 1, Item.c_str(), -1, SQLITE_TRANSIENT);
            double price = 0.0;
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                price = sqlite3_column_double(stmt, 0);
            } else {
                std::cerr << "Item not found in Menu\n";
                sqlite3_finalize(stmt);
                return;
            }
            sqlite3_finalize(stmt);

            double total = price * Qty;
            const char* sql = "INSERT INTO Orders(Item, Qty,'Total Bill') VALUES (?, ?,?);";

            if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
                std::cerr << "Failed to prepare statement\n";
                return;
            }

            sqlite3_bind_text(stmt, 1, Item.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(stmt, 2, Qty);
            sqlite3_bind_double(stmt, 3, total);

            if (sqlite3_step(stmt) != SQLITE_DONE) {
                std::cerr << "Insert failed: " << sqlite3_errmsg(db) << "\n";
            }

            sqlite3_finalize(stmt);
            std::cout << "Total Bill : " << total <<std::endl;

        }
        std::string md5Encrypt(const std::string& input) {
            unsigned char digest[EVP_MAX_MD_SIZE];
            unsigned int digest_len = 0;

            EVP_MD_CTX* ctx = EVP_MD_CTX_new();
            EVP_DigestInit_ex(ctx, EVP_md5(), nullptr);
            EVP_DigestUpdate(ctx, input.c_str(), input.size());
            EVP_DigestFinal_ex(ctx, digest, &digest_len);
            EVP_MD_CTX_free(ctx);

            std::stringstream ss;
            for (unsigned int i = 0; i < digest_len; ++i)
                ss << std::hex << std::setw(2) << std::setfill('0') << (int)digest[i];
            return ss.str();
        }
};
class Cafeteria 
{
    private:
        DB& database;
    public:
        Cafeteria(DB& db) : database(db) {}
        void banner()
            {
                std::cout << "=============================================================" << std::endl;
                std::cout << "         Welcome to Trincomalee Campus Cafeteria " << std::endl;
                std::cout << "=============================================================\n" << std::endl;
            }

        int login()
            {
                std::string uname;
                std::string pass;
                std::cout << "Username : ";
                std::cin >> uname;
                std::cout << "Password : ";
                std::cin >> pass;
                std::cout << uname<<pass;
                return 0;
            }

        void order() // Add a input validator
            {
                int item;
                int qty;
                std::cout << "Item No : ";
                std::cin >> item;
                std::cout << "Quantity : ";
                std::cin >> qty;
                //std::cout << item << qty;
                std::string item1 = std::to_string(item);
                database.insert_order(item1,qty);
                //std::cin.ignore();
                //std::cin.get();
                std::cout << "\nOrder Again?\n"
                             "1.Yes\n"
                             "2.No\n";
                int answer;
                std::cout<<"> ";
                std::cin>>answer;
                if(std::cin.fail()){
                    std::cout << "Not a number";
                    exit(1);
                }else if(answer == 1){
                    order();
                }else{
                    std::cout<<"\nThank You !";
                    std::cin.ignore();
                    std::cin.get();
                    exit(1);
                }
            }
        void list() // read data from DB;
           {
               database.read_data();
            }


        void ops()
            {
                int selected;
                banner();
                list();
                std::string menu = "\nPress choose an option\n"
                                    "1. Place an Order \n"
                                    "2. Login (Admin) \n"
                                    "3. Exit \n";
                std::cout << menu << std::endl;
                std::cout << "> ";
                std::cin >> selected;
                if(std::cin.fail()){
                    std::cout << "[-] Invalied input, Please enter number in list";
                    exit(1);
                }else if (selected == 1)
                {
                    order();
                }else if (selected == 2){
                    //login();
                    std::cout<< "Login is Under construction";
                    exit(0);
                }else if (selected ==3){
                    std::cout << "\nThank You!, Come again.\n";
                    exit(0);
                }else
                {
                    std::cout << "[-] Error, Please enter number in list";
                }

            }
       
};

int main()
{
    DB db;
    db.connect();
    Cafeteria cafe(db);
    cafe.ops();

    return 0;
}

