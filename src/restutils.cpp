#include "AbisRest.h"
#include "BioTemplate.h"
#include "restutils.h"
#include "ebsclient.h"
#include "fplibclient.h"
#include "liveclient.h"

void JSON_EXCEPTION(web::json::value& obj, const string msg)
{
	obj[ELEMENT_ERROR] = json::value::string(conversions::to_string_t(msg));
	obj[ELEMENT_RESULT] = json::value::boolean(false);
	BOOST_LOG_TRIVIAL(error) << "Exception: " << msg;
}

void display_json(json::value const& jvalue, std::string const& prefix)
{
	if (!prefix.empty()) BOOST_LOG_TRIVIAL(debug) << prefix;

	if (jvalue.is_object())
	{
		BOOST_LOG_TRIVIAL(debug) << "    {";
		for (auto const& e : jvalue.as_object())
		{
			BOOST_LOG_TRIVIAL(debug) << "      { " << conversions::to_utf8string(e.first).substr(0, 20)
				<< " : \t" << conversions::to_utf8string(e.second.to_string()).substr(0, 150) << "}";
		}
		BOOST_LOG_TRIVIAL(debug) << "    }";
	}
	if (jvalue.is_array())
	{
		BOOST_LOG_TRIVIAL(debug) << "  [";
		for (auto const& e : jvalue.as_array())
		{
			display_json(e, "");
		}
		BOOST_LOG_TRIVIAL(debug) << "  ]";
	}
}


void print_json(json::value const& jvalue)
{
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);

	std::stringstream ss;
	ss << "../log/" << std::put_time(std::localtime(&in_time_t), "%d-%m-%Y %H-%M-%S") << ".json.txt";

	ofstream ofs(ss.str(), ios::out);
	if (!ofs.is_open()) BOOST_LOG_TRIVIAL(debug) << "JSON saver error: " << errno;
	ofs << conversions::to_utf8string(jvalue.serialize());

	ofs.close();
}

static size_t _counter = 1;

void handle_request(http_request request, function<void(json::value const&, json::value&)> action)
{
	size_t cc = _counter++;
	auto start = steady_clock::now();
	http::status_code sc = status_codes::OK;
	string cc_s = to_string(cc) + ":";
	BOOST_LOG_TRIVIAL(debug) << cc_s << "From remote: " << utility::conversions::to_utf8string(request.get_remote_address());
	BOOST_LOG_TRIVIAL(debug) << cc_s << utility::conversions::to_utf8string(request.method()) << " "
		<< utility::conversions::to_utf8string(request.request_uri().to_string());

	auto answer = json::value::object();
	try
	{
		request
			.extract_json()
			.then([&answer, &cc_s, &action, &sc](pplx::task<json::value> task)
				{
					try
					{
						auto const& jvalue = task.get();
						display_json(jvalue, cc_s + "Request: ");

						print_json(jvalue);

						action(jvalue, answer);
					}
					catch (http_exception const& ec)
					{
						JSON_EXCEPTION(answer, ec.what());
					}
				})
			.wait();
	}
	catch (const std::exception& ec)
	{
		JSON_EXCEPTION(answer, ec.what());
	}

	display_json(answer, cc_s + "Answer: ");

	auto diff = steady_clock::now() - start;
	BOOST_LOG_TRIVIAL(debug) << cc_s << "duration: " << duration_cast<seconds>(diff).count() << "s " << duration_cast<milliseconds>(diff % seconds(1)).count() << "ms";

	request.reply(sc, answer);
}

bool check_tmp(void* ptr)
{
	float* tmp = (float*)ptr;
	float summ = 0.f;
	for (size_t i = 0; i < ABIS_TEMPLATE_LEN; i++) summ += tmp[i] * tmp[i];

	return abs(summ - 1.f) < 0.00001f;
}

void* json2tmp(const web::json::value& el)
{
	void* result = nullptr;

	float* tmp = (float*)malloc(ABIS_TEMPLATE_SIZE);
	if (tmp != nullptr)
	{
		memset(tmp, 0, ABIS_TEMPLATE_SIZE);

		auto element_tmp = el.at(ELEMENT_VALUE).as_array();
		for (size_t i = 0; i < element_tmp.size(); i++)
		{
			tmp[i] = element_tmp[i].as_double();
		}
		result = tmp;
	}

	return result;
}

void* json2fingergost_tmp(const web::json::value& el)
{
	void* result = nullptr;

	char* tmp = (char*)malloc(ABIS_FINGER_TEMPLATE_SIZE);
	if (tmp != nullptr)
	{
		memset(tmp, 0, ABIS_FINGER_TEMPLATE_SIZE);

		auto element_tmp = el.at(ELEMENT_VALUE).as_array();
		for (size_t i = 0; i < element_tmp.size(); i++)
		{
			tmp[i] = (char)(element_tmp[i].as_integer() & 0xFF);
		}
		result = tmp;
	}

	return result;
}

int face_tmp_from_json(json::value el, int& tmp_type, void*& tmp_ptr)
{
	int res = 0;

	int element_type = el.at(ELEMENT_TYPE).as_integer();
	if (element_type == ABIS_FACE_IMAGE)
	{
		auto element_image = el.at(ELEMENT_VALUE).as_string();
		while ((element_image.length() % 4) != 0) element_image += U("=");
		vector<unsigned char> buf = conversions::from_base64(element_image);

		float* face_tmp = (float*)malloc(ABIS_TEMPLATE_SIZE);
		memset(face_tmp, 0, ABIS_TEMPLATE_SIZE);

		tmp_type = ABIS_FACE_TEMPLATE;
		tmp_ptr = face_tmp;

		res = extract_face_template(buf.data(), buf.size(), face_tmp, ABIS_TEMPLATE_SIZE);

		if (res > 0)
		{
			float summ = 0.f;
			for (size_t i = 0; i < ABIS_TEMPLATE_LEN; i++) summ += face_tmp[i] * face_tmp[i];
			if (abs(summ - 1.f) >= 0.0001) res = -tmp_type;
		}
	}
	if (element_type == ABIS_FACE_TEMPLATE)
	{
		tmp_type = ABIS_FACE_TEMPLATE;
		tmp_ptr = json2tmp(el);
		res = (check_tmp(tmp_ptr) ? 1 : -tmp_type);
	}

	return res;
}

int finger_tmp_from_json(json::value el, int& tmp_type, void*& tmp_ptr)
{
	int element_type = el.at(ELEMENT_TYPE).as_integer();
	int res = 0;

	if (element_type == ABIS_FINGER_IMAGE)
	{
		auto element_image = el.at(ELEMENT_VALUE).as_string();
		while ((element_image.length() % 4) != 0) element_image += U("=");
		vector<unsigned char> buf = conversions::from_base64(element_image);

		tmp_ptr = malloc(ABIS_TEMPLATE_SIZE);
		if (tmp_ptr == nullptr) return -1;
		memset(tmp_ptr, 0, ABIS_TEMPLATE_SIZE);

		tmp_type = ABIS_FINGER_TEMPLATE;

		res = extract_finger_template(buf.data(), buf.size(), tmp_ptr, ABIS_TEMPLATE_SIZE, false);
	}

	if (element_type == ABIS_FINGER_GOST_IMAGE)
	{
		auto element_image = el.at(ELEMENT_VALUE).as_string();
		while ((element_image.length() % 4) != 0) element_image += U("=");
		vector<unsigned char> buf = conversions::from_base64(element_image);

		tmp_ptr = malloc(ABIS_FINGER_TEMPLATE_SIZE);
		if (tmp_ptr == nullptr) return -1;
		memset(tmp_ptr, 0, ABIS_FINGER_TEMPLATE_SIZE);

		tmp_type = ABIS_FINGER_GOST_TEMPLATE;

		res = extract_finger_templates(buf.data(), buf.size(), tmp_ptr, ABIS_TEMPLATE_SIZE, (uchar*)tmp_ptr + ABIS_TEMPLATE_SIZE, ABIS_FINGER_TMP_GOST_SIZE);
	}

	if (element_type == ABIS_FINGER_TEMPLATE)
	{
		tmp_type = ABIS_FINGER_TEMPLATE;
		tmp_ptr = json2tmp(el);
		res = (check_tmp(tmp_ptr) ? 1 : -tmp_type);
	}

	if (element_type == ABIS_FINGER_GOST_TEMPLATE)
	{
		tmp_type = ABIS_FINGER_GOST_TEMPLATE;
		tmp_ptr = json2fingergost_tmp(el);
		res = (check_tmp(tmp_ptr) ? 1 : -tmp_type);
	}

	return res;
}

int finger_xyt_from_json(json::value el, int& tmp_type, void*& tmp_ptr)
{
	int element_type = el.at(ELEMENT_TYPE).as_integer();
	int res = 0;

	if (element_type == ABIS_FINGER_GOST_IMAGE)
	{
		auto element_image = el.at(ELEMENT_VALUE).as_string();
		while ((element_image.length() % 4) != 0) element_image += U("=");
		vector<unsigned char> buf = conversions::from_base64(element_image);

		tmp_ptr = malloc(ABIS_FINGER_TEMPLATE_SIZE);
		if (tmp_ptr == nullptr) return -1;
		memset(tmp_ptr, 0, ABIS_FINGER_TEMPLATE_SIZE);

		tmp_type = ABIS_FINGER_GOST_TEMPLATE;

		res = extract_finger_xyt(buf.data(), buf.size(), tmp_ptr, ABIS_TEMPLATE_SIZE, 
			(uchar*)tmp_ptr + ABIS_TEMPLATE_SIZE, ABIS_FINGER_TMP_GOST_SIZE);
	}

	return res;
}

int livecheck_from_json(json::value el, int& tmp_type, void*& tmp_ptr)
{
	int element_type = el.at(ELEMENT_TYPE).as_integer();
	int res = 0;

	if (element_type == ABIS_LIVEFACE_IMAGE)
	{
		auto element_image = el.at(ELEMENT_VALUE).as_string();
		while ((element_image.length() % 4) != 0) element_image += U("=");
		vector<unsigned char> buf = conversions::from_base64(element_image);

		tmp_type = ABIS_LIVEFACE_IMAGE;

		int* live_res = (int*)malloc(sizeof(int));
		res = live_check(buf.data());
		*live_res = res;
	}

	return res;
}

int tmp_from_json(json::value el, int& tmp_type, void*& tmp_ptr)
{
	int res = 0;

	int element_type = el.at(ELEMENT_TYPE).as_integer();
	if (element_type == ABIS_FACE_IMAGE || element_type == ABIS_FACE_TEMPLATE) res = face_tmp_from_json(el, tmp_type, tmp_ptr);
	if (element_type == ABIS_FINGER_IMAGE || element_type == ABIS_FINGER_TEMPLATE) res = finger_tmp_from_json(el, tmp_type, tmp_ptr);
	if (element_type == ABIS_FINGER_GOST_IMAGE || element_type == ABIS_FINGER_GOST_TEMPLATE) res = finger_tmp_from_json(el, tmp_type, tmp_ptr);
	if (element_type == ABIS_LIVEFACE_IMAGE) res = livecheck_from_json(el, tmp_type, tmp_ptr);

	if (res < 0) res -= element_type * 1000;
	return res;
}

