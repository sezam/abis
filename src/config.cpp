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

string logging_path("../log");
string logging_level("info");

float threshold_min = 0.18f;
float threshold_max = 0.8f;
float ir_threshold = 0.54f;
float fish_threshold = 0.32f;
float ir_threshold_select = 0.8f;
float ensembled_threshold = 0.5f;

int finger_min_points = 12;
int finger_min_quality = 60;
int finger_min_goodpoints = 60;

inline void PrintVariableMap(const po::variables_map vm) {
	for (const auto& it : vm) {
		std::cout << "> " << it.first;
		if (((boost::any)it.second.value()).empty()) {
			std::cout << "(empty)";
		}
		if (vm[it.first].defaulted() || it.second.defaulted()) {
			std::cout << "(default)";
		}
		std::cout << "=";

		bool is_char;
		try {
			boost::any_cast<const char*>(it.second.value());
			is_char = true;
		}
		catch (const boost::bad_any_cast&) {
			is_char = false;
		}
		bool is_str;
		try {
			boost::any_cast<std::string>(it.second.value());
			is_str = true;
		}
		catch (const boost::bad_any_cast&) {
			is_str = false;
		}

		if (((boost::any)it.second.value()).type() == typeid(int)) {
			std::cout << vm[it.first].as<int>() << std::endl;
		}
		else if (((boost::any)it.second.value()).type() == typeid(bool)) {
			std::cout << vm[it.first].as<bool>() << std::endl;
		}
		else if (((boost::any)it.second.value()).type() == typeid(double)) {
			std::cout << vm[it.first].as<double>() << std::endl;
		}
		else if (is_char) {
			std::cout << vm[it.first].as<const char* >() << std::endl;
		}
		else if (is_str) {
			std::string temp = vm[it.first].as<std::string>();
			if (temp.size()) {
				std::cout << temp << std::endl;
			}
			else {
				std::cout << "true" << std::endl;
			}
		}
		else { // Assumes that the only remainder is vector<string>
			try {
				std::vector<std::string> vect = vm[it.first].as<std::vector<std::string> >();
				uint i = 0;
				for (std::vector<std::string>::iterator oit = vect.begin();
					oit != vect.end(); oit++, ++i) {
					std::cout << "\r> " << it.first << "[" << i << "]=" << (*oit) << std::endl;
				}
			}
			catch (const boost::bad_any_cast&) {
				std::cout << "UnknownType(" << ((boost::any)it.second.value()).type().name() << ")" << std::endl;
			}
		}
	}
}

void load_settings(char* path)
{
	if (path != nullptr)
	{
		po::options_description desc("settings");
		desc.add_options()
			("extract.host", po::value< string >(&extract_host), "hostname, ip")
			("extract.port_start", po::value< int >(&extract_port_start), "start port number")
			("extract.port_count", po::value< int >(&extract_port_count), "ports used")
			("logging.file", po::value< string >(&logging_path), "logging file")
			("logging.level", po::value< string >(&logging_level), "logging level")
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
			("finger.param", po::value< string >(&finger_param), "param table")
			("finger.min_points", po::value< int >(&finger_min_points), "finger min points")
			("finger.min_quality", po::value< int >(&finger_min_quality), "finger min quality")
			("finger.min_good", po::value< int >(&finger_min_goodpoints), "finger min goodpoints")
			("liveface.threshold_min", po::value< float >(&threshold_min), "threshold_min")
			("liveface.threshold_max", po::value< float >(&threshold_max), "threshold_max")
			("liveface.ir_threshold", po::value< float >(&ir_threshold), "ir_threshold")
			("liveface.fish_threshold", po::value< float >(&fish_threshold), "fish_threshold")
			("liveface.ir_threshold_select", po::value< float >(&ir_threshold_select), "ir_threshold_select")
			("liveface.ensembled_threshold", po::value< float >(&ensembled_threshold), "ensembled_threshold");

		po::variables_map vm;
		po::store(po::parse_config_file(path, desc), vm);
		notify(vm);

		PrintVariableMap(vm);
	}
}