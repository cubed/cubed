#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <exception>

#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <stdint.h>
#include <sys/file.h>

// Json
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/filereadstream.h"

#include "log.h"

static Log LOG("cubed.log", true);
static int CUBED_VERSION = 1;
