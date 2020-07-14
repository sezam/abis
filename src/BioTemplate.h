#pragma once

#ifndef BIOTEMPLATE_H
#define BIOTEMPLATE_H

#include "AbisRest.h"
#include "restutils.h"
#include "fplibclient.h"
#include "ebsclient.h"

class BioTemplate
{
public:
	vector<uchar> vec()
	{
		return { begin(m_tmp), begin(m_tmp) + ABIS_TEMPLATE_SIZE };
	}

	vector<float> tmp()
	{
		vector<float> t;
		float* p = (float*)m_tmp.data();
		t.assign(p, p + ABIS_TEMPLATE_LEN);

		return t;
	}

	bool check()
	{
		bool res = false;

		auto v = tmp();
		res = v.size() == ABIS_TEMPLATE_LEN;

		if (res)
		{
			float summ = 0.f;
			for (auto f : v) summ += f * f;
			res &= abs(summ - 1.f) <= 0.001;
		}
		return res;
	}

	uchar type()
	{
		return m_type;
	}

	void type(uchar type)
	{
		m_type = type;
	}

	BioTemplate() : m_type(ABIS_DATA)
	{
	}

protected:
	vector<uchar> m_tmp;
	uchar m_type;
};


class FaceTemplate : public BioTemplate
{
public:
	FaceTemplate(vector<uchar> base64)
	{
		m_tmp.resize(ABIS_TEMPLATE_SIZE);
		if (extract_face_template(base64.data(), base64.size(), m_tmp.data(), ABIS_TEMPLATE_SIZE) <= 0) m_tmp.clear();
		type(ABIS_FACE_TEMPLATE);
	}

	FaceTemplate(const web::json::value& el)
	{
		m_tmp.clear();
		auto json_tmp = el.at(ELEMENT_VALUE).as_array();
		for (auto el : json_tmp)
		{
			float f = el.as_double();
			m_tmp.insert(end(m_tmp), (uchar*)&f, (uchar*)&f + sizeof(float));
		}
		BOOST_LOG_TRIVIAL(trace) << typeid(this).name() << "." << __func__ << ": " << el.serialize();
		type(ABIS_FACE_TEMPLATE);
	}
};

class FingerTemplate : public BioTemplate
{
public:
	vector<uchar> gost()
	{
		auto v = vec();
		return { begin(v) + ABIS_TEMPLATE_SIZE, end(v) };
	}

	FingerTemplate(vector<uchar> base64)
	{
		m_tmp.resize(ABIS_FINGER_TEMPLATE_SIZE);
		if (extract_finger_templates(base64.data(), base64.size(), m_tmp.data(), ABIS_TEMPLATE_SIZE, 
			m_tmp.data() + ABIS_TEMPLATE_SIZE, ABIS_FINGER_TMP_GOST_SIZE) <= 0) m_tmp.clear();
		type(ABIS_FINGER_TEMPLATE);
	}

	FingerTemplate(const web::json::value& el)
	{
		m_tmp.clear();
		auto json_tmp = el.at(ELEMENT_VALUE).as_array();
		for (auto el : json_tmp)
		{
			m_tmp.push_back((uchar)el.as_integer() && 0xFF);
		}
		BOOST_LOG_TRIVIAL(trace) << typeid(this).name() << "." << __func__ << ": " << el.serialize();
		type(ABIS_FINGER_TEMPLATE);
	}

	bool check()
	{
		bool res = false;

		auto v = tmp();
		res = v.size() == ABIS_TEMPLATE_LEN;

		if (res)
		{
			float summ = 0.f;
			for (auto f : v) summ += f * f;
			res &= abs(summ - 1.f) <= 0.001;
		}

		auto g = gost();
		res &= g.size() == ABIS_FINGER_TMP_GOST_SIZE;
		return res;
	}
};

#endif
