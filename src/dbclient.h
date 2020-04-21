#pragma once

#ifndef DBCLIENT_H
#define DBCLIENT_H

#include "AbisRest.h"

extern  std::string db_connection_url;

#define DB_DATABASE_NAME        "dbABIS"
#define DB_BIOCARD_TABLE_NAME   "t_biocards"
#define DB_BIOCARD_VALUES_NAME  ""
#define DB_TMPLINK_TABLE_NAME   "biocard_template_link"
#define DB_TEMPLATE_TABLE_NAME  ""

#endif
