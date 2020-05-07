#include "AbisRest.h"

string extract_host("localhost");
int extract_port_start = 10080;
int extract_port_count = 4;

string postgres_host("localhost");
string postgres_port("5432");
string postgres_db("dbABIS");
string postgres_user("postgres");
string postgres_pwd("postgres");

int face_parts = 8;
string face_vector("faces_vector");
string face_index("faces_index");
string face_param("faces_param");

int finger_parts = 8;
string finger_vector("finger_vector");
string finger_index("finger_index");
string finger_param("finger_param");

void load_settings()
{
    po::options_description desc("settings");
    desc.add_options()
        ("extract.host", po::value< string >(&extract_host), "hostname, ip")
        ("extract.port_start", po::value< int >(&extract_port_start), "start port number")
        ("extract.port_count", po::value< int >(&extract_port_count), "ports used")
        ("postgres.host", po::value< string >(&postgres_host), "hostname, ip")
        ("postgres.port", po::value< string >(&postgres_port), "port number")
        ("postgres.db", po::value< string >(&postgres_db), "db name")
        ("postgres.user", po::value< string >(&postgres_user), "user")
        ("postgres.pwd", po::value< string >(&postgres_pwd), "password")
        ("face.parts", po::value< int >(&face_parts), "parts")
        ("face.vector", po::value< string >(&face_vector), "vector table")
        ("face.index", po::value< string >(&face_index), "index table")
        ("face.param", po::value< string >(&face_param), "param table")
        ("finger.parts", po::value< int >(&finger_parts), "parts")
        ("finger.vector", po::value< string >(&finger_vector), "vector table")
        ("finger.index", po::value< string >(&finger_index), "index table")
        ("finger.param", po::value< string >(&finger_param), "param table");

    po::variables_map vm;
    po::store(po::parse_config_file("../config/home.ini", desc), vm);
    notify(vm);
}