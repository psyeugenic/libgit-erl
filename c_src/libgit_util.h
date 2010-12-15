/*
 * Copyright (c) 2010 Björn-Egil Dahlberg
 *
 * Created: 2010-12-15
 * Author:  Björn-Egil Dahlberg
 *
 */

#ifndef __GIT_UTIL_H__
#define __GIT_UTIL_H__

#include <git2.h>
#include "erl_nif.h"

ERL_NIF_TERM egit_oid_to_binary(ErlNifEnv *env, const git_oid *id);
int egit_oid_from_binary(ErlNifEnv *env, ERL_NIF_TERM sha, git_oid *id);

#endif
