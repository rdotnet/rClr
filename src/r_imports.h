#pragma once

#define STRICT_R_HEADERS // Otherwise dangerously named macros such as Free, Realloc mess things up with the Windows SDK.
#define R_RANDOM_H

#include <R.h>
#include <Rinternals.h>
#include <Rdefines.h> // TODO: The usage seems more to be with S code. Consider try to stick to Rinternals only, if this is anyway a superset
#include <Rversion.h>
#include <R_ext/Callbacks.h>
